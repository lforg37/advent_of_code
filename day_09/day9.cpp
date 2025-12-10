
#include "utils/io.hpp"
#include <array>
#include <charconv>
#include <cstddef>
#include <iostream>
#include <limits>
#include <optional>
#include <ranges>
#include <span>
#include <string_view>
#include <vector>

using coord_t = std::array<size_t, 2>;
constexpr coord_t getFromView(std::string_view sv) {
  coord_t res{};
  auto parseRes = std::from_chars(sv.data(), sv.data() + sv.size(), res[0]);
  std::from_chars(parseRes.ptr + 1, sv.data() + sv.size(), res[1]);
  return res;
}

constexpr size_t getMaxArea(std::ranges::view auto &&values) {
  std::vector<std::optional<size_t>> minOnLine;
  std::vector<std::optional<size_t>> maxOnLine;
  std::vector<size_t> idxs;
  size_t minY{std::numeric_limits<size_t>::max()};
  size_t maxY{0};
  for (auto [x, y] : values) {
    if (x >= minOnLine.size()) {
      minOnLine.resize(x + 1, std::nullopt);
      maxOnLine.resize(x + 1, std::nullopt);
    }
    if (!minOnLine[x]) {
      minOnLine[x] = y;
      maxOnLine[x] = y;
      idxs.push_back(x);
    } else {
      minOnLine[x] = std::min(y, *minOnLine[x]);
      maxOnLine[x] = std::max(y, *maxOnLine[x]);
    }
    minY = std::min(minY, y);
    maxY = std::max(maxY, y);
  }
  std::sort(begin(idxs), end(idxs));
  size_t max{0};
  size_t lineWidthBound = maxY - minY + 1;
  for (size_t minXID{0}; minXID < idxs.size(); ++minXID) {
    size_t minX = idxs[minXID];
    auto minYminX = *minOnLine[minX];
    auto maxYminX = *maxOnLine[minX];
    for (size_t maxXID{idxs.size() - 1}; maxXID > minXID; --maxXID) {
      size_t maxX = idxs[maxXID];
      auto XDiff = maxX - minX + 1;
      // Bound
      if ((XDiff)*lineWidthBound <= max)
        break;
      auto minYmaxX = *minOnLine[maxX];
      auto maxYmaxX = *maxOnLine[maxX];

      if (minYmaxX < maxYminX)
        max = std::max(max, XDiff * (maxYminX - minYmaxX + 1));
      if (minYminX < maxYmaxX)
        max = std::max(max, XDiff * (maxYmaxX - minYminX + 1));
    }
  }
  return max;
}

constexpr std::array<coord_t, 8> example{
    {{7, 1}, {11, 1}, {11, 7}, {9, 7}, {9, 5}, {2, 5}, {2, 3}, {7, 3}}};

static_assert(getMaxArea(example | std::views::all) == 50);

int main() {
  size_t res =
      getMaxArea(utils::getLines() | std::views::transform(getFromView));
  std::cout << "Res: " << res << '\n';
}
