#include "utils/io.hpp"
#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <iostream>
#include <ranges>
#include <string_view>
#include <vector>

struct Machine {
  uint64_t mask;
  // Button i has xor buttons[i] with mask;
  std::vector<uint64_t> buttons;
  constexpr static Machine from(std::string_view sv) {
    uint64_t mask{0};
    std::vector<uint64_t> buttons;
    for (auto val :
         sv | std::views::drop(1) |
             std::views::take_while([](auto info) { return info != ']'; }) |
             std::views::reverse |
             std::views::transform([](char c) { return uint64_t{c == '#'}; })) {
      mask <<= 1;
      mask |= val;
    }
    for (auto parGroup :
         sv | std::views::drop_while([](char c) { return c != '('; }) |
             std::views::take_while([](auto c) { return c != '{'; }) |
             std::views::split(' ') | std::views::filter([](auto parGroup) {
               return !parGroup.empty();
             })) {
      uint64_t buttonMap{0};
      for (auto val :
           parGroup | std::views::drop(1) |
               std::views::take(parGroup.size() - 2) | std::views::split(',') |
               std::views::transform([](auto view) {
                 uint64_t res{0};
                 std::from_chars(&view[0], &view[0] + view.size(), res);
                 return res;
               })) {
        buttonMap |= uint64_t{1} << val;
      }
      buttons.push_back(buttonMap);
    }
    return {.mask = mask, .buttons = buttons};
  }
  constexpr size_t getNbClicks() const {
    struct maskCombination {
      size_t nbClicks;
      uint64_t producedMask;
      uint16_t usedSubMasks;
    };

    std::array<maskCombination, 1 << 12> index{maskCombination{0, 0, 0}};
    std::deque<size_t> novelties{};
    novelties.push_back(0);
    while (!novelties.empty()) {
      auto const &candidate = index[novelties.front()];
      novelties.pop_front();
      for (auto const &[idx, buttonMask] : buttons | std::views::enumerate) {
        uint16_t idMask = 1 << idx;
        if (candidate.usedSubMasks & idMask)
          continue;
        auto subMask = candidate.producedMask ^ buttonMask;
        if (subMask && index[subMask].nbClicks == 0) {
          if (subMask == mask)
            return candidate.nbClicks + 1;
          index[subMask] = {.nbClicks = candidate.nbClicks + 1,
                            .producedMask = subMask,
                            .usedSubMasks = static_cast<uint16_t>(
                                candidate.usedSubMasks ^ idMask)};
          novelties.push_back(subMask);
        }
      }
    }
    auto printer = [&index](size_t i) {
      auto const &pattern = index[i];
      std::cout << std::format(
          "Pattern {0} ({0:b}) : needs button {1:b} (needs {2} clicks)\n",
          pattern.producedMask, pattern.usedSubMasks, pattern.nbClicks);
    };
    printer(0);
    for (size_t i = 0; i < index.size(); ++i) {
      if (index[i].nbClicks)
        printer(i);
    }
    return -1;
  }
};

int main() {
  size_t res{0};
  for (auto const &machine :
       utils::getLines() | std::views::transform(Machine::from))
    res += machine.getNbClicks();
  std::cout << std::format("Res: {}\n", res);
  return 0;
}
