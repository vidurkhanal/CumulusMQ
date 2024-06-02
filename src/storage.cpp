#include "storage.h"

Storage *StorageFactory::Build(StorageType type) {
  if (type == StorageType::MemoryStorage) {
    return new Memory();
  }
  return nullptr;
}
