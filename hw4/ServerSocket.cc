/*
 * Copyright ©2023 Justin Hsia.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Washington
 * CSE 333 for use solely during Winter Quarter 2023 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

#include <stdio.h>       // for snprintf()
#include <unistd.h>      // for close(), fcntl()
#include <sys/types.h>   // for socket(), getaddrinfo(), etc.
#include <sys/socket.h>  // for socket(), getaddrinfo(), etc.
#include <arpa/inet.h>   // for inet_ntop()
#include <netdb.h>       // for getaddrinfo()
#include <errno.h>       // for errno, used by strerror()
#include <string.h>      // for memset, strerror()
#include <iostream>      // for std::cerr, etc.

#include "./ServerSocket.h"

extern "C" {
  #include "libhw1/CSE333.h"
}

namespace hw4 {

const int HOSTNAME_MAX = 1024;

ServerSocket::ServerSocket(uint16_t port) {
  port_ = port;
  listen_sock_fd_ = -1;
}

ServerSocket::~ServerSocket() {
  // Close the listening socket if it's not zero.  The rest of this
  // class will make sure to zero out the socket if it is closed
  // elsewhere.
  if (listen_sock_fd_ != -1)
    close(listen_sock_fd_);
  listen_sock_fd_ = -1;
}

bool ServerSocket::BindAndListen(int ai_family, int* const listen_fd) {
  // Use "getaddrinfo," "socket," "bind," and "listen" to
  // create a listening socket on port port_.  Return the
  // listening socket through the output parameter "listen_fd"
  // and set the ServerSocket data member "listen_sock_fd_"

  // STEP 1:
  // Populate the "hints" addrinfo structure for getaddrinfo()
  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = ai_family;      // Set family
  hints.ai_socktype = SOCK_STREAM;  // stream
  hints.ai_flags = AI_PASSIVE;      // use wildcard "in6addr_any" address
  hints.ai_flags |= AI_V4MAPPED;    // use v4-mapped v6 if no v6 found
  hints.ai_protocol = IPPROTO_TCP;  // tcp protocol
  hints.ai_canonname = nullptr;
  hints.ai_addr = nullptr;
  hints.ai_next = nullptr;

  // Call getaddrinfo() to find a local IP address for the given port
  struct addrinfo* result;
  char portnum[6];
  snprintf(portnum, sizeof(portnum), "%d", port_);
  int res = getaddrinfo(nullptr, portnum, &hints, &result);

  // Check for errors
  if (res != 0) {
    std::cerr << "getaddrinfo() failed: " << gai_strerror(res) << std::endl;
    return false;
  }

  // Loop through the returned address structures until we are able
  // to create a socket and bind to one of them
  int fd_temp = -1;
  for (struct addrinfo* rp = result; rp != nullptr; rp = rp->ai_next) {
    fd_temp = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (fd_temp == -1) {
      // Creating this socket failed.  So, loop to the next returned
      // result and try again.
      continue;
    }

    // Configure the socket
    // Set "SO_REUSEADDR", which tells the TCP stack to make the port we
    // bind to available again as soon as we exit, rather than waiting
    int optval = 1;
    setsockopt(fd_temp, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    // Try binding socket to address and port number from getaddrinfo()
    if (bind(fd_temp, rp->ai_addr, rp->ai_addrlen) == 0) {
      // Bind worked!
      // Set the output parameter to the network address family
      *listen_fd = fd_temp;
      // Set private data members
      listen_sock_fd_ = fd_temp;
      sock_family_ = rp->ai_family;
      break;
    }

    // The bind or listen failed.  Close the socket, then loop back around
    // and try the next address/port returned by getaddrinfo().
    close(fd_temp);
    fd_temp = -1;
  }

  // Free the addrinfo structure returned by getaddrinfo()
  freeaddrinfo(result);

  // Check to see if we were able to bind to a socket
  if (fd_temp == -1) {
    std::cerr << "Failed to create a socket or bind to a port" << std::endl;
    return false;
  }

  // Set the socket to listen for connections
  if (listen(fd_temp, 5) != 0) {
    std::cerr << "Failed to mark socket as listening: ";
    std::cerr << strerror(errno) << std::endl;
    close(fd_temp);
    return false;
  }

  return true;
}

bool ServerSocket::Accept(int* const accepted_fd,
                          std::string* const client_addr,
                          uint16_t* const client_port,
                          std::string* const client_dns_name,
                          std::string* const server_addr,
                          std::string* const server_dns_name) const {
  // Accept a new connection on the listening socket listen_sock_fd_.
  // (Block until a new connection arrives.)  Return the newly accepted
  // socket, as well as information about both ends of the new connection,
  // through the various output parameters.

  // STEP 2:
  int client_fd;
  while (1) {
    struct sockaddr_storage caddr;
    socklen_t caddrlen = sizeof(caddr);
    client_fd = accept(listen_sock_fd_,
                      reinterpret_cast<struct sockaddr*>(&caddr),
                      &caddrlen);
    if (client_fd < 0) {
      if ((errno == EINTR) || (errno == EAGAIN) || (errno == EWOULDBLOCK)) {
        // Try again
        continue;
      }
      std::cerr << "Failure on accept: " << strerror(errno) << std::endl;
      return false;
    }

    // client_fd is valid, set the output parameters
    *accepted_fd = client_fd;

    char hname_s[NI_MAXHOST];

    if (sock_family_ == AF_INET6) {
      // IPv6
      struct sockaddr_in6* caddr6 =
              reinterpret_cast<struct sockaddr_in6*>(&caddr);
      char astring[INET6_ADDRSTRLEN];

      // Get the client's IP address
      *client_addr = inet_ntop(AF_INET6, &(caddr6->sin6_addr),
                                astring, INET6_ADDRSTRLEN);

      // Get the client's port number
      *client_port = ntohs(caddr6->sin6_port);

      // Get the server's IP address
      struct sockaddr_in6 srvr;
      socklen_t srvrlen = sizeof(srvr);
      char addrbuf[INET6_ADDRSTRLEN];
      getsockname(client_fd,
                  reinterpret_cast<struct sockaddr*>(&srvr),
                  &srvrlen);
      inet_ntop(AF_INET6, &srvr.sin6_addr, addrbuf, INET6_ADDRSTRLEN);
      *server_addr = addrbuf;

      // Get the server's DNS name
      getnameinfo(reinterpret_cast<struct sockaddr*>(&srvr),
                  srvrlen, hname_s, sizeof(hname_s), nullptr, 0, 0);
    } else {
      // IPv4
      struct sockaddr_in* caddr4 =
              reinterpret_cast<struct sockaddr_in*>(&caddr);
      char astring[INET_ADDRSTRLEN];

      // Get the client's IP address
      *client_addr = inet_ntop(AF_INET, &(caddr4->sin_addr),
                                astring, INET_ADDRSTRLEN);

      // Get the client's port number
      *client_port = ntohs(caddr4->sin_port);

      // Get the server's IP address
      struct sockaddr_in srvr;
      socklen_t srvrlen = sizeof(srvr);
      char addrbuf[INET_ADDRSTRLEN];
      getsockname(client_fd,
                  reinterpret_cast<struct sockaddr*>(&srvr),
                  &srvrlen);
      inet_ntop(AF_INET, &srvr.sin_addr, addrbuf, INET_ADDRSTRLEN);
      *server_addr = addrbuf;

      // Get the server's DNS name
      getnameinfo(reinterpret_cast<struct sockaddr*>(&srvr),
                  srvrlen, hname_s, HOSTNAME_MAX, nullptr, 0, 0);
    }

    // Get the client's DNS name
    char hname_c[HOSTNAME_MAX];
    if (getnameinfo(reinterpret_cast<struct sockaddr*>(&caddr),
                    caddrlen, hname_c, HOSTNAME_MAX, nullptr, 0, 0) != 0) {
      snprintf(hname_c, sizeof(hname_c), "[reverse DNS failed]");
    }
    *client_dns_name = hname_c;

    // Set the server's DNS name
    *server_dns_name = hname_s;

    // We're done, exit the loop
    return true;
  }

  return false;
}

}  // namespace hw4
