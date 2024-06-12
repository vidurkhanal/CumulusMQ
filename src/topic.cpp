#include <cumulusmq/topic.h>

Topic::Topic(const std::string &name, Storage *storage)
    : name(name), storage(std::move(storage)) {}

Topic::~Topic() { delete storage; }

void Topic::publish(const char *message) { storage->Save(message); }

const char *Topic::consume(int offset) { return storage->Fetch(offset); }
