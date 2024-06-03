#ifndef INCLUDE_SRC_TOPIC_H_
#define INCLUDE_SRC_TOPIC_H_
#include "storage.h"
#include <cstddef>
#include <string>
#include <vector>

class Topic {
public:
  Topic(const std::string &name, Storage *storage);

  void publish(std::vector<uint8_t> message);
  void consume();

private:
  std::string name;
  Storage *storage;
};

#endif // INCLUDE_SRC_TOPIC_H_
