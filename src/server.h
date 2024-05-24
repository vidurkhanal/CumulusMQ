#ifndef SERVER_H
#define SERVER_H

#include "storage.h"
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <unistd.h>

struct ServerConfig {
  int port;
  Storage *storage;
};

class Server {
public:
  Server(ServerConfig config);
  void Start();

private:
  ServerConfig config_;
  int fd_;

  static void die(const char *msg);
  static void msg(const char *msg);
  static void handle_connection(int connfd);
};

#endif // SERVER_H
