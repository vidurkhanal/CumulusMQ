#ifndef SERVER_H
#define SERVER_H

#include "storage.h"
#include "topic.h"
#include <arpa/inet.h>
#include <cstdint>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <unistd.h>
#include <unordered_map>

namespace Server {

enum Actions { Subscribe, Unsubscribe, Publish, Retrieve, Unknown };

const int MAX_MESSAGE_SIZE = 5112;

struct ServerConfig {
  int port;
};

void die(const char *msg);

class Server {
public:
  Server(ServerConfig config, StorageFactory storage_factory,
         StorageType storage_type);
  void Start();

private:
  ServerConfig config;
  StorageFactory storage_factory;
  StorageType storage_type;
  int fd_;
  std::unordered_map<std::string, Topic> topics;
  int32_t handle_connection(int connfd);
};

} // namespace Server

#endif // SERVER_H
