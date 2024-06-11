#ifndef STORAGE_H
#define STORAGE_H

#include <cstddef>
#include <vector>

enum StorageType { MemoryStorage };

class Storage {
public:
  virtual ~Storage() = 0;
  virtual int Save(const char *) = 0;
  virtual const char *Fetch(int) = 0;
};

class StorageFactory {
public:
  StorageFactory() {}
  ~StorageFactory() {}
  Storage *Build(StorageType type);
};

StorageType parseStorageType(const char *type);

class Memory : public Storage {
public:
  Memory();
  ~Memory() override;
  int Save(const char *) override;
  const char *Fetch(int) override;

private:
  std::vector<const char *> bucket;
};

#endif
