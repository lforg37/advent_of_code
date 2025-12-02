#include <charconv>
#include <fstream>
#include <iostream>
#include <optional>
#include <ranges>
#include <string_view>

namespace {

constexpr size_t quadrantSize{100};

// Parses a movement line like "L10" or "R25" and returns the corresponding
// movement in terms of positive position change (modulo quadrantSize).
// Returns std::nullopt if the line is malformed.
constexpr std::optional<uint8_t> parseSignedMovement(std::string_view line) {
  char direction = line[0];
  uint32_t nbSteps{0};
  auto sizePart = line.substr(1);
  auto res = std::from_chars(sizePart.data(), sizePart.data() + sizePart.size(),
                             nbSteps);
  if (res.ec != std::errc{}) {
    return std::nullopt;
  }
  nbSteps = nbSteps % quadrantSize;

  switch (direction) {
  case 'L':
    return 100 - nbSteps;
  case 'R':
    return nbSteps;
  default:
    return std::nullopt;
  }
}
} // namespace

int main() {
  // Create a view of each line from standard input (the lines are supposed to
  // be correctly formed so no space in a line)
  std::ifstream input{"input.txt"};
  auto lines_view = std::views::istream<std::string>(input);
  uint8_t position{50};
  uint64_t counter{0};
  // Process each movement line by parsing it, filtering ut the invalid
  // movements and updating the cursor position and counting the number of times
  // we cross position 0.
  for (const auto movement :
       lines_view | std::views::transform(parseSignedMovement) |
           std::views::filter([](const auto &opt) { return opt.has_value(); }) |
           std::views::transform([](const auto &opt) { return *opt; })) {
    position += movement;
    position = position % quadrantSize;
    if (position == 0) {
      counter += 1;
    }
  }
  std::cout << "Number of times we crossed the position 0: " << counter << '\n';
  return 0;
}
