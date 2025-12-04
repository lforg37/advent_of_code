#include <algorithm>
#include <array>
#include <deque>
#include <generator>
#include <iostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

class Graph {
public:
  using handle_t = size_t;
  struct Cell {
    bool active{true};
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
             return cells[value].active && filter(value);
           });
  }

  auto constexpr getNeighbors(handle_t handle) {
    return cells[handle].neighbors |
           std::views::filter(
               [this](handle_t handle) { return cells[handle].active; });
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

  constexpr void erase(handle_t node) {
    cells[node].active = false;
    nbDeleted += 1;
  }

  constexpr size_t size() const { return cells.size() - nbDeleted - 1; }

private:
  std::vector<Cell> cells{};
  size_t nbDeleted{0};
};

// Add one extraLine
std::generator<std::string_view> getLines() {
  std::string line{};
  while (std::getline(std::cin, line)) {
    co_yield line;
  }
}

constexpr size_t getAvailables(std::ranges::view auto &&lines) {
  Graph myGraph{};
  std::vector<Graph::handle_t> oldCells{}, cells{};
  size_t size{0};
  for (auto line : lines) {
    if (oldCells.size() == 0) {
      oldCells.resize(line.size());
      cells.resize(line.size());
      size = line.size();
    }
    std::fill_n(begin(cells), oldCells.size(), Graph::handle_t{});
    for (auto [idx, node] : line | std::views::transform([&myGraph](char c) {
                              return c == '@' ? myGraph.addNode()
                                              : Graph::handle_t{};
                            }) | std::views::enumerate) {
      cells[idx] = node;
      if (idx) {
        myGraph.connect(node, cells[idx - 1]);
        myGraph.connect(node, oldCells[idx - 1]);
      }
      myGraph.connect(node, oldCells[idx]);
      if (static_cast<size_t>(idx) + 1 < size) {
        myGraph.connect(node, oldCells[idx + 1]);
      }
    }
    std::swap(oldCells, cells);
  }
  auto removeFilter = [&myGraph](auto handle) {
    return myGraph.getNumNeighbors(handle) < 4;
  };
  auto toRemove = std::ranges::to<std::vector>(myGraph.getNodes(removeFilter));
  size_t sum{0};
  while (!toRemove.empty()) {
    auto handle = toRemove.back();
    auto neighborList = myGraph.getNeighbors(handle);
    myGraph.erase(handle);
    sum += 1;
    toRemove.pop_back();
    for (auto n : neighborList | std::views::filter([&myGraph](auto handle) {
                    return myGraph.getNumNeighbors(handle) == 3;
                  })) {
      toRemove.push_back(n);
    }
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
                                   "@.@.@@@.@."};
static_assert(getAvailables(example | std::views::chunk(10)) == 43);

int main() {
  auto sum = getAvailables(getLines());
  std::cout << "Nb movable rolls: " << sum << '\n';
}
