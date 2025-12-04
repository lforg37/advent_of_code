#include "utils/decimal.hpp"
#include <algorithm>
#include <cstddef>
#include <functional>
#include <generator>
#include <iostream>
#include <iterator>
#include <ranges>
#include <string_view>
#include <vector>

std::generator<std::string_view> getBatteryLines() {
  std::string line;
  while (std::getline(std::cin, line))
    co_yield line;
}

template <size_t NBattery>
constexpr size_t getMaxJoltage(std::string_view const input) {
  std::array<char, NBattery> digits{};
  std::copy_n(begin(input), NBattery, begin(digits));
  auto sizeAsInt = static_cast<int>(input.size());
  auto offset = static_cast<int>(NBattery) - sizeAsInt;
  size_t idxLastInserted{0};
  size_t posLastInserted{0};
  // We can't "reuse" the value if it is used by a higher level digit
  for (auto [idx, value] :
       input | std::views::enumerate | std::views::drop(1)) {
    auto firstBlocking =
        std::min(idx + posLastInserted - idxLastInserted, NBattery);
    auto idxInsert =
        static_cast<size_t>(std::max(0, offset + static_cast<int>(idx)));
    if (firstBlocking < idxInsert) {
      continue;
    }
    auto insertPoint =
        std::find_if(begin(digits) + idxInsert, begin(digits) + firstBlocking,
                     [value](char c) { return c < value; });
    if (insertPoint == digits.end()) {
      continue;
    }
    auto nbPreserved = std::distance(begin(digits), insertPoint);
    idxLastInserted = idx;
    posLastInserted = nbPreserved;
    std::copy_n(begin(input) + idx, NBattery - nbPreserved, insertPoint);
  }
  size_t joltage{0};
  std::for_each(begin(digits), digits.end(),
                [&joltage](char c) { utils::addLowDigit(joltage, c); });
  return joltage;
}

int main() {
  size_t sum = std::ranges::fold_left(
      getBatteryLines() | std::views::transform(getMaxJoltage<12>), size_t{0},
      std::plus<>{});
  std::cout << "Sum of max joltages: " << sum << '\n';
  return 0;
}
