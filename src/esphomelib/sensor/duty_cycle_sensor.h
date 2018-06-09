//
//  duty_cycle_sensor.h
//  esphomelib
//
//  Created by Otto Winter on 09.06.18.
//  Copyright © 2018 Otto Winter. All rights reserved.
//

#ifndef ESPHOMELIB_DUTY_CYCLE_SENSOR_H
#define ESPHOMELIB_DUTY_CYCLE_SENSOR_H

#include "esphomelib/sensor/sensor.h"
#include "esphomelib/defines.h"
#include <vector>

#ifdef USE_DUTY_CYCLE_SENSOR

ESPHOMELIB_NAMESPACE_BEGIN

namespace sensor {

class DutyCycleSensor : public PollingSensorComponent {
 public:
  DutyCycleSensor(const std::string &name, GPIOPin *pin, uint32_t update_interval = 15000);

  void setup() override;
  void update() override;
  std::string unit_of_measurement() override;
  std::string icon() override;
  int8_t accuracy_decimals() override;

  void on_interrupt();

 protected:
  GPIOPin *pin_;
  volatile uint32_t last_interrupt_{0};
  volatile uint32_t on_time_{0};
  volatile bool last_level_{false};
};

extern std::vector<DutyCycleSensor *> duty_cycle_sensors;

} // namespace sensor

ESPHOMELIB_NAMESPACE_END

#endif //USE_DUTY_CYCLE_SENSOR

#endif //ESPHOMELIB_DUTY_CYCLE_SENSOR_H
