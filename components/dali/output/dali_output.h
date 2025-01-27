#pragma once

#if defined(USE_ARDUINO) && defined(USE_ESP32)

#include "esphome/core/component.h"
#include "esphome/components/output/float_output.h"
#include "../dali.h"

namespace esphome {
namespace DALI {

class DALIOutput: public output::FloatOutput, public Component {
 public:
  void setup() override;
  void loop() override;
  void write_state(float state) override;
  void dump_config() override;

  void set_interface(DALIInterface *interface_) { interface = interface_; }

  void set_address(int address) { address_ = address & 0x3F; }
  void set_group(int group) { address_ = 0x40 | (group & 0xF); }
  void set_broadcast(bool broadcast) { address_ = 0x7F; }

  void set_fade_time(int FadeTime) { fadetime_ = FadeTime; }
  void set_fade_rate(int FadeRate) { faderate_ = FadeRate; }

protected:
  DALIInterface *interface;

  //--- inputs
  int address_ = 0xFF;
  int fadetime_ = 1;
  int faderate_ = 2;
};


} //namespace DALI
} //namespace esphome

#endif
