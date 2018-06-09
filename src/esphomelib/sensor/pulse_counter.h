//
//  pulse_counter.h
//  esphomelib
//
//  Created by Otto Winter on 24.02.18.
//  Copyright © 2018 Otto Winter. All rights reserved.
//

#ifndef ESPHOMELIB_SENSOR_PULSE_COUNTER_H
#define ESPHOMELIB_SENSOR_PULSE_COUNTER_H

#include "esphomelib/sensor/sensor.h"
#include "esphomelib/esphal.h"
#include "esphomelib/defines.h"

#ifdef USE_PULSE_COUNTER_SENSOR

#ifdef ARDUINO_ARCH_ESP32
  #include <driver/pcnt.h>
#endif

ESPHOMELIB_NAMESPACE_BEGIN

namespace sensor {

enum PulseCounterCountMode {
  PULSE_COUNTER_DISABLE = 0,
  PULSE_COUNTER_INCREMENT,
  PULSE_COUNTER_DECREMENT,
};

/** Pulse Counter - This is the sensor component for the ESP32 integrated pulse counter peripheral.
 *
 * It offers 8 pulse counter units that can be setup in several ways to count pulses on a pin.
 * Also allows for some simple filtering of short pulses using set_filter(), any pulse shorter than
 * the value provided to that function will be discarded. The time is given in APB clock cycles,
 * which usually amount to 12.5 ns per clock. Defaults to the max possible (about 13 ms).
 * See http://esp-idf.readthedocs.io/en/latest/api-reference/peripherals/pcnt.html for more information.
 *
 * The pulse counter defaults to reporting a value of the measurement unit "pulses/min". To
 * modify this behavior, use filters in MQTTSensor.
 */
class PulseCounterSensorComponent : public PollingSensorComponent {
 public:
  /** Construct the Pulse Counter instance with the provided pin and update interval.
   *
   * The pulse counter unit will automatically be set and the pulse counter is set up
   * to increment the counter on rising edges by default.
   *
   * @param pin The pin.
   * @param update_interval The update interval in ms.
   */
  explicit PulseCounterSensorComponent(const std::string &name, GPIOPin *pin, uint32_t update_interval = 15000);

  /// Set the PulseCounterCountMode for the rising and falling edges. can be disable, increment and decrement.
  void set_edge_mode(PulseCounterCountMode rising_edge_mode, PulseCounterCountMode falling_edge_mode);

  void set_filter_us(uint32_t filter_us);

  // ========== INTERNAL METHODS ==========
  // (In most use cases you won't need these)
  /// Unit of measurement is "pulses/min".
  std::string unit_of_measurement() override;
  std::string icon() override;
  int8_t accuracy_decimals() override;
  void setup() override;
  void update() override;
  float get_setup_priority() const override;

 protected:
#ifdef ARDUINO_ARCH_ESP8266
  void gpio_intr();
  volatile int16_t counter_{0};
  volatile uint32_t last_pulse_{0};
#endif

  GPIOPin *pin_;
#ifdef ARDUINO_ARCH_ESP32
  pcnt_unit_t pcnt_unit_;
#endif
  PulseCounterCountMode rising_edge_mode_{PULSE_COUNTER_INCREMENT};
  PulseCounterCountMode falling_edge_mode_{PULSE_COUNTER_DECREMENT};
  uint32_t filter_us_{13};
  int16_t last_value_{0};
};

#ifdef ARDUINO_ARCH_ESP32
extern pcnt_unit_t next_pcnt_unit;
#endif

} // namespace sensor

ESPHOMELIB_NAMESPACE_END

#endif //USE_PULSE_COUNTER_SENSOR

#endif //ESPHOMELIB_SENSOR_PULSE_COUNTER_H
