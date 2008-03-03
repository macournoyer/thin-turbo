#ifndef _BACKEND_H_
#define _BACKEND_H_

#include <sys/types.h>
#include <arpa/inet.h>

#define EV_STANDALONE 1
#include <ev.h>

#include "ruby.h"

#define CONNECTION_POOL_SIZE 300
#define LISTEN_BACKLOG       300     // how many pending connections queue will hold

static VALUE cBackend;

typedef struct thin_backend thin_backend;

struct thin_backend {
  char *address;
  int port;
  
  unsigned open : 1;
  int fd;
  struct sockaddr_in local_addr;

  ev_io accept_watcher;
  struct ev_loop *loop;
};

VALUE thin_backend_init(VALUE self, VALUE address, VALUE port);
VALUE thin_backend_start(VALUE self);
VALUE thin_backend_stop(VALUE self);
void thin_backend_accept(EV_P_ struct ev_io *watcher, int revents);
void thin_backend_free(struct thin_backend *backend);
VALUE thin_backend_alloc(VALUE self);

#endif /* _BACKEND_H_ */
