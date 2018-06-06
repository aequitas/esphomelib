//
// Created by Otto Winter on 01.12.17.
//

#ifndef ESPHOMELIB_SWITCH_IR_TRANSMITTER_COMPONENT_H
#define ESPHOMELIB_SWITCH_IR_TRANSMITTER_COMPONENT_H

#include "esphomelib/component.h"
#include "esphomelib/switch_/switch.h"
#include "esphomelib/esphal.h"
#include "esphomelib/defines.h"

//#ifdef USE_IR_TRANSMITTER

#ifdef ARDUINO_ARCH_ESP32
  #include <driver/rmt.h>
#endif

ESPHOMELIB_NAMESPACE_BEGIN

namespace switch_ {

/// Namespace storing constants for vendor IR formats
namespace ir {

namespace nec {

const uint32_t CARRIER_FREQUENCY_HZ = 38000;
const uint32_t HEADER_HIGH_US = 9000;
const uint32_t HEADER_LOW_US = 4500;
const uint32_t BIT_HIGH_US = 560;
const uint32_t BIT_ONE_LOW_US = 1690;
const uint32_t BIT_ZERO_LOW_US = 560;

} // namespace nec

namespace lg {

const uint32_t CARRIER_FREQUENCY_HZ = 38000;
const uint32_t HEADER_HIGH_US = 8000;
const uint32_t HEADER_LOW_US = 4000;
const uint32_t BIT_HIGH_US = 600;
const uint32_t BIT_ONE_LOW_US = 1600;
const uint32_t BIT_ZERO_LOW_US = 550;

} // namespace lg

namespace sony {

const uint32_t CARRIER_FREQUENCY_HZ = 40000;
const uint32_t HEADER_HIGH_US = 2400;
const uint32_t HEADER_LOW_US = 600;
const uint32_t BIT_ONE_HIGH_US = 1200;
const uint32_t BIT_ZERO_HIGH_US = 600;
const uint32_t BIT_LOW_US = 600;

} // namespace sony

namespace panasonic {

const uint32_t CARRIER_FREQUENCY_HZ = 35000;
const uint32_t HEADER_HIGH_US = 3502;
const uint32_t HEADER_LOW_US = 1750;
const uint32_t BIT_HIGH_US = 502;
const uint32_t BIT_ZERO_LOW_US = 400;
const uint32_t BIT_ONE_LOW_US = 1244;

} // namespace panasonic

/** Struct that stores the raw data for sending out IR codes.
 *
 * Internally it stores some fields like carrier frequency and a data field, that
 * stores the raw IR codes as an array.
 */
struct SendData {
  uint32_t carrier_frequency; ///< The carrier frequency of the IR protocol.
  std::vector<int16_t> data; ///< Raw IR data, negative means off, positive means on.
  uint16_t repeat_times{1}; ///< How often to perform this data set.
  uint16_t repeat_wait; ///< How long to wait between repeats.

#ifdef ARDUINO_ARCH_ESP32
  /// Return the internal data as an RMT interface-compatible vector.
  std::vector<rmt_item32_t> get_rmt_data(uint16_t ticks_for_10_us);
#endif

  /// Push back a high value for the specified duration to the data.
  void mark(uint16_t duration_us);
  /// Push back a low value for the specified duration to the data.
  void space(uint16_t duration_us);
  /// Add a high+low value for the specified durations to the data.
  void add_item(uint16_t high_us, uint16_t low_us);

  /// Repeat this SendData with the specified wait time between sends.
  SendData repeat(uint16_t times, uint16_t wait_us = 30000);

  /// Calculate the total length required to run this IR code.
  uint32_t total_length_ms() const;

  /// Construct a SendData from an NEC address+command.
  static SendData from_nec(uint16_t address, uint16_t command);
  /// Construct a SendData from LG data (with an optional number of bits).
  static SendData from_lg(uint32_t data, uint8_t nbits = 28);
  /// Construct a SendData from Sony data (with an optional number of bits).
  static SendData from_sony(uint32_t data, uint8_t nbits = 12);
  /// Construct a SendData from Panasonic address+data.
  static SendData from_panasonic(uint16_t address, uint32_t data);
  /// Construct a SendData from a raw data array with specified carrier frequency..
  static SendData from_raw(std::vector<int> &raw_data, uint32_t carrier_frequency);
};

} // namespace ir

/// APB base clock speed.
const uint32_t BASE_CLOCK_HZ = 80000000;
/// Default clock divider we use for RMT.
const uint8_t DEFAULT_CLOCK_DIVIDER = 80;
/// Default carrier frequency we setup RMT with.
const uint32_t DEFAULT_CARRIER_FREQUENCY_HZ = 38000;

/** This is an IR Transmitter hub using the ESP32 RMT peripheral.
 *
 * With it you can send out IR commands from different vendors and have them appear
 * as switches in Home Assistant.
 */
class IRTransmitterComponent : public Component {
 public:
  /** Construct the IR Transmitter with the specified pin (can be inverted).
   *
   * @param pin The pin for this IR Transmitter.
   * @param carrier_duty_percent The duty percentage (from 0-100) for this transmitter.
   */
  explicit IRTransmitterComponent(GPIOPin *pin,
                                  uint8_t carrier_duty_percent = 50);

  class DataTransmitter;

  /** Create a DataTransmitter from the specified SendData and register it in the transmitter.
   *
   * @param send_data The SendData.
   * @return A DataTransmitter which can be used as a Switch.
   */
  DataTransmitter *create_transmitter(const std::string &name, const ir::SendData &send_data);

  /// Set the carrier duty percentage (from 0-100).
  void set_carrier_duty_percent(uint8_t carrier_duty_percent);

  // ========== INTERNAL METHODS ==========
  // (In most use cases you won't need these)
  /// Internal helper class that exposes a SendData as a Switch.
  class DataTransmitter : public switch_::Switch {
   public:
    DataTransmitter(const std::string &name, const ir::SendData &send_data,
                    IRTransmitterComponent *parent);

    void turn_on() override;
    void turn_off() override;

    std::string icon() override;
   protected:
    ir::SendData send_data_;
    IRTransmitterComponent *parent_;
  };

  /// Setup the RMT peripheral.
  void setup() override;
  /// HARDWARE_LATE setup priority.
  float get_setup_priority() const override;

  /// Send an IR SendData object on this pin.
  void send(ir::SendData &send_data);

#ifdef ARDUINO_ARCH_ESP32
  /// Set the clock divier for RMT.
  void set_clock_divider(uint8_t clock_divider);

  /// Set the RMT channel used.
  void set_channel(rmt_channel_t channel);
#endif

 protected:
#ifdef ARDUINO_ARCH_ESP32
  /// Setup the RMT peripheral for the specified carrier frequency, if it's already the used frequency, does nothing.
  void require_carrier_frequency(uint32_t carrier_frequency);

  /// Get the number of ticks (from the clock divider) for 10 µs.
  uint16_t get_ticks_for_10_us();

  /// Configure the RMT peripheral using the internal information.
  void configure_rmt();

  rmt_channel_t channel_{RMT_CHANNEL_0};
  uint8_t clock_divider_{DEFAULT_CLOCK_DIVIDER};
  uint32_t last_carrier_frequency_{DEFAULT_CARRIER_FREQUENCY_HZ};
#endif

#ifdef ARDUINO_ARCH_ESP8266
  void calculate_on_off_time_(uint32_t carrier_frequency, uint32_t *on_time_period, uint32_t *off_time_period);

  void delay_microseconds_accurate_(uint32_t usec);

  void mark_(uint32_t on_time, uint32_t off_time, uint32_t usec);

  void space_(uint32_t usec);
#endif

  GPIOPin *pin_;
  uint8_t carrier_duty_percent_;
};

} // namespace switch_

ESPHOMELIB_NAMESPACE_END

//#endif //USE_IR_TRANSMITTER

#endif //ESPHOMELIB_SWITCH_IR_TRANSMITTER_COMPONENT_H
