#ifndef _BACKEND_H_
#define _BACKEND_H_

#include <sys/types.h>
#include <arpa/inet.h>

#include <ev.h>

#include "ruby.h"

#define CONNECTION_POOL_SIZE 300
#define LISTEN_BACKLOG       300     // how many pending connections queue will hold

#define BACKEND_GET(B) struct thin_backend * ##B = NULL; DATA_GET(self, thin_backend, ##B)

typedef struct thin_backend {
  char *address;
  int port;
  
  unsigned open : 1;
  int fd;
  struct sockaddr_in local_addr;

  ev_io accept_watcher;
  
  struct thin_connection *connections[CONNECTION_POOL_SIZE];
  
  struct ev_loop *loop;
} thin_backend_t;

void thin_backend_init(VALUE self, VALUE address, VALUE port)
void thin_backend_start(VALUE self);
void thin_backend_accept(EV_P_ struct ev_io *watcher, int revents);
void thin_backend_free(thin_backend *backend);
VALUE thin_backend_alloc(VALUE self);

#endif /* _BACKEND_H_ */
