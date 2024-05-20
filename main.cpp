#include "include/storage/storage.h"
#include <cstddef>
#include <iostream>

int main() {
  Memory memory;

  for (int i = 0; i < 10; i++) {
    auto data = std::vector<std::byte>();

    for (int j = 0; j < 10; j++) {
      data.push_back(static_cast<std::byte>(3));
    }

    memory.Save(data);
  }

  for (int i = 0; i < 10; i++) {
    auto data = memory.Fetch(i);
    std::cout << static_cast<int>(data.front()) << std::endl;
  }
}
