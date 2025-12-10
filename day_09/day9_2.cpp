#include "utils/io.hpp"
#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <format>
#include <iostream>
#include <iterator>
#include <ranges>
#include <string_view>
#include <vector>

using coord_t = std::array<size_t, 2>;
constexpr coord_t getFromView(std::string_view sv) {
  coord_t res{};
  auto parseRes = std::from_chars(sv.data(), sv.data() + sv.size(), res[0]);
  std::from_chars(parseRes.ptr + 1, sv.data() + sv.size(), res[1]);
  return res;
}

struct GridPoly {
  enum struct Direction { Clockwise = 1, CounterClockWise = -1 };

  constexpr void addPoint(coord_t &&value) {
    points.emplace_back(std::move(value));
  }

  constexpr size_t getXIdx(size_t x) const {
    return std::distance(begin(xValues),
                         std::lower_bound(begin(xValues), end(xValues), x));
  }

  constexpr size_t getYIdx(size_t y) const {
    return std::distance(begin(yValues),
                         std::lower_bound(begin(yValues), end(yValues), y));
  }

  constexpr size_t getNextId(size_t pointId) const {
    return (pointId + 1) % points.size();
  }

  constexpr size_t getPrevId(size_t pointId) const {
    return (pointId + points.size() - 1) % points.size();
  }

  constexpr size_t getIndexedArea(coord_t left, coord_t right) const {
    auto mx = std::min(left[0], right[0]);
    auto Mx = std::max(left[0], right[0]);
    auto my = std::min(left[1], right[1]);
    auto My = std::max(left[1], right[1]);
    return (xValues[Mx] - xValues[mx] + 1) * (yValues[My] - yValues[my] + 1);
  }

  constexpr size_t getNextClockWise(size_t pointId) const {
    if (dir == Direction::Clockwise)
      return getNextId(pointId);
    return getPrevId(pointId);
  }

  constexpr size_t getPrevClockWise(size_t pointId) const {
    if (dir == Direction::CounterClockWise)
      return getNextId(pointId);
    return getPrevId(pointId);
  }

  // Will index the points on an irregular grid and compute
  // the histograms per y on this grid
  constexpr void buildIndex() {
    for (auto [x, y] : points) {
      auto xPtr = std::lower_bound(begin(xValues), end(xValues), x);
      if (xPtr == end(xValues) || *xPtr != x) {
        xValues.insert(xPtr, x);
      }
      auto yPtr = std::lower_bound(begin(yValues), end(yValues), y);
      if (yPtr == end(yValues) || *yPtr != y) {
        yValues.insert(yPtr, y);
      }
    }

    for (auto &point : points) {
      auto indexedX = std::distance(
          begin(xValues),
          std::lower_bound(begin(xValues), end(xValues), point[0]));
      auto indexedY = std::distance(
          begin(yValues),
          std::lower_bound(begin(yValues), end(yValues), point[1]));
      point[0] = indexedX;
      point[1] = indexedY;
    }

    pointsByY.resize(yValues.size());
    for (auto [idx, p] : points | std::views::all | std::views::enumerate) {
      auto &yLine = pointsByY[p[1]];
      auto xPtr = std::lower_bound(
          begin(yLine), end(yLine), idx,
          [this](size_t l, size_t r) { return points[l][0] < points[r][0]; });
      yLine.insert(xPtr, idx);
    }
    {
      auto lowerCornerIdx = pointsByY[0][0];
      auto next = getNextId(lowerCornerIdx);
      dir = Direction::CounterClockWise;
      if (points[lowerCornerIdx][0] == points[next][0])
        dir = Direction::Clockwise;
    }

    pointsCrossingY.resize(yValues.size());
    for (auto id : std::views::iota(0) | std::views::take(points.size())) {
      auto p0 = points[id];
      auto p1 = points[getPrevClockWise(id)];
      auto yM = std::max(p0[1], p1[1]);
      auto ym = std::min(p0[1], p1[1]);
      if (yM == ym)
        continue;
      for (size_t i = ym; i < yM + 1; ++i) {
        pointsCrossingY[i].push_back(id);
      }
    }

    for (auto &crossingList : pointsCrossingY) {
      std::sort(begin(crossingList), end(crossingList),
                [this](size_t l, size_t r) { return points[l] < points[r]; });
    }

    subHeights.reserve(yValues.size());
    size_t nbXCells = xValues.size() - 1;
    for (auto [yIdxLong, yBounds] :
         yValues | std::views::slide(2) | std::views::enumerate) {
      auto yIdx = static_cast<size_t>(yIdxLong);
      auto &curLine = subHeights.emplace_back();
      curLine.resize(nbXCells, 0);
      auto rangeHeight = yBounds[1] - yBounds[0] + 1;
      auto const &curCrossing = pointsCrossingY[yIdx];
      bool startIn{false};
      size_t oldIdx{0};
      size_t enable = startIn ? 1 : 0;
      for (auto crossPoint : curCrossing) {
        auto const &point = points[crossPoint];
        for (size_t i = oldIdx; i < point[0]; ++i) {
          curLine[i] = enable * rangeHeight;
        }
        oldIdx = point[0];
        auto const &prev = points[getPrevClockWise(crossPoint)];
        auto mY = std::min(point[1], prev[1]);
        auto MY = std::max(point[1], prev[1]);

        auto cornerCase = MY == yIdx || mY == yIdx;
        if (!cornerCase || mY == yIdx) {
          enable = 1 - enable;
        }
      }
    }
  }
  std::vector<coord_t> points;
  std::vector<std::vector<size_t>> pointsByY;
  std::vector<std::vector<size_t>> pointsCrossingY;
  std::vector<size_t> xValues;
  std::vector<size_t> yValues;
  std::vector<std::vector<size_t>> subHeights;
  Direction dir;
};

constexpr size_t getMaxArea(std::ranges::view auto &&values) {
  GridPoly gp{};
  for (auto coord : values) {
    gp.addPoint(std::move(coord));
  }
  gp.buildIndex();

  std::vector<size_t> histogram{};
  size_t nbins = gp.subHeights[0].size();
  histogram.resize(nbins, 0);
  size_t globalMax{1};

  for (auto yCand :
       std::views::iota(size_t{0}) | std::views::take(gp.subHeights.size())) {
    auto const &candLine = gp.pointsByY[yCand];
    auto slice = gp.subHeights[yCand];
    std::vector<int> prevNull{}, nextNull{};
    prevNull.resize(candLine.size(), 0);
    nextNull.resize(candLine.size(), 0);
    auto getXFromId = [&gp](size_t id) { return gp.points[id][0]; };
    auto updateLimits = [&slice, &nextNull, &prevNull, &candLine, &getXFromId] {
      prevNull.resize(0);
      prevNull.resize(candLine.size(), -1);
      int curLastNull{-1};
      auto oldId = 0;
      std::vector<size_t> idToUpdate{};
      for (auto [idx, candId] : candLine | std::views::transform(getXFromId) |
                                    std::views::enumerate) {
        for (size_t i = oldId; i < candId; ++i) {
          auto val =
              static_cast<size_t>(i) >= slice.size() ? slice.back() : slice[i];
          if (!val) {
            curLastNull = i;
          }
        }
        prevNull[idx] = curLastNull;
        oldId = candId;
      }

      nextNull.resize(0);
      nextNull.resize(candLine.size(), slice.size());
      size_t candLineIdx{0};
      for (auto [binId, binVal] : slice | std::views::enumerate) {
        if (!binVal) {
          while (!idToUpdate.empty()) {
            nextNull[idToUpdate.back()] = binId;
            idToUpdate.pop_back();
          }
        }
        if (static_cast<size_t>(binId) == getXFromId(candLine[candLineIdx])) {
          idToUpdate.push_back(candLineIdx);
          if (candLineIdx + 1 < candLine.size())
            candLineIdx += 1;
        }
      }
    };
    for (auto yDiff : std::views::iota(1) |
                          std::views::take(gp.yValues.size() - yCand - 1)) {
      if (*std::max_element(begin(slice), end(slice)) == 0)
        break;
      updateLimits();
      for (auto [xCandIdx, xCand] : candLine |
                                        std::views::transform(getXFromId) |
                                        std::views::enumerate) {
        for (auto xOther :
             gp.pointsByY[yCand + yDiff] | std::views::transform(getXFromId)) {
          // std::cout << std::format("Checking ({}, {}) with ({}, {})\n",
          // xCand,
          //  yCand, xOther, yCand + yDiff);
          if (static_cast<int>(xOther) <= prevNull[xCandIdx] ||
              static_cast<int>(xOther) >= nextNull[xCandIdx])
            continue;
          auto candidate =
              gp.getIndexedArea({xCand, yCand}, {xOther, yCand + yDiff});
          if (candidate > globalMax) {
            globalMax = candidate;
          }
        }
      }
      if (yCand + yDiff < gp.subHeights.size()) {
        auto const &curSlice = gp.subHeights[yCand + yDiff];
        for (auto [binIdx, binVal] : curSlice | std::views::enumerate) {
          if (binVal && slice[binIdx]) {
            slice[binIdx] += binVal - 1;
          } else {
            slice[binIdx] = 0;
          }
        }
      }
    }
  }
  return globalMax;
}

constexpr std::array<coord_t, 8> example{
    {{7, 1}, {11, 1}, {11, 7}, {9, 7}, {9, 5}, {2, 5}, {2, 3}, {7, 3}}};

static_assert(getMaxArea(example | std::views::all) == 24);

int main() {
  auto res = getMaxArea(utils::getLines() | std::views::transform(getFromView));
  // auto res = getMaxArea(example | std::views::all);
  std::cout << std::format("Res is: {}\n", res);
  return 0;
}
