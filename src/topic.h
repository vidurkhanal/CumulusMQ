#ifndef INCLUDE_SRC_TOPIC_H_
#define INCLUDE_SRC_TOPIC_H_
#include "storage.h"
#include <cstddef>
#include <string>

class Topic {
public:
  Topic(const std::string &name, Storage *storage);

  void publish(const std::byte &message);
  void consume(const std::byte &message);

private:
  std::string name;
  Storage *storage;
};

#endif // INCLUDE_SRC_TOPIC_H_
