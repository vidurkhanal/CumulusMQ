#include <cumulusmq/storage.h>
#include <vector>

Memory::Memory() = default; // Default constructor is sufficient

Memory::~Memory() {
  for (const char *data : bucket) {
    delete[] data;
  }
}

int Memory::Save(const char *data) {
  size_t len = strlen(data) + 1;
  char *newData = new char[len];
  strcpy(newData, data);

  bucket.push_back(newData);
  return bucket.size();
}

const char *Memory::Fetch(int offset) {
  if (offset < 0 || offset >= static_cast<int>(bucket.size())) {
    throw std::out_of_range("Index out of range");
  }
  return bucket[offset];
}
