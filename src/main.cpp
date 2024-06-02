#include "server.h"
#include "storage.h"
#include <cstddef>

int main() {
  Server::ServerConfig config = {
      8080,
  };

  StorageFactory storage_factory = StorageFactory();
  StorageType storage_type = StorageType::MemoryStorage;

  Server::Server server(config, storage_factory, storage_type);
  server.Start();
}
