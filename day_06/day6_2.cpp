#include "utils/io.hpp"
#include <algorithm>
#include <cstddef>
#include <functional>
#include <iostream>
#include <ranges>
#include <string_view>
#include <vector>

constexpr size_t getResult(std::ranges::view auto &&lines) {
  std::vector<size_t> elems;
  std::vector<bool> gotNumber;
  std::string_view operators{};
  for (auto const &line : lines) {
    if (line[0] == '*' || line[0] == '+') {
      operators = std::string_view{&line[0], line.size()};
      break;
    }
    if (elems.empty()) {
      elems.resize(line.size(), 0);
      gotNumber.resize(line.size(), false);
    }
    for (auto [i, c] : line | std::views::enumerate) {
      if (c == ' ')
        continue;
      elems[i] *= 10;
      gotNumber[i] = true;
      elems[i] += c - '0';
    }
  }
  auto numGroups = std::views::zip(elems, gotNumber) |
                   std::views::chunk_by([](auto val1, auto val2) {
                     return !(std::get<1>(val1) && !std::get<1>(val2));
                   });
  auto ops = operators | std::views::filter([](char c) { return c != ' '; });
  auto subResults =
      std::views::zip(numGroups, ops) | std::views::transform([](auto v) {
        auto [group, op] = v;
        auto values =
            group | std::ranges::views::filter([](auto elem) {
              return std::get<1>(elem);
            }) |
            std::views::transform([](auto elem) { return std::get<0>(elem); });
        size_t res{0};
        switch (op) {
        case '*':
          res = std::ranges::fold_left(values, size_t{1}, std::multiplies<>{});
          break;
        case '+':
          res = std::ranges::fold_left(values, size_t{0}, std::plus<>{});
          break;
        }
        return res;
      });
  return std::ranges::fold_left(subResults, size_t{0}, std::plus<>{});
}

constexpr std::string_view example{"123 328  51 64 \n"
                                   " 45 64  387 23 \n"
                                   "  6 98  215 314\n"
                                   "*   +   *   + "};

static_assert(getResult(example | std::views::split('\n')) == 3263827);

int main() {
  auto res = getResult(utils::getLines());
  std::cout << "Got result: " << res << '\n';
}
