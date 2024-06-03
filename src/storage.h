#ifndef STORAGE_H
#define STORAGE_H

#include <cstddef>
#include <cstdint>
#include <vector>

enum StorageType { MemoryStorage };

class Storage {
public:
  virtual int Save(std::vector<uint8_t>) = 0;
  virtual std::vector<uint8_t> Fetch(int) = 0;
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
  int Save(std::vector<uint8_t>) override;
  std::vector<uint8_t> Fetch(int) override;

private:
  std::vector<std::vector<uint8_t>> bucket; // Memory storage
};

#endif
