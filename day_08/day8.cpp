#include "utils/io.hpp"
#include "utils/parser.hpp"
#include <algorithm>
#include <array>
#include <cstddef>
#include <deque>
#include <functional>
#include <iostream>
#include <numeric>
#include <optional>
#include <ranges>
#include <string_view>
#include <unistd.h>
#include <vector>

class Graph {
public:
  using handle_t = size_t;
  enum struct State : uint8_t { DELETED = 0, ACTIVE = 1, VISITING = 2 };
  struct Cell {
    State state{State::ACTIVE};
    std::vector<handle_t> neighbors;
    void constexpr connect(handle_t node) { neighbors.push_back(node); }
  };
  constexpr Graph() {
    // Add sentinel
    cells.push_back({});
  }

  constexpr handle_t addNode() {
    auto res = cells.size();
    cells.push_back({});
    return res;
  }

  auto constexpr getNodes(auto &filter) const {
    return std::views::iota(size_t{1}, cells.size()) |
           std::views::filter([&filter, this](handle_t value) {
             return (cells[value].state != State::DELETED) && filter(value);
           });
  }

  auto constexpr getNodes() const {
    return std::views::iota(size_t{1}, cells.size()) |
           std::views::filter([this](handle_t value) {
             return (cells[value].state != State::DELETED);
           });
  }

  auto constexpr getNeighbors(handle_t handle) {
    return cells[handle].neighbors |
           std::views::filter([this](handle_t handle) {
             return (cells[handle].state != State::DELETED);
           });
  }

  auto constexpr getCleanNeighbors(handle_t handle) {
    return cells[handle].neighbors |
           std::views::filter([this](handle_t handle) {
             return (cells[handle].state == State::ACTIVE);
           });
  }

  constexpr size_t getNumNeighbors(handle_t node) {
    return std::ranges::fold_left(
        getNeighbors(node) | std::views::transform([](auto) { return 1; }), 0,
        std::plus<>{});
  }

  constexpr void connect(handle_t node0, handle_t node1) {
    if (0 == node0 || 0 == node1)
      return;
    cells[node0].connect(node1);
    cells[node1].connect(node0);
  }

  constexpr void visit(handle_t node) { cells[node].state = State::VISITING; }

  constexpr void erase(handle_t node) {
    cells[node].state = State::DELETED;
    nbDeleted += 1;
  }

  constexpr size_t size() const { return cells.size() - nbDeleted - 1; }

private:
  std::vector<Cell> cells{};
  size_t nbDeleted{0};
};

using coord_t = std::array<size_t, 3>;

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

constexpr size_t getGroups(std::ranges::view auto &&views) {
  constexpr size_t maxSize{1000};
  struct DistHolder {
    size_t firstIdx;
    size_t secondIdx;
    size_t squareDist;
    bool operator<(DistHolder const &other) const {
      return squareDist < other.squareDist;
    }
  };
  std::vector<DistHolder> distHolder;
  distHolder.reserve(maxSize);
  size_t nBoxes;
  {
    std::vector<coord_t> boxes;
    for (auto in : views) {
      auto coord = fromLine(std::string_view{in});
      auto idx = boxes.size();
      for (auto [idx2, otherCoord] : boxes | std::views::enumerate) {
        auto dist = sqDist(coord, otherCoord);
        auto insert =
            std::lower_bound(std::begin(distHolder), std::end(distHolder),
                             DistHolder{0, 0, dist});
        if (insert != end(distHolder) || (distHolder.size() < maxSize)) {
          if (distHolder.size() >= maxSize)
            distHolder.pop_back();
          distHolder.insert(insert, DistHolder{static_cast<size_t>(idx2),
                                               static_cast<size_t>(idx), dist});
        }
      }
      boxes.push_back(coord);
    }
    nBoxes = boxes.size();
  }

  std::vector<std::optional<Graph::handle_t>> newIds;
  newIds.resize(nBoxes, std::nullopt);

  Graph g{};
  auto getHandle = [&g, &newIds](size_t idx) {
    return newIds[idx]
        .or_else([&g, &newIds, idx] {
          auto res = g.addNode();
          newIds[idx] = res;
          return newIds[idx];
        })
        .value();
  };
  for (auto dist : distHolder) {
    auto idx0 = getHandle(dist.firstIdx);
    auto idx1 = getHandle(dist.secondIdx);
    g.connect(idx0, idx1);
  }
  std::vector<size_t> biggerGroupSize{{1, 1, 1}};
  for (auto n : g.getNodes()) {
    std::deque<Graph::handle_t> nodeList;
    size_t groupSize{0};
    nodeList.push_back(n);
    while (!nodeList.empty()) {
      auto curNode = nodeList.front();
      for (auto neighbor : g.getCleanNeighbors(curNode)) {
        g.visit(neighbor);
        nodeList.push_back(neighbor);
      }
      nodeList.pop_front();
      g.erase(curNode);
      groupSize += 1;
    }
    auto ip = std::lower_bound(begin(biggerGroupSize), end(biggerGroupSize),
                               groupSize, std::greater_equal<>{});
    if (ip != biggerGroupSize.end()) {
      biggerGroupSize.pop_back();
      biggerGroupSize.insert(ip, groupSize);
    }
  }
  return std::reduce(begin(biggerGroupSize), end(biggerGroupSize), size_t{1},
                     std::multiplies<>{});
}

int main() {
  auto res = getGroups(utils::getLines());
  std::cout << "Result is: " << res << '\n';
  return 0;
}
