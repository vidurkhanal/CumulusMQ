#include "server.h"
#include "storage.h"
#include <cstddef>

int main() {
  Memory memory;

  Server::ServerConfig config = {8080, (Storage *)&memory};
  Server::Server server(config);
  server.Start();
}
