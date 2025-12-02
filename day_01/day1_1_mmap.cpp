#include <cstddef>
#include <cstdint>
#include <fcntl.h>
#include <iostream>
#include <span>
#include <string_view>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utility>

namespace {

constexpr uint_fast32_t dialSize{100};
struct Parser {
  // Supposing well-formed input
  constexpr Parser(std::span<const char> data) {
    char const *readingHead = data.data();
    char const *end = data.data() + data.size();
    while (readingHead < end) {
      Direction dir;
      if (*readingHead == 'L') {
        dir = Direction::LEFT;
      } else if (*readingHead == 'R') {
        dir = Direction::RIGHT;
      } else {
        std::unreachable();
      }
      readingHead++;
      uint_fast32_t nbSteps{0};
      while (*readingHead != '\n') {
        nbSteps = nbSteps * 10 + static_cast<uint_fast32_t>(*readingHead - '0');
        readingHead++;
      }
      update(dir, nbSteps);
      // Skip the '\n'
      readingHead++;
    }
  }

  constexpr auto getCount() const { return count; }
  constexpr auto getPosition() const { return position; }

private:
  enum struct Direction { LEFT, RIGHT };
  constexpr void update(Direction dir, uint_fast32_t nbSteps) {
    count += nbSteps / dialSize;
    nbSteps = nbSteps % dialSize;
    if (dir == Direction::LEFT) {
      auto newPos = (position + (dialSize - nbSteps)) % dialSize;
      if (nbSteps >= position && position != 0) {
        count += 1;
      }
      position = newPos;
    } else {
      auto newPos = (position + nbSteps) % dialSize;
      if (newPos < position) {
        count += 1;
      }
      position = newPos;
    }
  }
  uint_fast8_t position{50};
  uint_fast16_t count{0};
};

namespace test {
consteval auto getExpected(std::string_view input) {
  Parser parser{std::span<const char>{input.data(), input.size()}};
  return parser.getCount();
}

static_assert(getExpected("R15\n") == 0);
static_assert(getExpected("L15\n") == 0);
static_assert(getExpected("L50\n") == 1);
static_assert(getExpected("R50\n") == 1);
static_assert(getExpected("L99\n") == 1);
static_assert(getExpected("R99\n") == 1);
static_assert(getExpected("L101\n") == 1);
static_assert(getExpected("R101\n") == 1);
static_assert(getExpected("L149\n") == 1);
static_assert(getExpected("R149\n") == 1);
static_assert(getExpected("L150\n") == 2);
static_assert(getExpected("R150\n") == 2);

static_assert(getExpected("R50\nR100\n") == 2);
static_assert(getExpected("R50\nR101\n") == 2);
static_assert(getExpected("R50\nR200\n") == 3);

static_assert(getExpected("L50\nL100\n") == 2);
static_assert(getExpected("L50\nL101\n") == 2);
static_assert(getExpected("L50\nL200\n") == 3);

}; // namespace test

} // namespace

int main() {

  int fd = open("input.txt", O_RDONLY, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    return -1;
  }
  struct stat sb;
  if (fstat(fd, &sb) == -1) {
    return -1;
  }
  size_t file_size = sb.st_size;

  void *addr = mmap(NULL, file_size, PROT_READ, MAP_SHARED, fd, 0);
  if (addr == MAP_FAILED) {
    return -1;
  }

  char *file_content = static_cast<char *>(addr);
  std::span char_view{file_content, file_size};

  Parser parser{char_view};
  auto counter = parser.getCount();

  std::cout << "Number of times we crossed the position 0:" << counter << '\n';

  if (munmap(addr, file_size) == -1) {
    return -1;
  }
  close(fd);
}
