#include "server.h"
#include "storage.h"
#include <iostream>

Server::Server *g_server = nullptr; // Global pointer to the Server instance

// Signal handler function
void signalHandler(int signal) {
  if (signal == SIGINT || signal == SIGTERM) {
    if (g_server != nullptr) {
      std::cout << "\n"
                << "Gracefully shutting down the server..." << "\n";
      delete g_server;
      g_server = nullptr;
    }
    exit(0); // Exit the program
  }
}
int main() {
  Server::ServerConfig config = {
      8080,
  };

  StorageFactory storage_factory = StorageFactory();
  StorageType storage_type = StorageType::MemoryStorage;

  signal(SIGINT, signalHandler);
  signal(SIGTERM, signalHandler);

  g_server = new Server::Server(config, storage_factory, storage_type);
  g_server->start();

  while (true) {
    pause(); // Pause the main thread until a signal is received
  }

  return 0;
}
