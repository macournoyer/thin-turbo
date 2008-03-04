#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include <sys/types.h>
#include <arpa/inet.h>

#define EV_STANDALONE 1
#include <ev.h>

#include "backend.h"
#include "parser.h"

typedef struct thin_buffer thin_buffer;
typedef struct thin_connection thin_connection;

struct thin_buffer {
  size_t len;
  u_char *data;
};

struct thin_connection {
  unsigned open : 1;
  int fd;
  struct sockaddr_in remote_addr;
  
  struct thin_buffer read_buffer;
  struct thin_buffer write_buffer;
  
  http_parser parser;
  VALUE env;

  ev_io read_watcher;
  ev_io write_watcher;  
};

void thin_start_connection(struct thin_backend *server, int fd, struct sockaddr_in remote_addr);
void thin_connection_recv(EV_P_ struct ev_io *watcher, int revents);
void thin_connection_send(EV_P_ struct ev_io *watcher, int revents);
void thin_connection_close(EV_P_ struct ev_io *watcher, int revents);

#endif /* _CONNECTION_H_ */
