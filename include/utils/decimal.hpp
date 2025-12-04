#ifndef UTILS_DECIMAL_HPP
#define UTILS_DECIMAL_HPP

#include <algorithm>
#include <array>
#include <concepts>

namespace utils {
template <std::integral T> constexpr T getDecimalDigits(T value) {
  T result{0};
  do {
    result += 1;
    value /= 10;
  } while (value != 0);
  return result;
}

template <std::unsigned_integral T> constexpr auto powersOfTen() {
  std::array<T, std::numeric_limits<T>::digits10 + 1> result{};
  std::generate_n(result.begin(), result.size(), [n = T{1}]() mutable {
    auto res{n};
    n *= 10;
    return res;
  });
  return result;
}

template <std::unsigned_integral T>
inline constexpr void addLowDigit(T &value, char digit) {
  value *= 10;
  value += digit - '0';
}

template <std::unsigned_integral T>
inline constexpr T addLowDigit(T &&value, char digit) {
  addLowDigit(value, digit);
  return value;
}

static_assert(addLowDigit(0u, '7') == 7u);
} // namespace utils

#endif // UTILS_DECIMAL_HPP
