#include <cstddef>
#include <cstdint>
#include <fcntl.h>
#include <iostream>
#include <span>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utility>

namespace {
struct Parser {
  // Supposing well-formed input
  constexpr Parser(std::span<char> data) {
    char *readingHead = data.data();
    char *end = data.data() + data.size();
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
      nbSteps = nbSteps % 100;
      if (dir == Direction::LEFT) {
        nbSteps = 100 - nbSteps;
      } else {
        nbSteps = nbSteps;
      }
      update(nbSteps);
      // Skip the '\n'
      readingHead++;
    }
  }

  constexpr auto getCount() const { return count; }
  constexpr auto getPosition() const { return position; }

private:
  enum struct Direction { LEFT, RIGHT };
  void update(uint_fast32_t nbSteps) {
    position += nbSteps;
    position = position % 100;
    if (position == 0) {
      count += 1;
    }
  }
  uint_fast8_t position{50};
  uint_fast16_t count{0};
};

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
