#include <cumulusmq/storage.h>

Storage::~Storage() = default;

Storage *StorageFactory::Build(StorageType type) {
  if (type == StorageType::MemoryStorage) {
    return new Memory();
  }
  return nullptr;
}

StorageType parseStorageType(const char *type) {
  if (strcmp(type, "MemoryStorage") == 0) {
    return MemoryStorage;
  }
  return MemoryStorage;
}
