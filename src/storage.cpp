#include "storage.h"

Storage::~Storage() = default;

Storage *StorageFactory::Build(StorageType type) {
  if (type == StorageType::MemoryStorage) {
    return new Memory();
  }
  return nullptr;
}
