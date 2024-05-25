#include "server.h"
#include "storage.h"
#include <cstddef>

int main() {
  Memory memory;

  ServerConfig config = {8080, (Storage *)&memory};
  Server server(config);
  server.Start();
}
