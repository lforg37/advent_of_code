#include "utils/io.hpp"
#include "utils/parser.hpp"
#include <algorithm>
#include <array>
#include <iostream>
#include <iterator>
#include <ranges>
#include <vector>

using coord_t = std::array<size_t, 3>;

struct UnionFind {
  using handle_t = size_t;
  constexpr UnionFind(size_t nbElems) {
    elems = std::ranges::to<std::vector>(std::views::iota(size_t{0}) |
                                         std::views::take(nbElems));
    count.resize(nbElems, 1);
  }

  handle_t find(size_t index) {
    std::vector<handle_t> toCompress{};
    while (elems[index] != index) {
      toCompress.push_back(index);
      index = elems[index];
    }
    for (auto &elem : toCompress)
      elem = index;
    return index;
  }

  void update(size_t from, size_t to) {
    count[to] += count[from];
    count[from] = 0;
    elems[from] = to;
  }

  handle_t merge(size_t val0, size_t val1) {
    auto leftRoot = find(val0);
    auto rightRoot = find(val1);
    if (leftRoot == rightRoot)
      return leftRoot;
    if (leftRoot < rightRoot) {
      update(rightRoot, leftRoot);
      return leftRoot;
    }
    update(leftRoot, rightRoot);
    return rightRoot;
  }
  std::vector<handle_t> elems;
  std::vector<size_t> count;
};

constexpr size_t sqDist(coord_t const &c0, coord_t const &c1) {
  return std::ranges::fold_left(
      std::views::zip(c0, c1) | std::views::transform([](auto coord) {
        auto c0 = std::get<0>(coord);
        auto c1 = std::get<1>(coord);
        auto diff = std::max(c0, c1) - std::min(c0, c1);
        return static_cast<size_t>(diff * diff);
      }),
      size_t{0}, std::plus<>{});
}

constexpr coord_t fromLine(std::string_view sv) {
  utils::Parser p(sv);
  coord_t res{};
  res[0] = p.getUnsignedInt();
  p.drop(1);
  res[1] = p.getUnsignedInt();
  p.drop(1);
  res[2] = p.getUnsignedInt();
  return res;
}

size_t getResult() {
  struct DistHolder {
    size_t firstIdx;
    size_t secondIdx;
    size_t squareDist;
    bool operator<(DistHolder const &other) const {
      return (squareDist < other.squareDist) ||
             ((squareDist == other.squareDist) &&
              ((firstIdx < other.firstIdx) || ((firstIdx == other.firstIdx) &&
                                               (secondIdx < other.secondIdx))));
    }
  };
  std::vector<coord_t> boxes;
  std::vector<DistHolder> distHolder;
  for (auto [idx, coord] : utils::getLines() | std::views::transform(fromLine) |
                               std::views::enumerate) {
    for (auto [idx2, otherCoord] : boxes | std::views::enumerate) {
      auto dist = sqDist(coord, otherCoord);
      distHolder.emplace_back(static_cast<size_t>(idx2),
                              static_cast<size_t>(idx), dist);
    }
    boxes.push_back(coord);
  }
  std::sort(begin(distHolder), end(distHolder));
  size_t nbBoxes = boxes.size();
  UnionFind uf(boxes.size());
  for (auto d : distHolder) {
    auto res = uf.merge(d.firstIdx, d.secondIdx);
    if (res == 0 && uf.count[0] == nbBoxes) {
      return boxes[d.firstIdx][0] * boxes[d.secondIdx][0];
    }
  }

  return 17;
}

int main() {
  auto res = getResult();
  std::cout << "Res is: " << res << '\n';
  return 0;
}
