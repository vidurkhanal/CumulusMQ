#include "storage.h"
#include <vector>

Memory::Memory() { this->bucket = std::vector<std::vector<std::byte>>(); }

Memory::~Memory() {}

int Memory::Save(std::vector<std::byte> data) {
  bucket.push_back(data);
  return bucket.size();
};

std::vector<std::byte> Memory::Fetch(int offset) {
  if (offset < 0) {
    return std::vector<std::byte>();
  }

  if (offset >= bucket.size()) {
    return std::vector<std::byte>();
  }

  return bucket[offset];
}
