#ifndef _THIN_H_
#define _THIN_H_

#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define EV_STANDALONE 1
#include <ev.h>

#include "ruby.h"
#include "ext_help.h"
#include "parser.h"
#include "array.h"

#ifdef __FreeBSD__
#define THIN_LISTEN_BACKLOG    -1
#else
#define THIN_LISTEN_BACKLOG    511
#endif
#define THIN_CONNECTIONS_SIZE  300
#define THIN_BUFFER_SIZE       1024

#define THIN_OK     0
#define THIN_ERROR -1

typedef struct thin_backend_s thin_backend_t;
typedef struct thin_buffer_s thin_buffer_t;
typedef struct thin_connection_s thin_connection_t;

struct thin_buffer_s {
  size_t  len;
  char   *ptr;
};

struct thin_connection_s {
  unsigned            open : 1;
  int                 fd;
  struct sockaddr_in  remote_addr;
  
  thin_buffer_t       read_buffer;
  
  http_parser         parser;
  VALUE               env;
  
  int                 status;
  VALUE               headers;
  thin_buffer_t       body;
  size_t              content_length;
  
  thin_backend_t     *backend;

  struct ev_loop     *loop;
  ev_io               read_watcher;
  ev_io               write_watcher;  
};

struct thin_backend_s {
  char               *address;
  unsigned            port;
  
  unsigned            open : 1;
  int                 fd;
  struct sockaddr_in  local_addr;

  ev_io               accept_watcher;
  
  VALUE               app;
  
  thin_array_t       *connections;
  
  struct ev_loop     *loop;
};

#define watch(conn, cb, event, ev_event) \
  ev_io_init(&conn->event##_watcher, cb, conn->fd, ev_event); \
  conn->event##_watcher.data = conn; \
  ev_io_start(conn->loop, &conn->event##_watcher);

#define unwatch(conn, event) \
  ev_io_stop(conn->backend->loop, &conn->event##_watcher);

#define get_ev_data(type, w, event) \
  (thin_##type##_t *) w->data; \
  assert(&((thin_##type##_t *)w->data)->event##_watcher == w);


VALUE thin_backend_init(VALUE self, VALUE address, VALUE port, VALUE app);
VALUE thin_backend_listen(VALUE self);
VALUE thin_backend_loop(VALUE self);
VALUE thin_backend_close(VALUE self);
VALUE thin_backend_alloc(VALUE self);

void thin_connection_init();
void thin_connection_start(thin_backend_t *backend, int fd, struct sockaddr_in remote_addr);
void thin_connections_create(thin_array_t *connections, size_t num);

void thin_parser_callbacks_init(VALUE module);
void thin_setup_parser_callbacks(thin_connection_t *connection);

#endif /* _THIN_H_ */