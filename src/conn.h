#ifndef CONN_H
#define CONN_H

#include "server.h"
#include <cstddef>
#include <cstdint>

namespace Conn {
enum {
  STATE_REQ = 0,
  STATE_RES = 1,
  STATE_END = 2, // mark the connection for deletion
};

void fd_set_nb(int fd);

struct Conn {
  int fd = -1;
  uint32_t state = 0; // either STATE_REQ or STATE_RES
  // buffer for reading
  size_t rbuf_size = 0;
  uint8_t rbuf[4 + Server::MAX_MESSAGE_SIZE];
  // buffer for writing
  size_t wbuf_size = 0;
  size_t wbuf_sent = 0;
  uint8_t wbuf[4 + Server::MAX_MESSAGE_SIZE];
};

void conn_put(std::vector<Conn *> &fd2conn, struct Conn *conn);
int32_t accept_new_conn(std::vector<Conn *> &fd2conn, int fd);
} // namespace Conn

#endif // CONN_H
