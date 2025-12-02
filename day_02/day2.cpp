#include <algorithm>
#include <array>
#include <cstdint>
#include <generator>
#include <iostream>
#include <numeric>
#include <string>
#include <string_view>

#include "utils/decimal.hpp"
#include "utils/parser.hpp"

struct Range {
  uint_fast32_t start;
  uint_fast32_t end;
};

std::generator<Range> getRanges(std::string_view input) {
  utils::Parser parser{input};
  if (input.empty()) {
    co_return;
  }
  std::string_view next{""};
  do {
    uint_fast32_t start{parser.getUnsignedInt()};
    parser.drop(1); // drop '-'
    uint_fast32_t end{parser.getUnsignedInt()};
    co_yield Range{start, end};
    next = parser.drop(1); // drop ','
  } while (!next.empty());
  co_return;
}

constexpr uint_fast32_t findFirstInvalidStemAfter(uint_fast32_t start) {
  constexpr auto powerOfTen = utils::powersOfTen<uint_fast32_t>();
  auto nbDigits = utils::getDecimalDigits(start);
  if (nbDigits % 2 == 1) {
    return powerOfTen[nbDigits / 2];
  }
  auto halfPower = powerOfTen[nbDigits / 2];
  auto highDigits = start / halfPower;
  ;
  auto lowDigits = start % halfPower;
  if (highDigits >= lowDigits)
    return highDigits;
  return highDigits + 1;
}

static_assert(findFirstInvalidStemAfter(1) == 1);
static_assert(findFirstInvalidStemAfter(123) == 10);
static_assert(findFirstInvalidStemAfter(2222) == 22);
static_assert(findFirstInvalidStemAfter(2227) == 23);
static_assert(findFirstInvalidStemAfter(5527) == 55);

constexpr uint_fast32_t findLastValidStemBefore(uint_fast32_t start) {
  constexpr auto powerOfTen = utils::powersOfTen<uint_fast32_t>();
  auto nbDigits = utils::getDecimalDigits(start);
  if (nbDigits % 2 == 1) {
    return powerOfTen[nbDigits / 2] - 1;
  }
  auto halfPower = powerOfTen[nbDigits / 2];
  auto highDigits = start / halfPower;
  auto lowDigits = start % halfPower;

  if (highDigits > lowDigits)
    return highDigits - 1;
  return highDigits;
}

static_assert(findLastValidStemBefore(1) == 0);
static_assert(findLastValidStemBefore(241) == 9);
static_assert(findLastValidStemBefore(83241) == 99);
static_assert(findLastValidStemBefore(8487) == 84);
static_assert(findLastValidStemBefore(8784) == 86);
static_assert(findLastValidStemBefore(5555) == 55);

constexpr uint_fast32_t duplicate(uint_fast32_t value) {
  constexpr auto powersOfTen = utils::powersOfTen<uint_fast32_t>();
  auto nbDigits = utils::getDecimalDigits(value);
  return value * powersOfTen[nbDigits] + value;
}

static_assert(duplicate(1) == 11);
static_assert(duplicate(12) == 1212);

constexpr uint_fast64_t sumInvalidInRange(Range const &range) {
  auto firstInvalid = findFirstInvalidStemAfter(range.start);
  auto lastValid = findLastValidStemBefore(range.end);
  if (firstInvalid > lastValid) {
    return 0;
  }
  return (lastValid - firstInvalid + 1) *
         (duplicate(lastValid) + duplicate(firstInvalid)) / 2;
}

static_assert(sumInvalidInRange({11, 22}) == 33);
static_assert(sumInvalidInRange({95, 115}) == 99);
static_assert(sumInvalidInRange({998, 1012}) == 1010);
static_assert(sumInvalidInRange({1188511880, 1188511890}) == 1188511885);
static_assert(sumInvalidInRange({222220, 222224}) == 222222);
static_assert(sumInvalidInRange({1698522, 1698528}) == 0);
static_assert(sumInvalidInRange({446443, 446449}) == 446446);
static_assert(sumInvalidInRange({38593856, 38593862}) == 38593859);
static_assert(sumInvalidInRange({565653, 565659}) == 0);
static_assert(sumInvalidInRange({824824821, 824824827}) == 0);
static_assert(sumInvalidInRange({2121212118, 2121212124}) == 0);

static_assert(sumInvalidInRange({2222, 2728}) ==
              2222 + 2323 + 2424 + 2525 + 2626 + 2727);

int main() {
  std::string input{};
  std::getline(std::cin, input);
  uint_fast64_t totalInvalid{0};
  for (auto const invalidCount :
       getRanges(input) | std::views::transform(sumInvalidInRange)) {
    totalInvalid += invalidCount;
  }
  std::cout << "Total invalid stems: " << totalInvalid << '\n';
}
