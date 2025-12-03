#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <generator>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <numeric>
#include <ranges>
#include <string>
#include <string_view>
#include <sys/types.h>
#include <type_traits>
#include <utility>
#include <vector>

#include "utils/decimal.hpp"
#include "utils/parser.hpp"

using ui = uint_fast32_t;

struct Range {
  ui start;
  ui end;
};

std::generator<Range> getSubRanges(Range range) {
  static constexpr auto powersOfTen = utils::powersOfTen<ui>();
  auto nbDigitsStart = utils::getDecimalDigits(range.start);
  auto nbDigitsEnd = utils::getDecimalDigits(range.end);
  auto start = range.start;
  auto end = range.end;
  while (nbDigitsStart < nbDigitsEnd) {
    co_yield Range{start, powersOfTen[nbDigitsStart] - 1};
    start = powersOfTen[nbDigitsStart];
    nbDigitsStart += 1;
  }
  co_yield Range{start, end};
}

std::generator<Range> getRanges(std::string_view input) {
  utils::Parser parser{input};
  if (input.empty()) {
    co_return;
  }
  std::string_view next{""};
  do {
    ui start{parser.getUnsignedInt()};
    parser.drop(1); // drop '-'
    ui end{parser.getUnsignedInt()};
    for (auto const subRange : getSubRanges(Range{start, end})) {
      co_yield subRange;
    }
    next = parser.drop(1); // drop ','
  } while (!next.empty());
}
namespace detail {
template <ui nbDigits> constexpr auto buildPrimeDivisors() {
  auto constexpr callOnDivisor = [](auto cb) constexpr {
    std::vector<ui> divisors{};
    for (ui i{2}; i <= nbDigits; ++i) {
      if (nbDigits % i == 0) {
        bool isNewDivisor{true};
        for (ui div : divisors) {
          if (i % div == 0) {
            isNewDivisor = false;
            break;
          }
        }
        if (isNewDivisor) {
          cb(i);
          divisors.push_back(i);
        }
      }
    }
  };
  constexpr auto nbDivisors = [callOnDivisor] {
    size_t count{0};
    callOnDivisor([&count](auto) { count += 1; });
    return count;
  }();
  constexpr auto res =
      [callOnDivisor]<size_t resSize>(std::integral_constant<size_t, resSize>) {
        std::array<ui, resSize> result{};
        size_t index{0};
        callOnDivisor(
            [&result, &index](auto divisor) { result[index++] = divisor; });
        return result;
      }(std::integral_constant<size_t, nbDivisors>{});
  return res;
}

template <ui nbDigits, ui... values> constexpr bool primeDivisorsAre() {
  constexpr auto res = buildPrimeDivisors<nbDigits>();
  static_assert(res.size() == sizeof...(values));
  return buildPrimeDivisors<nbDigits>() == std::array{values...};
}

static_assert(primeDivisorsAre<6, 2, 3>());
static_assert(primeDivisorsAre<7, 7>());
static_assert(primeDivisorsAre<8, 2>());
static_assert(primeDivisorsAre<9, 3>());
static_assert(primeDivisorsAre<160, 2, 5>());

template <ui nbDigits>
static constexpr auto primeDivisors = buildPrimeDivisors<nbDigits>();

} // namespace detail

constexpr std::span<const ui> getPrimeDivisors(ui nbDigits) {
  assert(nbDigits <= std::numeric_limits<ui>::digits10);
  auto table = [&]<size_t... Is>(std::index_sequence<Is...>) {
    std::array res{
        std::span<ui const>{detail::primeDivisors<static_cast<ui>(Is)>}...};
    return res;
  }(std::make_index_sequence<std::numeric_limits<ui>::digits10 + 1>());
  if (nbDigits < table.size())
    return table[nbDigits];
  return std::span<const ui>{};
}

constexpr ui duplicate(ui value) {
  constexpr auto powersOfTen = utils::powersOfTen<ui>();
  auto nbDigits = utils::getDecimalDigits(value);
  return value * powersOfTen[nbDigits] + value;
}

static_assert(duplicate(1) == 11);
static_assert(duplicate(12) == 1212);

namespace detail {
constexpr inline ui getSeed(ui totalWidth, ui patternWidth) {
  static constexpr auto powersOfTen = utils::powersOfTen<ui>();
  ui result{0};
  for (ui i{0}; i < totalWidth; i += patternWidth) {
    result += powersOfTen[i];
  }
  return result;
}

static_assert(getSeed(9, 3) == 1001001);
static_assert(getSeed(14, 7) == 10000001);

constexpr auto getHighLow(ui value, ui rangeWidth, ui patternWidth) {
  static constexpr auto powersOfTen = utils::powersOfTen<ui>();
  auto lowGroup = value % powersOfTen[rangeWidth - patternWidth];
  auto highGroup = value / powersOfTen[rangeWidth - patternWidth];
  return std::make_pair(highGroup, lowGroup);
}

static_assert(getHighLow(123456789, 9, 3) == std::make_pair(123, 456789));

constexpr uint_fast64_t sumInvalidInRangeForNumDivisor(Range range, ui divisor,
                                                       bool isFirstDivisor) {
  static constexpr auto powersOfTen = utils::powersOfTen<ui>();
  auto rangeWidth = utils::getDecimalDigits(range.start);
  auto patternWidth = rangeWidth / divisor;
  auto [startHigh, startLow] =
      getHighLow(range.start, rangeWidth, patternWidth);
  auto totalSeed = getSeed(rangeWidth, patternWidth);
  auto lowerSeed = getSeed(rangeWidth - patternWidth, patternWidth);
  ui min = startLow > (startHigh * lowerSeed) ? startHigh + 1 : startHigh;
  auto [endHigh, endLow] = getHighLow(range.end, rangeWidth, patternWidth);
  ui max = endLow < (endHigh * lowerSeed) ? endHigh - 1 : endHigh;
  if (min > max)
    return 0;
  ui sum = (max - min + 1) * totalSeed * (min + max) / 2;
  if (!isFirstDivisor) {
    auto allNines = powersOfTen[patternWidth] - 1;
    auto allOnes = allNines / 9;
    for (ui i = allOnes; i <= allNines; i += allOnes) {
      if (i >= min && i <= max) {
        sum -= i * totalSeed;
      }
    }
  }
  return sum;
}
} // namespace detail

constexpr uint_fast64_t sumInvalidInRange(Range const &range) {
  auto rangeNbDigits = utils::getDecimalDigits(range.start);
  bool isFirstDivisor{true};
  ui totalSum{0};
  for (auto divisor : getPrimeDivisors(rangeNbDigits)) {
    totalSum +=
        detail::sumInvalidInRangeForNumDivisor(range, divisor, isFirstDivisor);
    isFirstDivisor = false;
  }
  return totalSum;
}

int main() {
  std::string input{};
  std::getline(std::cin, input);
  uint_fast64_t totalInvalid{0};
  for (auto const invalidCount :
       getRanges(input) | std::views::transform(sumInvalidInRange)) {
    totalInvalid += invalidCount;
  }
  std::cout << "Total invalid ID: " << totalInvalid << '\n';
}
