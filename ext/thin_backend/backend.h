#ifndef _BACKEND_H_
#define _BACKEND_H_

#include <sys/types.h>
#include <arpa/inet.h>

#define EV_STANDALONE 1
#include <ev.h>

#include "ruby.h"
#include "parser.h"

#define CONNECTION_POOL_SIZE 10
#define LISTEN_BACKLOG       300     // how many pending connections queue will hold
#define BUFFER_SIZE          1024

typedef struct thin_backend thin_backend;
typedef struct thin_buffer thin_buffer;
typedef struct thin_connection thin_connection;

struct thin_buffer {
  size_t len;
  char data[BUFFER_SIZE];
};

struct thin_connection {
  unsigned open : 1;
  int fd;
  struct sockaddr_in remote_addr;
  
  struct thin_buffer read_buffer;
  struct thin_buffer write_buffer;
  
  http_parser parser;
  VALUE env;
  
  struct thin_backend *backend;

  ev_io read_watcher;
  ev_io write_watcher;  
};

struct thin_backend {
  char *address;
  int port;
  
  unsigned open : 1;
  int fd;
  struct sockaddr_in local_addr;

  ev_io accept_watcher;
  ev_signal signal_watcher;
  
  VALUE rb_obj;
  
  struct thin_connection connections[CONNECTION_POOL_SIZE];
  
  struct ev_loop *loop;
};

VALUE thin_backend_init(VALUE self, VALUE address, VALUE port);
VALUE thin_backend_start(VALUE self);
VALUE thin_backend_loop(VALUE self);
VALUE thin_backend_process(VALUE self);
VALUE thin_backend_stop(VALUE self);
void thin_backend_accept_cb(EV_P_ struct ev_io *watcher, int revents);
void thin_backend_signal_cb(EV_P_ struct ev_signal *watcher, int revents);
void thin_backend_free(struct thin_backend *backend);
VALUE thin_backend_alloc(VALUE self);

#endif /* _BACKEND_H_ */
