#include "server.h"
#include "ioutils.h"
#include <arpa/inet.h>
#include <cassert>
#include <cstdint>
#include <errno.h>
#include <iostream>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

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

void Server::Start() {
  int rv = listen(fd_, SOMAXCONN);
  if (rv) {
    die("listen()");
  }

  std::cout << "Server started on port " << config_.port << "\n";
  while (true) {
    // accept
    struct sockaddr_in client_addr = {};
    socklen_t socklen = sizeof(client_addr);
    int connfd = accept(fd_, (struct sockaddr *)&client_addr, &socklen);
    if (connfd < 0) {
      continue; // error
    }

    while (true) {
      int32_t err = handle_connection(connfd);
      if (err) {
        break;
      }
    }
    close(connfd);
  }
}

void Server::die(const char *msg) {
  int err = errno;
  fprintf(stderr, "[%d] %s\n", err, msg);
  abort();
}

static CumulusActions get_action(uint8_t message_type) {
  switch (message_type) {
  case 0x01:
    return Subscribe;
  case 0x02:
    return Unsubscribe;
  case 0x03:
    return Publish;
  case 0x04:
    return Retrieve;
  default:
    return Unknown;
  }
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

  char rbuf[4 + 1 + 4 + 4 + 4096];
  errno = 0;

  int ptr = 0;

  // Read the Message Length
  int32_t err = read_exact(connfd, &rbuf[ptr], 4);
  if (err) {
    if (errno == 0) {
      msg("EOF");
    } else {
      msg("read() error");
    }
    return err;
  }

  uint32_t message_length = 0;
  memcpy(&message_length, rbuf, 4);
  if (message_length > 4096 + 4 + 1 + 4 + 4) {
    msg("too long");
    return -1;
  }
  printf("Message Length: %u\n", message_length);

  // Read and Process the Message Type byte
  ptr += 4;
  err = read_exact(connfd, &rbuf[ptr], 1);
  if (err) {
    msg("read() error");
    return err;
  }

  uint8_t message_type = 0;
  memcpy(&message_type, &rbuf[ptr], 1);

  CumulusActions action = get_action(message_type);
  if (action == Unknown) {
    msg("unknown action");
    return -1;
  }

  printf("Action: %u\n", action);

  // Read the Topic Length
  ptr += 1;
  err = read_exact(connfd, &rbuf[ptr], 4);
  if (err) {
    msg("read() error");
    return err;
  }

  uint32_t topic_length = 0;
  memcpy(&topic_length, &rbuf[ptr], 4);
  printf("Topic Length: %u\n", topic_length);

  // read the topic
  ptr += 4;
  err = read_exact(connfd, &rbuf[ptr], topic_length);
  if (err) {
    msg("read()");
    return err;
  }

  std::string topic(&rbuf[ptr], topic_length);
  printf("Topic: %s\n", topic.c_str());

  // Read the Body Length
  ptr += topic_length;
  err = read_exact(connfd, &rbuf[ptr], 4);
  if (err) {
    msg("read() error");
    return err;
  }

  uint32_t body_length = 0;
  memcpy(&body_length, &rbuf[ptr], 4);
  if (message_length > 4096) {
    msg("too long");
    return -1;
  }
  printf("Body Length: %u\n", body_length);

  // read the body
  ptr += 4;
  err = read_exact(connfd, &rbuf[ptr], body_length);
  if (err) {
    msg("read()");
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
  return write_all(connfd, wbuf, 4 + message_length);
}
