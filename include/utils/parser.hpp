#ifndef UTILS_PARSER_HPP
#define UTILS_PARSER_HPP

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <string_view>

namespace utils {
class Parser {
public:
  constexpr Parser(std::string_view input) : data(input) {};
  constexpr uint_fast32_t getUnsignedInt() {
    uint_fast32_t result{0};
    size_t index{0};
    for (auto const c : data | std::views::take_while([](char c) {
                          return c >= '0' && c <= '9';
                        })) {
      result = result * 10 + static_cast<uint_fast32_t>(c - '0');
      index += 1;
    }
    data = data.substr(index);
    return result;
  }
  constexpr std::string_view drop(size_t n) {
    auto result = data.substr(0, std::min(n, data.size()));
    data = data.substr(std::min(n, data.size()));
    return result;
  }
  constexpr size_t eat(char toEat) {
    size_t res{0};
    for (auto c : data) {
      if (c != toEat)
        break;
      res += 1;
    }
    drop(res);
    return res;
  }
  constexpr bool empty() const { return data.empty(); }

private:
  std::string_view data;
};
} // namespace utils

#endif // UTILS_PARSER_HPP
