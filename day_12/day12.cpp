#include "utils/io.hpp"
#include "utils/parser.hpp"
#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <string>
#include <tuple>
#include <vector>

class PresentShape {
  // All shapes are 3x3 in input and example: store as bitmask
  std::array<uint8_t, 3> shapeRows;

  size_t nbFull;

public:
  PresentShape(std::array<std::string, 3> const &shapeLines) {
    nbFull = 0;
    for (auto const &[inRow, locRow] : std::views::zip(shapeLines, shapeRows)) {
      for (auto c : inRow) {
        locRow <<= 1;
        if (c == '#') {
          locRow |= 1;
          nbFull += 1;
        }
      }
    }
  }

  size_t countFull() const { return nbFull; }
};

class Problem {
public:
  struct TreeArea {
    size_t width;
    size_t height;
    std::vector<size_t> countOfShapes;
  };

private:
  std::vector<PresentShape> shapes;
  std::vector<TreeArea> treeAreas;

public:
  enum struct QuickCheckResultFits { Yes = 0, No = 1, Maybe = 2 };

  void addShape(std::array<std::string, 3> const &shapeLines) {
    shapes.emplace_back(shapeLines);
  }

  auto const &getShapes() const { return shapes; }
  auto const &getTreeAreas() const { return treeAreas; }

  void addTreeArea(size_t width, size_t height,
                   std::vector<size_t> &&countOfShapes) {
    treeAreas.emplace_back(width, height, std::move(countOfShapes));
  }

  QuickCheckResultFits quickCheckFits(TreeArea const &treeArea) const {
    size_t totalShapes{0}, totalFilledCells{0};
    for (auto [shapeId, count] :
         treeArea.countOfShapes | std::views::enumerate) {
      totalShapes += count;
      totalFilledCells += count * shapes[shapeId].countFull();
    }
    if (totalFilledCells > treeArea.width * treeArea.height) {
      return QuickCheckResultFits::No;
    }
    if (totalShapes <= treeArea.width / 3 * treeArea.height / 3) {
      return QuickCheckResultFits::Yes;
    }
    return QuickCheckResultFits::Maybe;
  }
};

class ProblemParser {
  Problem problem;
  void ParseShapes() {
    std::array<std::string, 3> shapeLines;
    for (auto [id, sv] :
         utils::getLines() | std::views::take(3) | std::views::enumerate) {
      shapeLines[id] = sv;
    }
    problem.addShape(shapeLines);
  }

  void ParseTreeArea(std::string_view view) {
    utils::Parser parser{view};
    auto width = parser.getUnsignedInt();
    parser.eat('x');
    auto height = parser.getUnsignedInt();
    parser.eat(':');
    auto counts = std::ranges::to<std::vector>(
        parser.remain() | std::views::split(' ') |
        std::views::filter([](auto const &val) { return !val.empty(); }) |
        std::views::transform([&](auto const &range) {
          size_t count{0};
          std::from_chars(range.begin(), range.end(), count);
          return count;
        }));
    problem.addTreeArea(width, height, std::move(counts));
  }

public:
  void parse() {
    for (auto sv : utils::getLines()) {
      if (sv.empty())
        continue;
      utils::Parser parser{sv};
      std::ignore = parser.getUnsignedInt();
      if (*parser.remain().begin() == ':') {
        ParseShapes();
      } else {
        ParseTreeArea(sv);
      }
    }
  }

  Problem const &getProblem() const { return problem; }
};

void solveInstance() {
  ProblemParser parser;
  parser.parse();
  auto const &problem = parser.getProblem();
  std::array<size_t, 3> quickCheckResults{0};
  for (auto const &[id, treeArea] :
       problem.getTreeAreas() | std::views::enumerate) {
    auto result = problem.quickCheckFits(treeArea);
    quickCheckResults[static_cast<size_t>(result)] += 1;
    if (result == Problem::QuickCheckResultFits::Maybe) {
      std::cout << std::format("Maybe case for tree area {}\n", id);
      // For now, we do not handle the "maybe" cases
      continue;
    }
  }
  if (quickCheckResults[static_cast<size_t>(
          Problem::QuickCheckResultFits::Maybe)] != 0) {
    throw std::runtime_error("There are some \"maybe\" "
                             "cases, which is not handled yet");
  }
  std::cout << std::format("Fits: {} , Does not fit: {}\n",
                           quickCheckResults[static_cast<size_t>(
                               Problem::QuickCheckResultFits::Yes)],
                           quickCheckResults[static_cast<size_t>(
                               Problem::QuickCheckResultFits::No)]);
}

int main() {
  solveInstance();
  return 0;
}
