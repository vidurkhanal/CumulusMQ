#include "topic.h"

Topic::Topic(const std::string &name, std::unique_ptr<Storage> storage)
    : name(name), storage(std::move(storage)) {}

void Topic::publish(const std::byte &message) {
  std::vector<std::byte> data;
  data.push_back(message);
  storage->Save(data);
}

void Topic::consume(const std::byte &message) { storage->Fetch(0); }
