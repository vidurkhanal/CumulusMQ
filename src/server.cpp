#include "server.h"
#include <arpa/inet.h>
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

    handle_connection(connfd);
    close(connfd);
  }
}

void Server::die(const char *msg) {
  int err = errno;
  fprintf(stderr, "[%d] %s\n", err, msg);
  abort();
}

void Server::msg(const char *msg) { fprintf(stderr, "%s\n", msg); }

void Server::handle_connection(int connfd) {
  char rbuf[64] = {};
  ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1);
  if (n < 0) {
    msg("read() error");
    return;
  }
  printf("client says: %s\n", rbuf);

  char wbuf[] = "world\n";
  write(connfd, wbuf, strlen(wbuf));
}
