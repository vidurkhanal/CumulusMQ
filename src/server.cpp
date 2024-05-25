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

int32_t Server::handle_connection(int connfd) {
  char rbuf[4 + 4096 + 1];
  errno = 0;
  int32_t err = read_full(connfd, rbuf, 4);
  if (err) {
    if (errno == 0) {
      msg("EOF");
    } else {
      msg("read() error");
    }
    return err;
  }

  uint32_t len = 0;
  memcpy(&len, rbuf, 4); // assume little endian
  if (len > 4096) {
    msg("too long");
    return -1;
  }

  // request body
  err = read_full(connfd, &rbuf[4], len);
  if (err) {
    msg("read() error");
    return err;
  }

  // do something
  rbuf[4 + len] = '\0';
  printf("client says: %s\n", &rbuf[4]);

  // reply using the same protocol
  const char reply[] = "world";
  char wbuf[4 + sizeof(reply)];
  len = (uint32_t)strlen(reply);
  memcpy(wbuf, &len, 4);
  memcpy(&wbuf[4], reply, len);
  return write_all(connfd, wbuf, 4 + len);
}
