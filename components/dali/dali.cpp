#if defined(USE_ARDUINO) && defined(USE_ESP32)

#include "dali.h"
#include "dali_cmds.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "driver/timer.h"



namespace esphome {
namespace DALI {

static const char *TAG = "dali_interface";


/*******************************************************************************
 * Typedefs and defines
 ******************************************************************************/

#define BI_PHASE_HIGH           0b01
#define BI_PHASE_LOW            0b10
#define BI_PHASE_MASK           0b11


/*******************************************************************************
 * Define DALI timing values. One Bit on a DALI bus is
 * 833.33µs (1200 bauds) which corresponds to 2TE.
 ******************************************************************************/

#define TE            (417)                     // half bit time = 417 usec

/* Transmission timing, option to compensate for unequal physical layer delay in rising or falling edge*/
#define TE_HIGH       (TE + 0)
#define TE_LOW        (TE - 0)

#define DALI_STOP_BIT_TIME          (4*TE)
#define MIN_FORWARD_FRAME_DELAY     (22*TE)     // minimum time between two forward frames - 9.17ms ==> 22 x TE time
#define BACKWARD_FRAME_DELAY        (12*TE)     // time between forward frame and backward frame >= 7TE

#if 0 /* strict receive timing according to specification  */
  #define MIN_TE      (TE - 42)                 // minimum half bit time
  #define MAX_TE      (TE + 42)                 // maximum half bit time
  #define MIN_2TE     (2*TE - 83)               // minimum full bit time
  #define MAX_2TE     (2*TE + 83)               // maximum full bit time
#else /* More relaxed receive timing */
  #define MIN_TE      (300)                     // minimum half bit time
  #define MAX_TE      (550)                     // maximum half bit time
  #define MIN_2TE     (2*TE - (2*(TE/5)))       // minimum full bit time
  #define MAX_2TE     (2*TE + (2*(TE/5)))       // maximum full bit time
#endif

#define BACKWARD_FRAME_BIT_LENGTH     18 // two byte commands have 34 bit in total (including start bit)



/* State machine states: */
typedef enum stateTag
{
  MS_IDLE = 0,                        // bus idle
  MS_TX_SECOND_HALF_START_BIT,        //
  MS_TX_DALI_FORWARD_FRAME,           // sending the dali forward frame
  MS_TX_STOP_BITS,                    //
  MS_SETTLING_BEFORE_BACKWARD,        // settling between forward and backward - stop bits
  MS_SETTLING_BEFORE_IDLE,            // settling before going to idle, after forward frame
  MS_WAITING_FOR_SLAVE_START_WINDOW,  // waiting for 7Te, start of slave Tx window
  MS_WAITING_FOR_SLAVE_START,         // start of slave Tx window
  MS_RECEIVING_ANSWER                 // receiving slave message
} MASTER_STATE;


void inline DALI_SetOutputHigh(DALIState* driver) {
  driver->tx_pin->digital_write(false); // INVERTED!!!
}

void inline DALI_SetOutputLow(DALIState *driver) {
  driver->tx_pin->digital_write(true); // INVERTED!!!
}

bool inline DALI_GetInput(DALIState *driver) {
  return driver->rx_pin->digital_read();
}


/// Run timer interrupt code and return in how many µs the next event is expected
/* the handling of the protocol is done in the IRQ */

DALIState *gbl_driver;

static void IRAM_ATTR HOT gpio_interrupt(DALIState *driver);

static void IRAM_ATTR HOT timer_interrupt() // DALIState *driver);
{
  DALIState *driver = gbl_driver;

  switch(driver->state)
  {
    case MS_TX_SECOND_HALF_START_BIT:
      /*
       * Sending start bit
       */
      DALI_SetOutputHigh(driver);
      driver->bitcount = 0;

      // Timer auto reloads and next waits TE
      driver->state = MS_TX_DALI_FORWARD_FRAME;

      break;

    case MS_TX_DALI_FORWARD_FRAME:
      /*
       * Sending coded data bits
       */
      if (driver->forward_frame & 0x80000000)
        DALI_SetOutputHigh(driver);
      else
        DALI_SetOutputLow(driver);

      driver->forward_frame <<= 1;
      driver->bitcount++;

      // End of data bits?
      if (driver->bitcount == 32)
        driver->state = MS_TX_STOP_BITS;

      // Timer auto reloads and next waits TE
      break;

    case MS_TX_STOP_BITS:
      /*
       * Sending stop bits
       */

      // High state to free the bus
      DALI_SetOutputHigh(driver);

      // The first half of the first stop bit has just been output.
      // Do we have to wait for an answer?
      if (driver->waitForAnswer)
      {
        // Elapse until the end of the last half of the second stop bit
        timerAlarmWrite(driver->timer, 4 * TE, false);

        driver->backward_frame = 0;
        driver->state = MS_SETTLING_BEFORE_BACKWARD;
      }
      else
      {
        // No answer from slave expected, need to wait for the remaining
        // bus idle time before next forward frame.
        // Add additional 3 TE to minimum specification to be not at the edge of the timing specification
        timerAlarmWrite(driver->timer, (4 + 22 + 3) * TE, false);
        driver->state = MS_SETTLING_BEFORE_IDLE;
      }
      break;

    case MS_SETTLING_BEFORE_BACKWARD:
      /*
       * Wait for
       */

      DALI_SetOutputHigh(driver);

      // Setup the first window limit for the slave answer.
      // The slave should not respond before 7TE.
      timerAlarmWrite(driver->timer, 7 * MIN_TE, false);

      // Enable capture on both edges.
      driver->rx_pin->attach_interrupt(&gpio_interrupt, driver, gpio::INTERRUPT_ANY_EDGE);

      driver->state = MS_WAITING_FOR_SLAVE_START_WINDOW;
      break;

    case MS_WAITING_FOR_SLAVE_START_WINDOW:
      /*
       *
       */

      // Setup the second window limit for the slave answer,
      // slave must start transmit within the next 23TE window.
      timerAlarmWrite(driver->timer, 23 * MAX_TE, false);
      driver->state = MS_WAITING_FOR_SLAVE_START;

      // Next we should receive GPIO interrupts
      break;

    case MS_WAITING_FOR_SLAVE_START:
      /*
       * Receive timeout. Nothing was received
       */

      // If we still get here, got 'no' or too early answer from slave
      // idle time of 23TE was already elapsed while waiting, so
      // immediately release the bus.

      // Reset and stop the timer
      timerStop(driver->timer);

      // Disable capture
      driver->rx_pin->detach_interrupt();

      // Execution returns to Do_transfer function
      driver->state = MS_IDLE;
      break;

    case MS_RECEIVING_ANSWER:
      /*
       * Receive timeout in middle of frame
       */

      // Stop receiving
      // Now idle the bus between backward and next forward frame
      // since we don't track the last edge of received frame,
      // conservatively we wait for 23 TE (>= 22 TE as for specification).
      // Receive interval considered anyway the max tolerance for
      // backward frame duration so >22TE should already be asserted.
      timerAlarmWrite(driver->timer, 23 * TE, false);

      // Disable capture
      driver->rx_pin->detach_interrupt();
      DALI_SetOutputHigh(driver);

      driver->state = MS_SETTLING_BEFORE_IDLE;
      break;

    case MS_SETTLING_BEFORE_IDLE:
      /*
       * Settling before the bus is considered idle/available again.
       */

      timerStop(driver->timer);
      driver->rx_pin->detach_interrupt();

      // Execution returns to Do_transfer function
      driver->state = MS_IDLE;
      break;

    default:
      /*
       * Fault state
       */
      DALI_SetOutputHigh(driver);
      timerStop(driver->timer);
      driver->rx_pin->detach_interrupt();
      driver->state = MS_IDLE;
  }
}


static void IRAM_ATTR HOT gpio_interrupt(DALIState *driver)
{
  switch (driver->state) {
    case MS_WAITING_FOR_SLAVE_START_WINDOW:
      /*
       * The slave should not have answered yet, it came too early!!!!
       */
      timerStop(driver->timer);
      driver->rx_pin->detach_interrupt();
      driver->state = MS_IDLE;
      break;

    case MS_WAITING_FOR_SLAVE_START: {
      /*
       * We got an edge (he first half bit of START), so the slave is transmitting now.
       */

      // Set receive timeout time.
      timerAlarmWrite(driver->timer, 22 * MAX_TE, false);

      // Clear timer counter so we can measure the pulse length
      timerWrite(driver->timer, 0);

      driver->backward_frame <<= 1;

      bool DaliBusLevel = DALI_GetInput(driver);
      if (DaliBusLevel)
        driver->backward_frame |= BI_PHASE_HIGH;

      // Increment bit counter by 1
      driver->bitcount += 1;
      driver->state = MS_RECEIVING_ANSWER;
      break;
    }
    case MS_RECEIVING_ANSWER: {
      /*
       * Received a next edge
       */

      // Get current bus level and the length of the pulse
      bool DaliBusLevel = DALI_GetInput(driver);
      uint32_t CT = timerRead(driver->timer);
      timerWrite(driver->timer, 0);

      // Check the pulse width for TE or 2TE time
      if (MIN_2TE < CT && CT < MAX_2TE)
      {
        // This is a 2TE pulse, we need to shift to bits shift both bits to the left...
        driver->backward_frame <<= 2;

        if (DaliBusLevel)
          driver->backward_frame |= BI_PHASE_HIGH;
        else
          driver->backward_frame |= BI_PHASE_LOW;

        // increment position counter by 2
        driver->bitcount += 2;
      }
      else if (MIN_TE < CT && CT < MAX_TE)
      {
        // This is a TE pulse, we just need to shift one bit == current LEVEL
        // shift one bit to the left...
        driver->backward_frame <<= 1;

        if (DaliBusLevel)
          driver->backward_frame |= BI_PHASE_HIGH;

        // Increment position counter by 1
        driver->bitcount += 1;
      }
      else
      {
        // This pulse is not a valid DALI bi-phase pulse. We stop receiving
        // by setting the state to DALI_PD_STATE_IDLE and wait for the next one.
        // The idle timeout (MR0) will set us back to receive mode...
        driver->bitcount = 0;
        driver->backward_frame = 0;
        driver->state = MS_IDLE;

        driver->rx_pin->detach_interrupt();

        break;
      }

      // Check if we have receive all bits...
      if (driver->bitcount == BACKWARD_FRAME_BIT_LENGTH)
      {
        // we are ready now... we wait for STOP bits using MR2 interrupt.
        timerWrite(driver->timer, 0);
        timerAlarmWrite(driver->timer, 22 * MAX_TE, false);

        driver->rx_pin->detach_interrupt();

        // Goto idle via settling state to avoid spam.
        driver->state = MS_SETTLING_BEFORE_IDLE;
      }

      break;
    }

    default:
      break;
  }
}



void DALIInterface::setup()
{
  int div = getApbFrequency() / 1000000;
  ESP_LOGD(TAG, "DALI setup %d", div);

  assert(gbl_driver == nullptr);
  DALIState *driver = &driver_;
  gbl_driver = driver;

  // Force the DALI high aka disable pull down
  DALI_SetOutputHigh(driver);

  /*
   * Setup timer
   * 80 Divider -> 1 count = 1µs
   */
  driver->timer = timerBegin(0, 80, true);
  timerAttachInterrupt(driver->timer, &timer_interrupt, false);
  // Timer will be started later when we transmit.

  /*
   * Setup RX pin
   */
  ISRInternalGPIOPin rx_isr_pin = driver->rx_pin->to_isr();
  driver->rx_pin->attach_interrupt(&gpio_interrupt, driver, gpio::INTERRUPT_ANY_EDGE);
}

void DALIInterface::set_tx_pin(InternalGPIOPin *tx_pin)
{
  driver_.tx_pin = tx_pin;
}

void DALIInterface::set_rx_pin(InternalGPIOPin *rx_pin)
{
  driver_.rx_pin = rx_pin;
}

void DALIInterface::dump_config()
{
  ESP_LOGCONFIG(TAG, "DALIInterface");
  LOG_PIN("  tx_pin: ", this->driver_.tx_pin);
  LOG_PIN("  rx_pin: ", this->driver_.rx_pin);
}

uint32_t DALIInterface::DALI_Encode(uint16_t forwardFrame)
{
  uint32_t coded = 0;
  for (int i = 15; i >= 0; i--) // MSB to LSB
  {
    if (forwardFrame & (1 << i))
    {
      coded <<= 2;
      coded |= 0b01;
    }
    else
    {
      coded <<= 2;
      coded |= 0b10;
    }
  }
  return coded;
}

bool DALIInterface::DALI_Decode(uint32_t rawData, uint16_t *pBackwardFrame)
{
  uint16_t backward_frame = 0;
  for (int i = 0; i < 8; i++)
  {
    backward_frame >>= 1;
    switch (rawData & BI_PHASE_MASK)
    {
    case BI_PHASE_HIGH:
      // We shift a 1 into backward_frame from MSB to LSB position
      backward_frame |= 0x8000;
      break;
    case BI_PHASE_LOW:
      break;
    default:
      return false;
    }
    rawData >>= 2;
  }

  *pBackwardFrame = backward_frame;
  return true;
}


void DALIInterface::search()
{
  // https://github.com/sde1000/python-dali/blob/master/examples/find_ballasts.py

  // send_forward_frame((0x3f & address) << 9 | 0x0100 | power);
}

void DALIInterface::send_forward_frame(uint16_t forwardFrame, bool repeat)
{
  DALIState *driver = &driver_;

  driver->state = MS_TX_SECOND_HALF_START_BIT;
  driver->waitForAnswer = false;
  driver->forward_frame = DALI_Encode(forwardFrame);

  // Pull down to indicate first half-bit of start bit
  DALI_SetOutputLow(driver);

  // Activate the timer module to output the forward frame
  timerAlarmWrite(driver->timer, TE, true);
  timerAlarmEnable(driver->timer);
  timerRestart(driver->timer);
  timerStart(driver->timer);

  // Wait for the transmission to happen in the timer/gpio interrupt
  // Components should block for at most 30 ms
  int timeout = 10;
  while (driver->state != MS_IDLE) {
    //ESP_LOGD(TAG, "waiting.. %d %d ", driver->state, debug);
    delay(3);
    timeout--;
    if (timeout == 0) {
      //ESP_LOGD(TAG, "timeout.. %d %d ", driver->state, debug);
      // Timeout
      DALI_SetOutputHigh(driver);
      return;
    }
  }

  DALI_SetOutputHigh(driver);
  //ESP_LOGD(TAG, "done... %d %d ", driver->state, debug);

  if (driver->waitForAnswer)
  {
    // Parse return

    uint16_t back;
    if (DALI_Decode(driver->backward_frame, &back)) {
      ESP_LOGD(TAG, "backward_frame: coded=%x, raw=%x", driver->backward_frame, back);
      //return driver->backward_frame;
    }
    else {
      ESP_LOGE(TAG, "Failed to decode backward frame: %x", driver->backward_frame);
    }

  }
}


void DALIInterface::send_dapc(uint8_t address, uint8_t power)
{
  ESP_LOGD(TAG, "send_dapc: address=%d, power=%d", address, power);
  send_forward_frame((0x3f & address) << 9 | power);
}

void DALIInterface::off(uint8_t address)
{
  ESP_LOGD(TAG, "off: address=%d", address);
  send_forward_frame((0x3f & address) << 9 | 0x0100 | DALI_CMD_OFF);
}

void DALIInterface::up(uint8_t address)
{
  ESP_LOGD(TAG, "up: address=%d", address);
  send_forward_frame((0x3f & address) << 9 | 0x0100 | DALI_CMD_UP, true);
}

void DALIInterface::down(uint8_t address)
{
  ESP_LOGD(TAG, "down: address=%d", address);
  send_forward_frame((0x3f & address) << 9 | 0x0100 | DALI_CMD_DOWN);
}

void DALIInterface::goto_scene(uint8_t address, uint8_t scene)
{
  ESP_LOGD(TAG, "goto_scene: address=%d, scene=%d", address, scene);
  send_forward_frame((0x3f & address) << 9 | 0x0100 | (DALI_CMD_GO_TO_SCENE + scene));
}

void DALIInterface::set_max_level(uint8_t address, uint8_t level)
{
  ESP_LOGD(TAG, "set_max_level: address=%d, level=%d", address, level);
  send_forward_frame((DALI_CMD_DTR0_DATA << 8) | level);
  send_forward_frame((0x3f & address) << 9 | 0x0100 | DALI_CMD_SET_MAX_LEVEL);
  // TODO: Perform twice
}

void DALIInterface::set_min_level(uint8_t address, uint8_t level)
{
  ESP_LOGD(TAG, "set_min_level: address=%d, level=%d", address, level);
  send_forward_frame((DALI_CMD_DTR0_DATA << 8) | level);
  send_forward_frame((0x3f & address) << 9 | 0x0100 | DALI_CMD_SET_MIN_LEVEL);
  // TODO: Perform twice
}

void DALIInterface::set_on_power_level(uint8_t address, uint8_t level)
{
  ESP_LOGD(TAG, "set_on_power_level: address=%d, level=%d", address, level);
  send_forward_frame((DALI_CMD_DTR0_DATA << 8) | level);
  send_forward_frame((0x3f & address) << 9 | 0x0100 | DALI_CMD_SET_POWER_ON_LEVEL);
  // TODO: Perform twice
}

void DALIInterface::set_fade_time(uint8_t address, uint8_t fade_time)
{
  ESP_LOGD(TAG, "set_fade_time: address=%d, fade_time=%d", address, fade_time);
  send_forward_frame((DALI_CMD_DTR0_DATA << 8) | fade_time);
  send_forward_frame((0x3f & address) << 9 | 0x0100 | DALI_CMD_SET_FADE_TIME);
  // TODO: Perform twice
}

void DALIInterface::set_fade_rate(uint8_t address, uint8_t fade_rate)
{
  ESP_LOGD(TAG, "set_fade_rate: address=%d, fade_rate=%d", address, fade_rate);
  send_forward_frame((DALI_CMD_DTR0_DATA << 8) | fade_rate);
  send_forward_frame((0x3f & address) << 9 | 0x0100 | DALI_CMD_SET_FADE_RATE);
  // TODO: Perform twice
}

void DALIInterface::set_extended_fade_time(uint8_t address, uint8_t fade_time)
{
  ESP_LOGD(TAG, "set_extended_fade_time: address=%d, fade_time=%d", address, fade_time);
  send_forward_frame((DALI_CMD_DTR0_DATA << 8) | fade_time);
  send_forward_frame((0x3f & address) << 9 | 0x0100 | DALI_CMD_SET_EXTENDED_FADE_TIME);
}

void DALIInterface::set_scene(uint8_t address, uint8_t scene, uint8_t level)
{
  ESP_LOGD(TAG, "set_scene: address=%d, scene=%d, level=%d", address, scene, level);
  send_forward_frame((DALI_CMD_DTR0_DATA << 8) | level);
  send_forward_frame((0x3f & address) << 9 | 0x0100 | (DALI_CMD_SET_SCENE + scene));
  // TODO: Perform twice
}

void DALIInterface::remove_from_scene(uint8_t address, uint8_t scene)
{
  ESP_LOGD(TAG, "remove_from_scene: address=%d, scene=%d", address, scene);
  send_forward_frame((0x3f & address) << 9 | 0x0100 | (DALI_CMD_REMOVE_FROM_SCENE + scene));
  // TODO: Perform twice
}

void DALIInterface::add_to_group(uint8_t address, uint8_t group)
{
  ESP_LOGD(TAG, "add_to_group: address=%d, group=%d", address, group);
  send_forward_frame((0x3f & address) << 9 | 0x0100 | (DALI_CMD_ADD_TO_GROUP + group));
  // TODO: Perform twice
}

void DALIInterface::remove_from_group(uint8_t address, uint8_t group)
{
  ESP_LOGD(TAG, "remove_from_group: address=%d, group=%d", address, group);
  send_forward_frame((0x3f & address) << 9 | 0x0100 | (DALI_CMD_REMOVE_FROM_GROUP + group));
  // TODO: Perform twice
}

void DALIInterface::set_short_address(uint8_t old_address, uint8_t new_address)
{
  ESP_LOGD(TAG, "set_short_address: old_address=%d, new_address=%d", old_address, new_address);
  send_forward_frame((DALI_CMD_DTR0_DATA << 8) | new_address);
  send_forward_frame((0x3f & old_address) << 9 | 0x0100 | DALI_CMD_SET_SHORT_ADDRESS);
}

void DALIInterface::enable_write_memory(uint8_t address)
{
  ESP_LOGD(TAG, "enable_write_memory: address=%d", address);
  send_forward_frame((0x3f & address) << 9 | 0x0100 | DALI_CMD_ENABLE_WRITE_MEMORY);
}

void DALIInterface::write_memory(uint8_t location, uint8_t bank, uint8_t value)
{
  ESP_LOGD(TAG, "write_memory: location=%d, bank=%d, value=%d", location, bank, value);
  send_forward_frame((DALI_CMD_DTR0_DATA << 8) | location);
  send_forward_frame((DALI_CMD_DTR1_DATA << 8) | bank);
  send_forward_frame((DALI_CMD_WRITE_MEMORY_LOCATION_NO_REPLY << 8) | value);
}

void DALIInterface::read_memory(uint8_t address, uint8_t location, uint8_t bank)
{
  ESP_LOGD(TAG, "read_memory: address=%d, location=%d, bank=%d", address, location, bank);
  send_forward_frame((DALI_CMD_DTR0_DATA << 8) | location);
  send_forward_frame((DALI_CMD_DTR1_DATA << 8) | bank);
  uint8_t value;
  send_forward_frame((0x3f & address) << 9 | 0x0100 | DALI_CMD_READ_MEMORY_LOCATION, value);
  ESP_LOGD(TAG, "   value=%d", value);
}

void DALIInterface::send_xcoord(uint8_t address, unsigned int coord)
{
  ESP_LOGD(TAG, "send_xcoord: address=%d, xcoord=%d", address, coord);
  send_forward_frame((DALI_CMD_DTR0_DATA << 8) | (0xFF & (coord >> 0))); // LSB
  send_forward_frame((DALI_CMD_DTR1_DATA << 8) | (0xFF & (coord >> 8))); // MSB
  send_forward_frame((0x3f & address) << 9 | DALI_CMD_SET_TEMPORARY_X_COORDINATE);
}

void DALIInterface::send_ycoord(uint8_t address, unsigned int coord)
{
  ESP_LOGD(TAG, "send_xcoord: address=%d, ycoord=%d", address, coord);
  send_forward_frame((DALI_CMD_DTR0_DATA << 8) | (0xFF & (coord >> 0))); // LSB
  send_forward_frame((DALI_CMD_DTR1_DATA << 8) | (0xFF & (coord >> 8))); // MSB
  send_forward_frame((0x3f & address) << 9 | DALI_CMD_SET_TEMPORARY_Y_COORDINATE);
}

void DALIInterface::set_cct(uint8_t address, unsigned int kelvin)
{
  if (kelvin < 2000 || kelvin > 6500) {
    ESP_LOGE(TAG, "set_cct: invalid ct %d", kelvin);
    return;
  }

  unsigned int mirek = (1000000 / kelvin);
  ESP_LOGD(TAG, "send_cct: address=%d, mirek=%d", address, mirek);
  send_forward_frame((DALI_CMD_DTR0_DATA << 8) | (0xFF & (mirek >> 0))); // LSB
  send_forward_frame((DALI_CMD_DTR1_DATA << 8) | (0xFF & (mirek >> 8))); // MSB
  send_forward_frame((0x3f & address) << 9 | DALI_CMD_SET_TEMPORARY_COLOUR_TEMPERATURE);
  send_forward_frame((0x3f & address) << 9 | DALI_CMD_ACTIVATE);
}

void DALIInterface::set_tc_limit(uint8_t address, TemperatureLimit what, unsigned int limit)
{
  unsigned int mirek = (1000000 / limit);
  ESP_LOGD(TAG, "set_tc_limit: address=%d, what=%d, mirek=%d", address, what, mirek);
  send_forward_frame((DALI_CMD_DTR0_DATA << 8) | (0xFF & (mirek >> 0))); // LSB
  send_forward_frame((DALI_CMD_DTR1_DATA << 8) | (0xFF & (mirek >> 8))); // MSB
  send_forward_frame((DALI_CMD_DTR2_DATA << 8) | what);
  send_forward_frame((0x3f & address) << 9 | DALI_CMD_STORE_COLOUR_TEMPERATURE_LIMIT);
}

void DALIInterface::set_device_type(uint8_t device_type)
{
  ESP_LOGD(TAG, "set_device_type: device_type=%d", device_type);
  send_forward_frame((DALI_CMD_ENABLE_DEVICE_TYPE << 8) | device_type);
}

uint16_t DALIInterface::query_colour_value(uint8_t address, QueryColourValue query)
{
#if 0
  // 62386-209, command 250, Note 2: start by sending "QUERY ACTUAL LEVEL"
  send_forward_frame((0x3f & address) << 9 | DALI_CMD_QUERY_ACTUAL_LEVEL);
  send_forward_frame((DALI_CMD_DTR0_DATA << 8) | query);

  uint16_t msb, lsb;
  send_forward_frame((0x3f & address) << 9 | DALI_CMD_QUERY_COLOUR_VALUE, msb);
  send_forward_frame((0x3f & address) << 9 | DALI_CMD_QUERY_CONTENT_DTR0, lsb);
  return (msb << 8) | lsb;
#endif
  return 0;
}


}  // namespace DALI
}  // namespace esphome

#endif
