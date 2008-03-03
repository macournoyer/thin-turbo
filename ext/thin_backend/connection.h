#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include <sys/types.h>
#include <arpa/inet.h>

#include <ev.h>

#include "backend.h"

typedef struct thin_buffer {
  size_t len;
  u_char *data;
} thin_buffer_t;

typedef struct thin_connection {
  unsigned open : 1;
  int fd;
  struct sockaddr_in remote_addr;
  
  struct thin_buffer read_buffer;
  struct thin_buffer write_buffer;

  ev_io read_watcher;
  ev_io write_watcher;  
} thin_connection_t;

#define thin_buffer(str) { sizeof(str) - 1, (u_char *) str }

void thin_start_connection(struct thin_backend *server, int fd, struct sockaddr_in remote_addr);
void thin_connection_recv(EV_P_ struct ev_io *watcher, int revents);
void thin_connection_send(EV_P_ struct ev_io *watcher, int revents);
void thin_connection_close(EV_P_ struct ev_io *watcher, int revents);

#endif /* _CONNECTION_H_ */
