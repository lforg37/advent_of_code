#include <algorithm>
#include <cstdint>
#include <generator>
#include <iostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

struct Cell {
  bool full{false};
  uint8_t topFree{3};
  uint8_t freeSides{2};
  uint8_t bottomFree{3};
  constexpr uint8_t totalFreeInLine() const {
    return freeSides + (full ? 0 : 1);
  }
  constexpr uint8_t totalFreeNeighbors() const {
    return topFree + bottomFree + freeSides;
  }
  constexpr bool canBeManipulated() const {
    return full && totalFreeNeighbors() > 4;
  }
};

// Add one extraLine
std::generator<std::string_view> getLines() {
  std::string line{};
  while (std::getline(std::cin, line)) {
    co_yield line;
  }
  std::fill_n(line.begin(), line.size(), '.');
  co_yield line;
}

constexpr size_t getAvailables(std::ranges::view auto &&lines) {
  std::vector<Cell> oldCells{}, cells{};
  size_t size, sum{0};
  for (auto line : lines) {
    if (oldCells.size() == 0) {
      oldCells.resize(line.size());
      cells.resize(line.size());
      size = line.size();
    }
    std::fill_n(begin(cells), oldCells.size(), Cell{});
    for (auto [idx, filled] : line | std::views::transform([](char c) {
                                return c == '@';
                              }) | std::views::enumerate) {
      auto &curCell = cells[idx];
      auto &oldCell = oldCells[idx];
      if (filled) {
        curCell.topFree = oldCell.totalFreeInLine();
        curCell.full = true;
        oldCell.bottomFree -= 1;
        if (static_cast<size_t>(idx) + 1 < size) {
          oldCells[idx + 1].bottomFree -= 1;
        }
      }
      if (idx) {
        auto &neighBor = cells[idx - 1];
        neighBor.freeSides -= filled ? 1 : 0;
        curCell.freeSides -= neighBor.full ? 1 : 0;
        oldCells[idx - 1].bottomFree -= filled ? 1 : 0;
      }
    }
    sum += std::ranges::fold_left(
        oldCells | std::views::transform([](Cell const &cell) -> size_t {
          return cell.canBeManipulated() ? 1 : 0;
        }),
        size_t{0}, std::plus<>{});
    std::swap(oldCells, cells);
  }
  return sum;
}
constexpr std::string_view example{"..@@.@@@@."
                                   "@@@.@.@.@@"
                                   "@@@@@.@.@@"
                                   "@.@@@@..@."
                                   "@@.@@@@.@@"
                                   ".@@@@@@@.@"
                                   ".@.@.@.@@@"
                                   "@.@@@.@@@@"
                                   ".@@@@@@@@."
                                   "@.@.@@@.@."
                                   ".........."};
static_assert(getAvailables(example | std::views::chunk(10)) == 13);

int main() {
  auto sum = getAvailables(getLines());
  std::cout << "Nb movable rolls: " << sum << '\n';
}
