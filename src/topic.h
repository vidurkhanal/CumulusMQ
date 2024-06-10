#ifndef INCLUDE_SRC_TOPIC_H_
#define INCLUDE_SRC_TOPIC_H_
#include "storage.h"
#include <cstddef>
#include <string>

class Topic {
public:
  Topic(const std::string &name, Storage *storage);
  ~Topic();

  void publish(const char *message);
  const char *consume(int);

private:
  std::string name;
  Storage *storage;
};

#endif // INCLUDE_SRC_TOPIC_H_
