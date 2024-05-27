#ifndef SERVER_H
#define SERVER_H

#include "storage.h"
#include <arpa/inet.h>
#include <cstdint>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <unistd.h>

namespace Server {

enum Actions { Subscribe, Unsubscribe, Publish, Retrieve, Unknown };

const int MAX_MESSAGE_SIZE = 5112;

struct ServerConfig {
  int port;
  Storage *storage;
};

void die(const char *msg);

class Server {
public:
  Server(ServerConfig config);
  void Start();

private:
  ServerConfig config_;
  int fd_;
  static int32_t handle_connection(int connfd);
};

} // namespace Server

#endif // SERVER_H
