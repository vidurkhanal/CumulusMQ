#include <cumulusmq/conn.h>
#include <cumulusmq/server.h>
#include <cumulusmq/storage.h>
#include <iostream>
#include <mutex>

std::mutex g_server_mutex; // Mutex to protect the g_server pointer
std::unique_ptr<Server::Server> g_server =
    nullptr; // Global pointer to the Server instance

// Signal handler function
void signalHandler(int signal) {
  if (signal == SIGINT || signal == SIGTERM) {
    std::lock_guard<std::mutex> lock(g_server_mutex); // Lock the mutex
    if (g_server) {
      std::cout << "\n"
                << "Gracefully shutting down the server..." << "\n";
      g_server.reset(); // Release the Server object and its resources
    }
    exit(0); // Exit the program
  }
}

int main() {
  Server::ServerConfig config = {
      8080,
  };
  StorageFactory storage_factory = StorageFactory();
  Env env{StorageType::MemoryStorage};
  const char *storage_type = getenv("CUMULUS_STORAGE_TYPE");
  env.storage_type = storage_type != NULL ? parseStorageType(storage_type)
                                          : StorageType::MemoryStorage;

  signal(SIGTERM, signalHandler);
  signal(SIGINT, signalHandler);

  try {
    std::lock_guard<std::mutex> lock(g_server_mutex); // Lock the mutex
    g_server = std::make_unique<Server::Server>(config, storage_factory, &env);
    g_server->start();
  } catch (...) {
    std::cerr << "An error occurred during server initialization." << std::endl;
    return 1;
  }

  while (true) {
    pause(); // Pause the main thread until a signal is received
  }

  return 0;
}
