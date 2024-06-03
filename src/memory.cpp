#include "storage.h"
#include <cstdint>
#include <vector>

Memory::Memory() { this->bucket = std::vector<std::vector<uint8_t>>(); }

Memory::~Memory() {}

int Memory::Save(std::vector<uint8_t> data) {
  bucket.push_back(data);
  return bucket.size();
};

std::vector<uint8_t> Memory::Fetch(int offset) {
  if (offset < 0) {
    return std::vector<uint8_t>();
  }

  if (offset >= bucket.size()) {
    return std::vector<uint8_t>();
  }

  return bucket[offset];
}
