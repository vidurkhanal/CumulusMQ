#ifndef STORAGE_H
#define STORAGE_H

#include <cstddef>
#include <vector>

class Storage {
public:
  Storage() {}          // Default constructor
  virtual ~Storage() {} // Virtual destructor
  virtual int Save(std::vector<std::byte>) = 0;
  virtual std::vector<std::byte> Fetch(int) = 0;
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
