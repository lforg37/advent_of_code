#include "utils/io.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <ranges>
#include <vector>

constexpr size_t getSplits(std::ranges::view auto &&lines) {
  std::vector<uint8_t> hasRay{};
  size_t nbSplits{0};
  size_t gridWidth{0};
  std::vector<uint8_t> next{};
  bool firstIter{true};
  for (auto line : lines) {
    if (firstIter) {
      hasRay = std::ranges::to<std::vector>(
          line |
          std::views::transform([](auto c) -> uint8_t { return c == 'S'; }));
      gridWidth = hasRay.size();
      firstIter = false;
      continue;
    }
    next.resize(gridWidth, false);
    for (auto [idx, c] : line | std::views::enumerate) {
      if (hasRay[idx]) {
        if (c == '^') {

          nbSplits += 1;
          if (idx > 0)
            next[idx - 1] = true;
          if (idx < static_cast<long>(gridWidth) - 1) {
            next[idx + 1] = true;
          }
          continue;
        }
        next[idx] = true;
      }
    }
    std::swap(next, hasRay);
    next.resize(0);
  }
  return nbSplits;
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

static_assert(getSplits(example | std::views::split('\n')) == 21);

int main() {
  auto getRes = getSplits(utils::getLines());
  std::cout << "Res: " << getRes << '\n';
  return 0;
}
