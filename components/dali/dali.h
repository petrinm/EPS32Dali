#pragma once

#if defined(USE_ARDUINO) && defined(USE_ESP32)

#include "esphome/core/component.h"
#include "esphome/core/gpio.h"
#include "esphome/core/hal.h"

#include "esphome/components/esp32/gpio.h"

#include <esp32-hal-timer.h>


namespace esphome {
namespace DALI {


struct DALIState {

  // Current state
  unsigned int state;

  // Manchester coded forward frame
  uint32_t forward_frame;

  // Bit counter
  uint32_t bitcount;

  // Flag to indicate wether a backward frame is expected
  bool waitForAnswer;

  // Received backwarf frame
  uint32_t backward_frame;

  // Transmitter pin
  InternalGPIOPin *tx_pin;

  // Receiver pin
  InternalGPIOPin *rx_pin;

  // Timer
  hw_timer_t *timer;

};

/* DT8 Colour value defines (Part 209 Table 11) */
enum QueryColourValue
{
  XCoordinate = 0,
  YCoordinate = 1,
  ColourTemperatureTC = 2,
  PrimaryNDimLevel0 = 3,
  PrimaryNDimLevel1 = 4,
  PrimaryNDimLevel2 = 5,
  PrimaryNDimLevel3 = 6,
  PrimaryNDimLevel4 = 7,
  PrimaryNDimLevel5 = 8,
  RedDimLevel = 9,
  GreenDimLevel = 10,
  BlueDimLevel = 11,
  WhiteDimLevel = 12,
  AmberDimLevel = 13,
  FreecolourDimLevel = 14,
  RGBWAFControl = 15,
  XCoordinatePrimaryN0 = 64,
  YCoordinatePrimaryN0 = 65,
  TYPrimaryN0 = 66,
  XCoordinatePrimaryN1 = 67,
  YCoordinatePrimaryN1 = 68,
  TYPrimaryN1 = 69,
  XCoordinatePrimaryN2 = 70,
  YCoordinatePrimaryN2 = 71,
  TYPrimaryN2 = 72,
  XCoordinatePrimaryN3 = 73,
  YCoordinatePrimaryN3 = 74,
  TYPrimaryN3 = 75,
  XCoordinatePrimaryN4 = 76,
  YCoordinatePrimaryN4 = 77,
  TYPrimaryN4 = 78,
  XCoordinatePrimaryN5 = 79,
  YCoordinatePrimaryN5 = 80,
  TYPrimaryN5 = 81,
  NumberOfPrimaries = 82,
  ColourTemperatureTcCoolest = 128,
  ColourTemperatureTcPhysicalCoolest = 129,
  ColourTemperatureTcWarmest = 130,
  ColourTemperatureTcPhysicalWarmest = 131,
  TemporaryXCoordinate = 192,
  TemporaryYCoordinate = 193,
  TemporaryColourTemperature = 194,
  TemporaryPrimaryNDimLevel0 = 195,
  TemporaryPrimaryNDimLevel1 = 196,
  TemporaryPrimaryNDimLevel2 = 197,
  TemporaryPrimaryNDimLevel3 = 198,
  TemporaryPrimaryNDimLevel4 = 199,
  TemporaryPrimaryNDimLevel5 = 200,
  TemporaryRedDimLevel = 201,
  TemporaryGreenDimLevel = 202,
  TemporaryBlueDimLevel = 203,
  TemporaryWhiteDimLevel = 204,
  TemporaryAmberDimLevel = 205,
  TemporaryFreecolourDimLevel = 206,
  TemporaryRgbwafControl = 207,
  TemporaryColourType = 208,
  ReportXCoordinate = 224,
  ReportYCoordinate = 225,
  ReportColourTemperatureTc = 226,
  ReportPrimaryNDimLevel0 = 227,
  ReportPrimaryNDimLevel1 = 228,
  ReportPrimaryNDimLevel2 = 229,
  ReportPrimaryNDimLevel3 = 230,
  ReportPrimaryNDimLevel4 = 231,
  ReportPrimaryNDimLevel5 = 232,
  ReportRedDimLevel = 233,
  ReportGreenDimLevel = 234,
  ReportBlueDimLevel = 235,
  ReportWhiteDimLevel = 236,
  ReportAmberDimLevel = 237,
  ReportFreecolourDimLevel = 238,
  ReportRgbwafControl = 239,
  ReportColourType = 240,
};


/* DT8 Temperature limit defines */
enum TemperatureLimit
{
  Tc_Coolest = 0,
  Tc_Warmest = 1,
  Tc_Physical_Coolest = 2,
  Tc_Physical_Warmest = 3
};


// Broadcast address
const uint8_t BROADCAST = 0x7F;


class DALIInterface: public Component {
 public:
  void setup() override;
  void dump_config() override;

  // Search ballasts
  void search();

  //--- Setup functions
  void set_tx_pin(InternalGPIOPin *tx_pin);
  void set_rx_pin(InternalGPIOPin *rx_pin);

  /* */
  void send_forward_frame(uint16_t forward_frame, bool repeat=false);
  void send_forward_frame(uint16_t forward_frame, uint16_t &response);

  /* Send Direct Arc Power Control */
  void send_dapc(uint8_t address, uint8_t power);

  /* Extinguish the lamp immediately without fading. */
  void off(uint8_t address);

  /* Increases the target illumination level */
  void up(uint8_t address);

  /* Decreases the target illumination level */
  void down(uint8_t address);

  /* Sets a group of lamps to a predefined scene */
  void goto_scene(uint8_t address, uint8_t scene);

  /* Configures the control gear's maximum output level */
  void set_max_level(uint8_t address, uint8_t level);

  /* Configures the control gear's minimum output level */
  void set_min_level(uint8_t address, uint8_t level);

  /* Configures the output level upon power-up */
  void set_on_power_level(uint8_t address, uint8_t level);

  /* Sets the fade time */
  void set_fade_time(uint8_t address, uint8_t fade_time);

  /* Sets the fade rate */
  void set_fade_rate(uint8_t address, uint8_t fade_rate);

  /* Sets the extended fade time */
  void set_extended_fade_time(uint8_t address, uint8_t fade_time);

  /* Adds the gear to scene */
  void set_scene(uint8_t address, uint8_t scene, uint8_t level);

  /* Removes one of the control gears from a scene */
  void remove_from_scene(uint8_t address, uint8_t scene);

  /* Adds a control gear to a group */
  void add_to_group(uint8_t address, uint8_t group);

  /* Removes a control gear from a group */
  void remove_from_group(uint8_t address, uint8_t group);

  /* Sets a control gear's short address */
  void set_short_address(uint8_t address, uint8_t new_address);

  /* Allows writing into memory banks */
  void enable_write_memory(uint8_t address);

  /* Write memory location */
  void write_memory(uint8_t location, uint8_t bank, uint8_t value);

  /* Read memory location */
  void read_memory(uint8_t address, uint8_t location, uint8_t bank);

  /* Set Temporary X-Coordinate (DT8) */
  void send_xcoord(uint8_t address, unsigned int coord);

  /* Set Temporary Y-Coordinate (DT8) */
  void send_ycoord(uint8_t address, unsigned int coord);

  /* Set Temporary Colour Temperature (DT8) */
  void set_cct(uint8_t address, unsigned int kelvin);

  /* Set Colour Temperarture Limit (DT8) */
  void set_tc_limit(uint8_t address, TemperatureLimit what, unsigned int limit);

  /* Query Colour value */
  uint16_t query_colour_value(uint8_t address, QueryColourValue query);

  /* Send device type */
  void set_device_type(uint8_t device_type);

protected:

  DALIState driver_;

  /* Perform Manchester coding to the forward frame payload bytes. */
  static uint32_t DALI_Encode(uint16_t forwardFrame);

  /**/
  static bool DALI_Decode(uint32_t rawData, uint16_t *pBackwardFrame);

};


}  // namespace DALI
}  // namespace esphome

#endif
