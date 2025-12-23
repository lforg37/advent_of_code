#include "utils/io.hpp"
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

class Graph {
  struct Node {
    std::vector<size_t> forward;
    std::vector<size_t> backward;
  };
  std::vector<Node> nodes;
  std::unordered_map<std::string, size_t> nodeIndex;

  size_t getOrCreateNode(std::string const &nodeName) {
    auto it = nodeIndex.find(nodeName);
    if (it != nodeIndex.end())
      return it->second;
    auto id = nodes.size();
    nodes.emplace_back();
    nodeIndex.emplace(nodeName, id);
    return id;
  }

public:
  void add_line(std::string const &nodeName,
                std::span<std::string> outputNames) {
    auto newNodeIdx = getOrCreateNode(nodeName);
    for (auto outputName : outputNames) {
      auto destId = getOrCreateNode(outputName);
      nodes[newNodeIdx].forward.push_back(destId);
    }
  }

  // Should be called on registered nodes
  size_t countPath(std::string const &from, std::string const &to,
                   std::string const &excluding) {
    enum struct State { CLEAN, VISITING, VISITED };
    std::vector<State> states;
    states.resize(nodes.size(), State::CLEAN);
    std::vector<size_t> paths{};
    paths.resize(nodes.size(), 0);
    auto node = getOrCreateNode(from);
    auto toIdx = getOrCreateNode(to);
    auto excluded = getOrCreateNode(excluding);
    states[excluded] = State::VISITED;

    std::vector<size_t> toExplore{node};
    while (!toExplore.empty()) {
      auto curNode = toExplore.back();
      if (states[curNode] == State::VISITED) {
        toExplore.pop_back();
        continue;
      }
      auto firstVisit = states[curNode] == State::CLEAN;
      if (firstVisit) {
        states[curNode] = State::VISITING;
      }
      auto allExplored{true};
      size_t sum{0};
      for (auto successor : nodes[curNode].forward) {
        if (successor == toIdx) {
          if (firstVisit) {
            paths[curNode] += 1;
          }
        } else {
          if (states[successor] == State::VISITED) {
            sum += paths[successor];
          } else {
            allExplored = false;
            toExplore.push_back(successor);
          }
        }
      }
      if (allExplored) {
        paths[curNode] += sum;
        states[curNode] = State::VISITED;
        toExplore.pop_back();
      }
    }
    return paths[node];
  }

  // Should be called on registered nodes
  size_t countPath(std::string const &from, std::string const &to) {
    enum struct State { CLEAN, VISITING, VISITED };
    std::vector<State> states;
    states.resize(nodes.size(), State::CLEAN);
    std::vector<size_t> paths{};
    paths.resize(nodes.size(), 0);
    auto node = nodeIndex.find(from)->second;
    auto toIdx = nodeIndex.find(to)->second;
    std::vector<size_t> toExplore{node};
    while (!toExplore.empty()) {
      auto curNode = toExplore.back();
      auto firstVisit = states[curNode] == State::CLEAN;
      if (firstVisit) {
        states[curNode] = State::VISITING;
      }
      auto allExplored{true};
      size_t sum{0};
      for (auto successor : nodes[curNode].forward) {
        if (successor == toIdx) {
          if (firstVisit) {
            paths[curNode] += 1;
          }
        } else {
          if (states[successor] == State::VISITED) {
            sum += paths[successor];
          } else {
            allExplored = false;
            toExplore.push_back(successor);
          }
        }
      }
      if (allExplored) {
        paths[curNode] += sum;
        states[curNode] = State::VISITED;
        toExplore.pop_back();
      }
    }
    return paths[node];
  }
};

size_t getValues() {
  Graph g;
  for (auto sv : utils::getLines()) {
    auto splitPos = sv.find(':');
    auto name = std::string(sv.begin(), sv.begin() + splitPos);
    auto nodes = std::ranges::to<std::vector>(
        sv | std::views::split(' ') | std::views::drop(1) |
        std::views::filter([](auto const &val) { return !val.empty(); }) |
        std::views::transform([](auto const &range) {
          return std::string{range.begin(), range.end()};
        }));
    g.add_line(name, nodes);
  }
  g.add_line("sentinel", {});
  auto svrToDac = g.countPath("svr", "dac", "fft");
  auto svrToFFT = g.countPath("svr", "fft", "dac");
  auto dacToFFT = g.countPath("dac", "fft");
  auto FFTToDac = g.countPath("fft", "dac");
  auto FFTToOut = g.countPath("fft", "out", "dac");
  auto DacToOut = g.countPath("dac", "out");
  return svrToDac * dacToFFT * FFTToOut + svrToFFT * FFTToDac * DacToOut;
}

int main() {
  auto res = getValues();
  std::cout << std::format("Res is {}\n", res);
  return 0;
}
