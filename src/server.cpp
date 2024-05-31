#include "server.h"
#include "conn.h"
#include "ioutils.h"
#include <arpa/inet.h>
#include <cassert>
#include <cstdint>
#include <errno.h>
#include <iostream>
#include <netinet/ip.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

namespace Server {

static Actions get_action(uint8_t message_type) {
  switch (message_type) {
  case 0x01:
    return Actions::Subscribe;
  case 0x02:
    return Actions::Unsubscribe;
  case 0x03:
    return Actions::Publish;
  case 0x04:
    return Actions::Retrieve;
  default:
    return Actions::Unknown;
  }
}

Server::Server(ServerConfig config) : config_(config) {
  fd_ = socket(AF_INET /* says use IPv4*/, SOCK_STREAM /*use TCP*/,
               0 /*use default protocol*/);

  if (fd_ < 0) {
    /* socket syscall failed */
    die("socket()");
  }

  /* Setting optiions for socket */
  int val = 1;
  setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

  // bind and store informations about the scoket
  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = ntohs(config.port);
  addr.sin_addr.s_addr = ntohl(0 /* wildcard address 0.0.0.0*/);

  /* Bind the socket to the address:port */
  int rv = bind(fd_, (const sockaddr *)&addr, sizeof(addr));
  if (rv) {
    /* bind syscall failed */
    die("bind()");
  }
}

static bool try_flush_buffer(Conn::Conn *conn) {
  ssize_t rv = 0;
  do {
    size_t remain = conn->wbuf_size - conn->wbuf_sent;
    rv = write(conn->fd, &conn->wbuf[conn->wbuf_sent], remain);
  } while (rv < 0 && errno == EINTR);
  if (rv < 0 && errno == EAGAIN) {
    // got EAGAIN, stop.
    return false;
  }
  if (rv < 0) {
    IOUtils::msg("write() error");
    conn->state = Conn::STATE_END;
    return false;
  }
  conn->wbuf_sent += (size_t)rv;
  assert(conn->wbuf_sent <= conn->wbuf_size);
  if (conn->wbuf_sent == conn->wbuf_size) {
    // response was fully sent, change state back
    conn->state = Conn::STATE_REQ;
    conn->wbuf_sent = 0;
    conn->wbuf_size = 0;
    return false;
  }
  // still got some data in wbuf, could try to write again
  return true;
}

static void state_res(Conn::Conn *conn) {
  while (try_flush_buffer(conn)) {
  }
}

static bool try_one_request(Conn::Conn *conn) {
  // try to parse a request from the buffer
  if (conn->rbuf_size < 4) {
    // not enough data in the buffer. Will retry in the next iteration
    return false;
  }

  uint32_t len = 0;
  uint32_t ptr = 0;
  memcpy(&len, &conn->rbuf[ptr], 4);
  if (len > MAX_MESSAGE_SIZE) {
    IOUtils::msg("too long");
    conn->state = Conn::STATE_END;
    return false;
  }

  printf("Message Length: %u\n", len);
  ptr += 4;

  if (ptr + len > conn->rbuf_size) {
    // not enough data in the buffer. Will retry in the next iteration
    return false;
  }

  uint8_t message_type = 0;
  memcpy(&message_type, &conn->rbuf[ptr], 1);
  Actions action = get_action(message_type);
  if (action == Actions::Unknown) {
    IOUtils::msg("unknown action");
    conn->state = Conn::STATE_END;
    return false;
  }
  ptr += 1;
  printf("Action: %u\n", action);

  uint32_t topic_length = 0;
  memcpy(&topic_length, &conn->rbuf[ptr], 4);
  printf("Topic Length: %u\n", topic_length);
  ptr += 4;

  char topic_buf[topic_length + 1];
  memcpy(&topic_buf, &conn->rbuf[ptr], topic_length);
  topic_buf[topic_length] = '\0';
  printf("Topic: %s\n", topic_buf);
  ptr += topic_length;

  uint32_t body_length = 0;
  memcpy(&body_length, &conn->rbuf[ptr], 4);
  printf("body Length: %u\n", body_length);
  ptr += 4;

  char body_buf[body_length + 1];
  memcpy(&body_buf, &conn->rbuf[ptr], body_length);
  topic_buf[body_length] = '\0';
  printf("Body: %s\n", body_buf);
  ptr += body_length;

  // got one request, do something with it
  /* printf("client says: %.*s\n", len, &conn->rbuf[4]); */

  // generating echoing response
  memcpy(&conn->wbuf[0], &len, 4);
  memcpy(&conn->wbuf[4], &conn->rbuf[4], len);
  conn->wbuf_size = 4 + len;

  // remove the request from the buffer.
  // TODO: frequent memmove is inefficient.
  // TODO: need better handling for production code.
  size_t remain = conn->rbuf_size - 4 - len;
  if (remain) {
    memmove(conn->rbuf, &conn->rbuf[4 + len], remain);
  }
  conn->rbuf_size = remain;

  // change state
  conn->state = Conn::STATE_RES;
  state_res(conn);

  // continue the outer loop if the request was fully processed
  return (conn->state == Conn::STATE_REQ);
}

static bool try_fill_buffer(Conn::Conn *conn) {
  // try to fill the buffer
  assert(conn->rbuf_size < sizeof(conn->rbuf));
  ssize_t rv = 0;
  do {
    size_t cap = sizeof(conn->rbuf) - conn->rbuf_size;
    rv = read(conn->fd, &conn->rbuf[conn->rbuf_size], cap);
  } while (rv < 0 && errno == EINTR);

  if (rv < 0 && errno == EAGAIN) {
    // got EAGAIN, stop.
    return false;
  }

  if (rv < 0) {
    IOUtils::msg("read() error");
    conn->state = Conn::STATE_END;
    return false;
  }

  if (rv == 0) {
    if (conn->rbuf_size > 0) {
      IOUtils::msg("unexpected EOF");
    } else {
      IOUtils::msg("EOF");
    }
    conn->state = Conn::STATE_END;
    return false;
  }

  conn->rbuf_size += (size_t)rv;
  assert(conn->rbuf_size <= sizeof(conn->rbuf));

  // Try to process requests one by one.
  // Why is there a loop? Please read the explanation of "pipelining".
  while (try_one_request(conn)) {
  }
  return (conn->state == Conn::STATE_REQ);
}

static void state_req(Conn::Conn *conn) {
  while (try_fill_buffer(conn)) {
  };
}

static void connection_io(Conn::Conn *conn) {
  if (conn->state == Conn::STATE_REQ) {
    state_req(conn);
  } else if (conn->state == Conn::STATE_RES) {
    state_res(conn);
  } else {
    assert(0); // not expected
  }
}

void Server::Start() {
  int rv = listen(fd_, SOMAXCONN);
  if (rv) {
    die("listen()");
  }

  std::cout << "Server started on port " << config_.port << "\n";
  // a map of all client connections, keyed by fd
  std::vector<Conn::Conn *> fd2conn;

  // set the listen fd to nonblocking mode
  Conn::fd_set_nb(fd_);

  std::vector<struct pollfd> poll_args;

  while (true) {
    // prepare the arguments of the poll()
    poll_args.clear();
    // for convenience, the listening fd is put in the first position
    struct pollfd pfd = {fd_, POLLIN, 0};

    poll_args.push_back(pfd);

    // connection fds
    for (Conn::Conn *conn : fd2conn) {
      if (!conn) {
        continue;
      }
      struct pollfd pfd = {};
      pfd.fd = conn->fd;
      pfd.events = (conn->state == Conn::STATE_REQ) ? POLLIN : POLLOUT;
      pfd.events = pfd.events | POLLERR;
      poll_args.push_back(pfd);
    }

    // poll for active fds
    // the timeout argument doesn't matter here
    int rv = poll(poll_args.data(), (nfds_t)poll_args.size(), 1000);
    if (rv < 0) {
      die("poll");
    }

    // process active connections
    for (size_t i = 1; i < poll_args.size(); ++i) {
      if (poll_args[i].revents) {
        Conn::Conn *conn = fd2conn[poll_args[i].fd];
        connection_io(conn);
        if (conn->state == Conn::STATE_END) {
          // client closed normally, or something bad happened.
          // destroy this connection
          fd2conn[conn->fd] = NULL;
          (void)close(conn->fd);
          free(conn);
        }
      }
    }

    // try to accept a new connection if the listening fd is active
    if (poll_args[0].revents) {
      (void)Conn::accept_new_conn(fd2conn, fd_);
    }

    /* // accept */
    /* struct sockaddr_in client_addr = {}; */
    /* socklen_t socklen = sizeof(client_addr); */
    /* int connfd = accept(fd_, (struct sockaddr *)&client_addr, &socklen); */
    /* if (connfd < 0) { */
    /*   continue; // error */
    /* } */
    /**/
    /* while (true) { */
    /*   int32_t err = handle_connection(connfd); */
    /*   if (err) { */
    /*     break; */
    /*   } */
    /* } */
    /* close(connfd); */
  }
}

void die(const char *msg) {
  int err = errno;
  fprintf(stderr, "[%d] %s\n", err, msg);
  abort();
}

int32_t Server::handle_connection(int connfd) {

  /* | Message Length | Message Type | Body Length | Message Body |   */
  /* |----------------|--------------|-------------|--------------| */
  /* | 4 bytes        | 1 byte       | 4 bytes     | n bytes(Max of 4MB)      |
   */

  /* | Message Type Value | Action      | */
  /* |--------------------|-------------| */
  /* | 0x01               | Subscribe   | */
  /* | 0x02               | Unsubscribe | */
  /* | 0x03               | Publish     | */
  /* | 0x04               | Retrieve    | */

  char rbuf[MAX_MESSAGE_SIZE];
  errno = 0;

  int ptr = 0;

  // Read the Message Length
  int32_t err = IOUtils::read_exact(connfd, &rbuf[ptr], 4);
  if (err) {
    if (errno == 0) {
      IOUtils::msg("EOF");
    } else {
      IOUtils::msg("read() error");
    }
    return err;
  }

  uint32_t message_length = 0;
  memcpy(&message_length, rbuf, 4);
  if (message_length > MAX_MESSAGE_SIZE) {
    IOUtils::msg("too long");
    return -1;
  }
  printf("Message Length: %u\n", message_length);

  // Read and Process the Message Type byte
  ptr += 4;
  err = IOUtils::read_exact(connfd, &rbuf[ptr], 1);
  if (err) {
    IOUtils::msg("read() error");
    return err;
  }

  uint8_t message_type = 0;
  memcpy(&message_type, &rbuf[ptr], 1);

  Actions action = get_action(message_type);
  if (action == Actions::Unknown) {
    IOUtils::msg("unknown action");
    return -1;
  }

  printf("Action: %u\n", action);

  // Read the Topic Length
  ptr += 1;
  err = IOUtils::read_exact(connfd, &rbuf[ptr], 4);
  if (err) {
    IOUtils::msg("read() error");
    return err;
  }

  uint32_t topic_length = 0;
  memcpy(&topic_length, &rbuf[ptr], 4);
  printf("Topic Length: %u\n", topic_length);

  // read the topic
  ptr += 4;
  err = IOUtils::read_exact(connfd, &rbuf[ptr], topic_length);
  if (err) {
    IOUtils::msg("read()");
    return err;
  }

  std::string topic(&rbuf[ptr], topic_length);
  printf("Topic: %s\n", topic.c_str());

  // Read the Body Length
  ptr += topic_length;
  err = IOUtils::read_exact(connfd, &rbuf[ptr], 4);
  if (err) {
    IOUtils::msg("read() error");
    return err;
  }

  uint32_t body_length = 0;
  memcpy(&body_length, &rbuf[ptr], 4);
  if (message_length > 4096) {
    IOUtils::msg("too long");
    return -1;
  }
  printf("Body Length: %u\n", body_length);

  // read the body
  ptr += 4;
  err = IOUtils::read_exact(connfd, &rbuf[ptr], body_length);
  if (err) {
    IOUtils::msg("read()");
    return err;
  }

  std::string body(&rbuf[ptr], body_length);
  printf("Body: %s\n", body.c_str());

  ptr += body_length;
  rbuf[ptr] = '\0';

  // reply using the same protocol
  const char reply[] = "world";
  char wbuf[4 + sizeof(reply)];
  message_length = (uint32_t)strlen(reply);
  memcpy(wbuf, &message_length, 4);
  memcpy(&wbuf[4], reply, message_length);
  return IOUtils::write_all(connfd, wbuf, 4 + message_length);
}
} // namespace Server
