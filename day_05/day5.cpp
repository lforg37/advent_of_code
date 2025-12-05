#include "utils/parser.hpp"
#include <algorithm>
#include <cstddef>
#include <functional>
#include <generator>
#include <iostream>
#include <iterator>
#include <map>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>

using Range = std::pair<size_t, size_t>;

std::generator<std::string_view> getLines() {
  std::string res{};
  while (std::getline(std::cin, res)) {
    if (res.empty())
      co_return;
    co_yield res;
  }
}

Range parseRange(std::string_view view) {
  Range res{};
  utils::Parser p{view};
  res.first = p.getUnsignedInt();
  p.drop(1);
  res.second = p.getUnsignedInt();
  return res;
}

class RangeSet {
private:
  // All ranges there will be disjoint
  std::map<size_t, size_t> ranges;

public:
  void addRange(Range range) {
    auto lBound = std::lower_bound(ranges.begin(), ranges.end(), range);
    if (lBound != begin(ranges)) {
      lBound--;
      auto &prev = *lBound;
      // Case where we can merge with previous element
      if (prev.second >= range.first) {
        if (prev.second >= range.second)
          return;
        prev.second = range.second;
        lBound++;
        // Check to see if we also overlap with next
        if (lBound != ranges.end() && lBound->first <= prev.second) {
          prev.second = lBound->second;
          ranges.erase(lBound);
        }
        return;
      }
      lBound++;
    }
    if (lBound == ranges.end() || lBound->first > range.second) {
      ranges.insert(range);
      return;
    }
    // There is overlap with next one
    if (lBound->first == range.first) // Range is already contained
      return;

    // Now there is only the case where we have new range
    // having lower bound than lBound but start at least
    // contained in lBound

    if (range.second <= lBound->second) {
      range.second = lBound->second;
      ranges.erase(lBound);
      ranges.insert(range);
      return;
    }

    if (range.second > lBound->second) {
      Range toDelete{*lBound};
      lBound++;
      if (range.second >= lBound->first) {
        range.second = lBound->second;
        ranges.erase(lBound);
      }
      ranges.erase(toDelete.first);
      ranges.insert(range);
    }
  }

  size_t getDisjointIntervals() const { return ranges.size(); }
  bool isContained(size_t value) const {
    auto lb = std::lower_bound(begin(ranges), end(ranges),
                               std::make_pair(value, value));
    if (lb == begin(ranges))
      return false;
    lb--;
    return lb->second >= value;
  }
};

size_t nbIngredients() {
  RangeSet rs{};
  for (auto range : getLines() | std::views::transform(parseRange))
    rs.addRange(range);
  return std::ranges::fold_left(
      getLines() | std::views::transform([&rs](std::string_view v) -> size_t {
        utils::Parser p{v};
        auto val = p.getUnsignedInt();
        return rs.isContained(val) ? 1 : 0;
      }),
      size_t{0}, std::plus<>{});
}

int main() {
  auto res = nbIngredients();
  std::cout << "N ingredients: " << res << '\n';
  return 0;
}
