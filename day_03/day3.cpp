#include <algorithm>
#include <cstddef>
#include <functional>
#include <generator>
#include <iostream>
#include <ranges>
#include <string_view>

std::generator<std::string_view> getBatteryLines() {
  std::string line;
  while (std::getline(std::cin, line))
    co_yield line;
}

size_t getMaxJoltage(std::string_view input) {
  char head = input[0];
  char next = input[1];
  for (auto slice : input | std::views::slide(2)) {
    if (slice[0] > head) {
      head = slice[0];
      next = slice[1];
      continue;
    }
    if (slice[1] > next) {
      next = slice[1];
    }
  }
  return static_cast<size_t>(10 * (head - '0') + (next - '0'));
}

int main() {
  size_t sum = std::ranges::fold_left(getBatteryLines() |
                                          std::views::transform(getMaxJoltage),
                                      size_t{0}, std::plus<>{});
  std::cout << "Sum of max joltages: " << sum << '\n';
  return 0;
}
