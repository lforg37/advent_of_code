#include "utils/io.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <execution>
#include <functional>
#include <iostream>
#include <iterator>
#include <numeric>
#include <ranges>
#include <vector>

constexpr size_t getSplits(std::ranges::view auto &&lines) {
  std::vector<uint64_t> hasRay{};
  size_t gridWidth{0};
  std::vector<uint64_t> next{};
  bool firstIter{true};
  for (auto line : lines) {
    if (firstIter) {
      hasRay = std::ranges::to<std::vector>(
          line |
          std::views::transform([](auto c) -> uint64_t { return c == 'S'; }));
      gridWidth = hasRay.size();
      firstIter = false;
      continue;
    }
    next.resize(gridWidth, 0);
    for (auto [idx, c] : line | std::views::enumerate) {
      if (hasRay[idx]) {
        if (c == '^') {
          if (idx > 0)
            next[idx - 1] += hasRay[idx];
          if (idx < static_cast<long>(gridWidth) - 1) {
            next[idx + 1] += hasRay[idx];
          }
          continue;
        }
        next[idx] += hasRay[idx];
      }
    }
    std::swap(next, hasRay);
    next.resize(0);
  }
  return std::reduce(std::begin(hasRay), std::end(hasRay), uint64_t{0},
                     std::plus<>{});
}

constexpr std::string_view example{".......S.......\n"
                                   "...............\n"
                                   ".......^.......\n"
                                   "...............\n"
                                   "......^.^......\n"
                                   "...............\n"
                                   ".....^.^.^.....\n"
                                   "...............\n"
                                   "....^.^...^....\n"
                                   "...............\n"
                                   "...^.^...^.^...\n"
                                   "...............\n"
                                   "..^...^.....^..\n"
                                   "...............\n"
                                   ".^.^.^.^.^...^.\n"
                                   "..............."};

static_assert(getSplits(example | std::views::split('\n')) == 40);

int main() {
  auto getRes = getSplits(utils::getLines());
  std::cout << "Res: " << getRes << '\n';
  return 0;
}
