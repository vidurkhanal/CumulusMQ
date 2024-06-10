#ifndef SERVER_H
#define SERVER_H

#include "conn.h"
#include "storage.h"
#include "topic.h"
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <unistd.h>
#include <unordered_map>

namespace Server {

enum Actions { Subscribe, Unsubscribe, Publish, Retrieve, Unknown };

struct ServerConfig {
  int port;
};

void die(const char *msg);

class Server {
public:
  Server(ServerConfig config, StorageFactory storage_factory,
         StorageType storage_type);
  ~Server();
  void start();

private:
  ServerConfig config;
  StorageFactory storage_factory;
  StorageType storage_type;
  int fd_;
  std::unordered_map<const char *, Topic *> topics;
  Topic *getTopic(const char *name);
  bool handleConnection(Conn::Conn *conn);
  void handleQuery(Actions action, const char *topic, const char *body,
                   Conn::Conn *conn);
  void connectionIO(Conn::Conn *conn);
  void stateReq(Conn::Conn *conn);
  bool tryFillBuffer(Conn::Conn *conn);
  void stateRes(Conn::Conn *conn);
  bool tryFlushBuffer(Conn::Conn *conn);
  bool createTopic(const char *topic_name);
  bool deleteTopic(const char *topic_name);
};
} // namespace Server

#endif // SERVER_H
