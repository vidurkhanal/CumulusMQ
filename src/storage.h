#ifndef STORAGE_H
#define STORAGE_H

#include <cstddef>
#include <vector>

enum StorageType { MemoryStorage };

class Storage {
public:
  virtual int Save(std::vector<std::byte>) = 0;
  virtual std::vector<std::byte> Fetch(int) = 0;
};

class StorageFactory {
public:
  StorageFactory() {}               // Default constructor
  ~StorageFactory() {}              // Destructor
  Storage *Build(StorageType type); // Create storage
};

class Memory : public Storage {
public:
  Memory();  // Constructor declaration
  ~Memory(); // Destructor declaration
  int Save(std::vector<std::byte>) override;
  std::vector<std::byte> Fetch(int) override;

private:
  std::vector<std::vector<std::byte>> bucket; // Memory storage
};

#endif
