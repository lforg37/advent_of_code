#include "utils/io.hpp"
#include "utils/parser.hpp"
#include <algorithm>
#include <cassert>
#include <functional>
#include <generator>
#include <iostream>
#include <iterator>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

constexpr void getEntries(std::span<const char> line,
                          std::vector<size_t> &res) {
  utils::Parser p{std::string_view{&line[0], line.size()}};
  res.resize(0);
  while (!p.empty()) {
    p.eat(' ');
    if (p.empty())
      return;
    res.push_back(p.getUnsignedInt());
  }
}

namespace detail {
constexpr bool checkEntries(std::string_view input,
                            std::vector<size_t> expectedRes) {
  std::vector<size_t> actual{};
  getEntries(input, actual);
  return actual == expectedRes;
}
static_assert(checkEntries("  14  ", {14}));
static_assert(checkEntries("  14 17 ", {14, 17}));
static_assert(checkEntries("14 17 ", {14, 17}));
static_assert(checkEntries("14 17", {14, 17}));
static_assert(checkEntries(" 14   17", {14, 17}));
} // namespace detail

constexpr std::size_t getResult(std::ranges::view auto &&lines) {
  std::vector<size_t> valueBuffer{};
  std::vector<size_t> sums{};
  std::vector<size_t> prods{};

  for (auto line : lines) {
    if (line[0] == '*' || line[0] == '+') {
      size_t res{0};
      for (auto [i, op] : line | std::views::filter([](auto c) {
                            return c != ' ';
                          }) | std::views::enumerate) {
        switch (op) {
        case '*':
          res += prods[i];
          break;
        case '+':
          res += sums[i];
          break;
        }
      }
      return res;
    }
    getEntries(line, valueBuffer);
    if (sums.empty()) {
      sums = prods = valueBuffer;
      continue;
    }
    for (auto i : std::views::iota(size_t{0}, valueBuffer.size())) {
      sums[i] += valueBuffer[i];
      prods[i] *= valueBuffer[i];
    }
  }
  return 0;
}

constexpr std::string_view example{"123 328  51 64 \n"
                                   " 45 64  387 23 \n"
                                   "  6 98  215 314\n"
                                   "*   +   *   + "};

static_assert(getResult(example | std::views::split('\n')) == 4277556);

int main() {
  auto res = getResult(utils::getLines());
  std::cout << "Control sum: " << res << '\n';
}
