#ifndef UTILS_IO_HPP
#define UTILS_IO_HPP

#include <generator>
#include <iostream>
#include <string>
#include <string_view>

namespace utils {
inline std::generator<std::string_view> getLines() {
  std::string value{};
  while (std::getline(std::cin, value)) {
    co_yield value;
  }
}
} // namespace utils

#endif // UTILS_IO_HPP
