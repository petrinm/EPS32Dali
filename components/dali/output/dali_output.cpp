#if defined(USE_ARDUINO) && defined(USE_ESP32)

#include "dali_output.h"
#include "../dali_cmds.h"
#include "esphome/core/log.h"


namespace esphome {
namespace DALI {

static const char *TAG = "dali_output";

void DALIOutput::setup() {

  // bool _check_destination(uint8_t address)
  //   return address < 0 or address > 63;

  // add to group
  // remove from group

}

void DALIOutput::loop()
{

  /*if(updateOutput_ == true){
    if(millis() > lasttime_ms_ + updatetime_ms_){
      ESP_LOGD(TAG, "Sending Brightness: %f", state_);
      updateOutput_ = false;
      interface->send_dapc(address, state_);
    }
  }*/
}


void DALIOutput::dump_config() {
  ESP_LOGCONFIG(TAG, "DALIOutput");
  ESP_LOGCONFIG(TAG, "   address: %d", address_);
  //ESP_LOGCONFIG(TAG, "   faderate: %.1f%%", faderate_);
  //ESP_LOGCONFIG(TAG, "   fadetme: %.1f%%", fadetime_);
}


void DALIOutput::write_state(float value)
{
  ESP_LOGD(TAG, "write_state: value=%f", value);
  //state_ = value;
  //updateOutput_ = true;
  //lasttime_ms_ = millis();

  interface->send_dapc(address_, (int)(255 * value));

  //interface->sendSetFadeTime(address_, 0);
  //interface->sendSetFadeRate(address_, 0);
}


} //namespace DALI
} //namespace esphome

#endif
