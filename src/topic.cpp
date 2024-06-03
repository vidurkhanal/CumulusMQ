#include "topic.h"
#include <cstdint>

Topic::Topic(const std::string &name, Storage *storage)
    : name(name), storage(std::move(storage)) {}

/*
 * NOTE: The publish and consume methods are just placeholders.
 * */
void Topic::publish(std::vector<uint8_t> message) { storage->Save(message); }

void Topic::consume() { storage->Fetch(0); }
