#include "storage.h"
#include <vector>

Memory::Memory() { this->bucket = std::vector<const char *>(); }

Memory::~Memory() {}

int Memory::Save(const char *data) {
  bucket.push_back(data);
  return bucket.size();
};

const char *Memory::Fetch(int offset) {
  if (offset < 0 || offset >= bucket.size()) {
    throw std::out_of_range("Index out of range");
  }
  return bucket[offset];
}
