#include "topic.h"

Topic::Topic(const std::string &name, Storage *storage)
    : name(name), storage(std::move(storage)) {}

/*
 * NOTE: The publish and consume methods are just placeholders.
 * */
void Topic::publish(const std::byte &message) {
  std::vector<std::byte> data;
  data.push_back(message);
  storage->Save(data);
}

void Topic::consume(const std::byte &message) { storage->Fetch(0); }
