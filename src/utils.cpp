#include "utils.h"

#include <cerrno>
#include <cstdlib>
#include <limits>
#include <stdexcept>

int stringToInt(const char *str) {
  char *endptr;
  errno = 0;
  long value = std::strtol(str, &endptr, 10);

  if (endptr == str) {
    throw std::invalid_argument("Invalid or empty string");
  }

  if (errno == ERANGE || value < std::numeric_limits<int>::min() ||
      value > std::numeric_limits<int>::max()) {
    throw std::out_of_range("Value out of range for int");
  }

  return static_cast<int>(value);
}
