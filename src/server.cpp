#include <arpa/inet.h>
#include <cassert>
#include <cstdint>
#include <cumulusmq/conn.h>
#include <cumulusmq/ioutils.h>
#include <cumulusmq/server.h>
#include <cumulusmq/storage.h>
#include <cumulusmq/utils.h>
#include <errno.h>
#include <iostream>
#include <netinet/ip.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

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

Server::Server(ServerConfig config, StorageFactory storage_factory, Env *env)
    : config(config), storage_factory(storage_factory), env(env) {
  fd_ = socket(AF_INET /* says use IPv4*/, SOCK_STREAM /*use TCP*/,
               0 /*use default protocol*/);

  if (fd_ < 0) {
    /* socket syscall failed */
    die("socket()");
  }

  /* Setting optiions for socket */
  int val = 1;
  setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

  // bind and store information about the socket
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

Server::~Server() {
  // Close the server socket
  close(fd_);

  // Delete all the Topic objects
  for (auto &pair : topics) {
    delete pair.second;
  }
}

bool Server::tryFlushBuffer(Conn::Conn *conn) {
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

void Server::stateRes(Conn::Conn *conn) {
  while (tryFlushBuffer(conn)) {
  }
}

void Server::handleQuery(Actions action, const char *topic_name,
                         const char *body, Conn::Conn *conn) {
  switch (action) {
  case Actions::Subscribe: {
    bool created = createTopic(topic_name);
    if (created) {
      std::string msg = "OK";
      const int msg_len = static_cast<int>(msg.size());
      memcpy(&conn->wbuf[0], &msg_len, 4);
      memcpy(&conn->wbuf[4], msg.c_str(), msg_len);
      conn->wbuf_size = 4 + msg_len;
    } else {
      std::string msg = "EXISTS";
      const int msg_len = static_cast<int>(msg.size());
      memcpy(&conn->wbuf[0], &msg_len, 4);
      memcpy(&conn->wbuf[4], msg.c_str(), msg_len);
      conn->wbuf_size = 4 + msg_len;
    }
    break;
  }
  case Actions::Unsubscribe: {
    bool deleted = deleteTopic(topic_name);
    if (deleted) {
      int msg_len = 2;
      std::string msg = "OK";
      memcpy(&conn->wbuf[0], &msg_len, 4);
      memcpy(&conn->wbuf[4], &msg, msg_len);
      conn->wbuf_size = 4 + msg_len;
    } else {
      int msg_len = 7;
      const char *msg = "NOTFOUND";
      memcpy(&conn->wbuf[0], &msg_len, 4);
      memcpy(&conn->wbuf[4], &msg, msg_len);
      conn->wbuf_size = 4 + msg_len;
    }
    break;
  }
  case Actions::Publish: {
    try {
      Topic *topic = getTopic(topic_name);
      topic->publish(body);
      std::string msg = "OK";
      const int msg_len = static_cast<int>(msg.size());
      memcpy(&conn->wbuf[0], &msg_len, 4);
      memcpy(&conn->wbuf[4], msg.c_str(), msg_len);
      conn->wbuf_size = 4 + msg_len;
    } catch (std::runtime_error &e) {
      const std::string msg = "NOTFOUND";
      const int msg_len = static_cast<int>(msg.size());
      memcpy(&conn->wbuf[0], &msg_len, 4);
      memcpy(&conn->wbuf[4], msg.c_str(), msg_len);
      conn->wbuf_size = 4 + msg_len;
    }
    break;
  }
  case Actions::Retrieve: {
    try {
      Topic *topic = getTopic(topic_name);
      int offset = stringToInt(body);
      const char *data = topic->consume(0);
    } catch (std::runtime_error &e) {
      const std::string msg = "NOTFOUND";
      const int msg_len = static_cast<int>(msg.size());
      memcpy(&conn->wbuf[0], &msg_len, 4);
      memcpy(&conn->wbuf[4], msg.c_str(), msg_len);
      conn->wbuf_size = 4 + msg_len;
    } catch (std::invalid_argument &e) {
      const std::string msg = "INVALID_OFFSET";
      const int msg_len = static_cast<int>(msg.size());
      memcpy(&conn->wbuf[0], &msg_len, 4);
      memcpy(&conn->wbuf[4], msg.c_str(), msg_len);
      conn->wbuf_size = 4 + msg_len;
    } catch (std::out_of_range &e) {
      const std::string msg = "OUT_OF_RANGE";
      const int msg_len = static_cast<int>(msg.size());
      memcpy(&conn->wbuf[0], &msg_len, 4);
      memcpy(&conn->wbuf[4], msg.c_str(), msg_len);
      conn->wbuf_size = 4 + msg_len;
    }
    break;
  }
  case Actions::Unknown:
    // unreachable
    break;
  }
}

bool Server::handleConnection(Conn::Conn *conn) {
  if (conn->rbuf_size < 4) {
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

  /* printf("Message Length: %u\n", len); */
  ptr += 4;

  if (ptr + len > conn->rbuf_size) {
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
  /* printf("Action: %u\n", action); */

  uint32_t topic_length = 0;
  memcpy(&topic_length, &conn->rbuf[ptr], 4);
  if (topic_length > MAX_MESSAGE_SIZE - ptr) {
    IOUtils::msg("invalid topic length");
    conn->state = Conn::STATE_END;
    return false;
  }
  /* printf("Topic Length: %u\n", topic_length); */
  ptr += 4;

  char topic_buf[topic_length + 1];
  memcpy(&topic_buf, &conn->rbuf[ptr], topic_length);
  topic_buf[topic_length] = '\0';
  /* printf("Topic: %s\n", topic_buf); */

  ptr += topic_length;

  uint32_t body_length = 0;
  memcpy(&body_length, &conn->rbuf[ptr], 4);
  if (body_length > MAX_MESSAGE_SIZE - ptr) {
    IOUtils::msg("invalid body length");
    conn->state = Conn::STATE_END;
    return false;
  }
  /* printf("Body Length: %u\n", body_length); */
  ptr += 4;

  char body_buf[body_length + 1];
  memcpy(&body_buf, &conn->rbuf[ptr], body_length);
  body_buf[body_length] = '\0';
  /* printf("Body: %s\n", body_buf); */

  ptr += body_length;

  handleQuery(action, topic_buf, body_buf, conn);

  /* memcpy(&conn->wbuf[0], &len, 4); */
  /* memcpy(&conn->wbuf[4], &conn->rbuf[4], len); */
  /* conn->wbuf_size = 4 + len; */

  size_t remain = conn->rbuf_size - 4 - len;
  if (remain) {
    memmove(conn->rbuf, &conn->rbuf[4 + len], remain);
  }
  conn->rbuf_size = remain;

  conn->state = Conn::STATE_RES;
  stateRes(conn);

  return (conn->state == Conn::STATE_REQ);
}

bool Server::tryFillBuffer(Conn::Conn *conn) {
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
  while (handleConnection(conn)) {
  }
  return (conn->state == Conn::STATE_REQ);
}

void Server::stateReq(Conn::Conn *conn) {
  while (tryFillBuffer(conn)) {
  };
}

void Server::connectionIO(Conn::Conn *conn) {
  if (conn->state == Conn::STATE_REQ) {
    stateReq(conn);
  } else if (conn->state == Conn::STATE_RES) {
    stateRes(conn);
  } else {
    assert(0); // not expected
  }
}

void Server::start() {
  int rv = listen(fd_, SOMAXCONN);
  if (rv) {
    die("listen()");
  }

  std::cout << "Server started on port " << config.port << "\n";
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
        connectionIO(conn);
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
  }
}

bool Server::createTopic(const char *topic_name) {
  if (topics.find(topic_name) == topics.end()) {
    Storage *storage = storage_factory.Build(env->storage_type);
    Topic topic(topic_name, storage);
    topics[topic_name] = &topic;
    return true;
  }
  return false;
}

bool Server::deleteTopic(const char *topic_name) {
  if (topics.find(topic_name) != topics.end()) {
    topics.erase(topic_name);
    return true;
  }
  return false;
}

Topic *Server::getTopic(const char *topic_name) {
  if (topics.find(topic_name) != topics.end()) {
    throw std::runtime_error("Topic not found");
  }
  return topics[topic_name];
}

void die(const char *msg) {
  int err = errno;
  fprintf(stderr, "[%d] %s\n", err, msg);
  abort();
}

} // namespace Server
