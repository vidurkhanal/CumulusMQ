#ifndef STORAGE_H
#define STORAGE_H

#include <cstddef>
#include <vector>

enum StorageType { MemoryStorage };

class Storage {
public:
  virtual int Save(const char *) = 0;
  virtual const char *Fetch(int) = 0;
  virtual ~Storage() = 0;
};

class StorageFactory {
public:
  StorageFactory() {}               // Default constructor
  ~StorageFactory() {}              // Destructor
  Storage *Build(StorageType type); // Create storage
};

class Memory : public Storage {
public:
  Memory();           // Constructor declaration
  ~Memory() override; // Destructor declaration
  int Save(const char *) override;
  const char *Fetch(int) override;

private:
  std::vector<const char *> bucket; // Memory storage
};

#endif
