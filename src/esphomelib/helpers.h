//
// Created by Otto Winter on 25.11.17.
//

#ifndef ESPHOMELIB_HELPERS_H
#define ESPHOMELIB_HELPERS_H

#include <string>
#include <IPAddress.h>
#include <memory>
#include <queue>
#include <functional>
#include <ArduinoJson.h>

#include "esphomelib/esphal.h"
#include "esphomelib/defines.h"
#include "esphomelib/optional.h"

#ifndef JSON_BUFFER_SIZE
  #define JSON_BUFFER_SIZE (JSON_OBJECT_SIZE(32))
#endif

#ifdef ARDUINO_ARCH_ESP32
  #include <driver/rmt.h>
#endif

ESPHOMELIB_NAMESPACE_BEGIN

/// Callback function typedef for parsing JsonObjects.
using json_parse_t = std::function<void(JsonObject &)>;

/// Callback function typedef for building JsonObjects.
using json_build_t = std::function<void(JsonBuffer &, JsonObject &)>;

/// The characters that are allowed in a hostname.
extern const char *HOSTNAME_CHARACTER_WHITELIST;

/// Gets the MAC address as a string, this can be used as way to identify this ESP32.
std::string get_mac_address();

/// Constructs a hostname by concatenating base, a hyphen, and the MAC address.
std::string generate_hostname(const std::string &base);

/// Sanitize the hostname by removing characters that are not in the whitelist and truncating it to 63 chars.
std::string sanitize_hostname(const std::string &hostname);

/// Truncate a string to a specific length
std::string truncate_string(const std::string &s, size_t length);

/// Checks whether the provided IPAddress is empty (is 0.0.0.0).
bool is_empty(const IPAddress &address);

/// Force a shutdown (and reboot) of the ESP, calling any registered shutdown hooks.
void reboot(const char *cause);

/// Add a shutdown callback.
void add_shutdown_hook(std::function<void(const char *)> &&f);

/// Create a safe shutdown (and reboot) of the ESP, calling any registered shutdown and safe shutdown hooks.
void safe_reboot(const char *cause);

/// Run shutdown hooks.
void run_shutdown_hooks(const char *cause);

/// Add a safe shutdown callback that will be called if the device is shut down intentionally.
void add_safe_shutdown_hook(std::function<void(const char *)> &&f);

/// Run safe shutdown and force shutdown hooks.
void run_safe_shutdown_hooks(const char *cause);

/// Convert the string to lowercase_underscore.
std::string to_lowercase_underscore(std::string s);

/// Build a JSON string with the provided json build function.
std::string build_json(const json_build_t &f);

/// Parse a JSON string and run the provided json parse function if it's valid.
void parse_json(const std::string &data, const json_parse_t &f);

/** Clamp the value between min and max.
 *
 * @tparam T The input/output typename.
 * @param min The minimum value.
 * @param max The maximum value.
 * @param val The value.
 * @return val clamped in between min and max.
 */
template<typename T>
T clamp(T min, T max, T val);

/** Linearly interpolate between end start and end by completion.
 *
 * @tparam T The input/output typename.
 * @param start The start value.
 * @param end The end value.
 * @param completion The completion. 0 is start value, 1 is end value.
 * @return The linearly interpolated value.
 */
template<typename T>
T lerp(T start, T end, T completion);

/// std::make_unique
template<typename T, typename ...Args>
std::unique_ptr<T> make_unique(Args &&...args);

/// Return a random 32 bit unsigned integer.
uint32_t random_uint32();

/** Returns a random double between 0 and 1.
 *
 * Note: This function probably doesn't provide a truly uniform distribution.
 */
double random_double();

/// Returns a random float between 0 and 1. Essentially just casts random_double() to a float.
float random_float();

/// Applies gamma correction with the provided gamma to value.
float gamma_correct(float value, float gamma);

/// Create a string from a value and an accuracy in decimals.
std::string value_accuracy_to_string(float value, int8_t accuracy_decimals);

/// Convert a uint64_t to a hex string
std::string uint64_to_string(uint64_t num);

/// Convert a uint32_t to a hex string
std::string uint32_to_string(uint32_t num);

/// Sanitizes the input string with the whitelist.
std::string sanitize_string_whitelist(const std::string &s, const std::string &whitelist);

/** Cross-platform method to disable interrupts.
 *
 * Useful when you need to do some timing-dependent communication.
 *
 * @see Do not forget to call `enable_interrupts()` again or otherwise things will go very wrong.
 */
void disable_interrupts();

/// Cross-platform method to enable interrupts after they have been disabled.
void enable_interrupts();

/// Calculate a crc8 of data with the provided data length.
uint8_t crc8(uint8_t *data, uint8_t len);

optional<bool> parse_on_off(const char *str, const char *payload_on = "on", const char *payload_off = "off");

/// Helper class that implements a sliding window moving average.
template<typename T>
class SlidingWindowMovingAverage {
 public:
  /** Create the SlidingWindowMovingAverage.
   *
   * @param max_size The window size.
   */
  explicit SlidingWindowMovingAverage(size_t max_size);

  /** Add value to the interval buffer.
   *
   * @param value The value.
   * @return The new average.
   */
  T next_value(T value);

  /// Return the average across the sliding window.
  T calculate_average();

  size_t get_max_size() const;
  void set_max_size(size_t max_size);

 protected:
  std::queue<T> queue_;
  size_t max_size_;
  T sum_;
};

/// Helper class that implements an exponential moving average.
class ExponentialMovingAverage {
 public:
  explicit ExponentialMovingAverage(float alpha);

  float next_value(float value);

  float calculate_average();

  void set_alpha(float alpha);
  float get_alpha() const;

 protected:
  float alpha_;
  float accumulator_;
};

template<typename... X> class CallbackManager;


/** Simple helper class to allow having multiple subscribers to a signal.
 *
 * @tparam Ts The arguments for the callback, wrapped in void().
 */
template<typename... Ts>
class CallbackManager<void(Ts...)> {
 public:
  /// Add a callback to the internal callback list.
  void add(std::function<void(Ts...)> &&callback);

  /// Call all callbacks in this manager.
  void call(Ts... args);

 protected:
  std::vector<std::function<void(Ts...)>> callbacks_;
};

template<typename T, typename X>
class TemplatableValue {
 public:
  TemplatableValue() : type_(EMPTY) {

  }

  TemplatableValue(T const &value) : type_(VALUE), value_(value) {

  }

  TemplatableValue(std::function<T(X)> f) : type_(LAMBDA), f_(f) {

  }

  bool has_value() {
    return this->type_ != EMPTY;
  }

  T value(X x) {
    if (this->type_ == LAMBDA) {
      return this->f_(x);
    } else {
      // return value also when empty
      return this->value_;
    }
  }

 protected:
  enum {
    EMPTY,
    VALUE,
    LAMBDA,
  } type_;

  T value_;
  std::function<T(X)> f_;
};

extern CallbackManager<void(const char *)> shutdown_hooks;
extern CallbackManager<void(const char *)> safe_shutdown_hooks;

#ifdef ARDUINO_ARCH_ESP32
  extern rmt_channel_t next_rmt_channel;

  rmt_channel_t select_next_rmt_channel();
#endif

void delay_microseconds_accurate(uint32_t usec);

// ================================================
//                 Definitions
// ================================================

template<typename T>
T clamp(T min, T max, T val) {
  if (min > max) std::swap(min, max);
  if (val < min) return min;
  if (val > max) return max;
  return val;
}

template<typename T>
T lerp(T start, T end, T completion) {
  return start + (end - start) * completion;
}

template<typename T, typename ...Args>
std::unique_ptr<T> make_unique(Args &&...args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template<typename T>
SlidingWindowMovingAverage<T>::SlidingWindowMovingAverage(size_t max_size) : max_size_(max_size), sum_(0) {

}

template<typename T>
T SlidingWindowMovingAverage<T>::next_value(T value) {
  if (isnan(value)) {
    // protect from NAN values
    return this->calculate_average();
  }
  if (this->queue_.size() == this->max_size_) {
    this->sum_ -= this->queue_.front();
    this->queue_.pop();
  }
  this->queue_.push(value);
  this->sum_ += value;

  return this->calculate_average();
}
template<typename T>
T SlidingWindowMovingAverage<T>::calculate_average() {
  if (this->queue_.size() == 0)
    return 0;
  else
    return this->sum_ / this->queue_.size();
}

template<typename T>
size_t SlidingWindowMovingAverage<T>::get_max_size() const {
  return this->max_size_;
}

template<typename T>
void SlidingWindowMovingAverage<T>::set_max_size(size_t max_size) {
  this->max_size_ = max_size;

  while (this->queue_.size() > max_size) {
    this->sum_ -= this->queue_.front();
    this->queue_.pop();
  }
}

template<typename... Ts>
void CallbackManager<void(Ts...)>::add(std::function<void(Ts...)> &&callback) {
  this->callbacks_.push_back(std::move(callback));
}
template<typename... Ts>
void CallbackManager<void(Ts...)>::call(Ts... args) {
  for (auto &cb : this->callbacks_)
    cb(args...);
}

ESPHOMELIB_NAMESPACE_END

#endif //ESPHOMELIB_HELPERS_H
