#ifndef SERVER_H
#define SERVER_H

#include "storage.h"
#include <arpa/inet.h>
#include <cstdint>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <unistd.h>
#define MAXMSG 4096

enum CumulusActions { Subscribe, Unsubscribe, Publish, Retrieve, Unknown };

struct ServerConfig {
  int port;
  Storage *storage;
};

class Server {
public:
  Server(ServerConfig config);
  void Start();
  void die(const char *msg);

private:
  ServerConfig config_;
  int fd_;
  static int32_t handle_connection(int connfd);
};

#endif // SERVER_H
