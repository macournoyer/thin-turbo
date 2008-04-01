#ifndef _THIN_H_
#define _THIN_H_

#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
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
#include "palloc.h"
#include "status.h"

#ifdef __FreeBSD__
#define THIN_LISTEN_BACKLOG    -1
#else
#define THIN_LISTEN_BACKLOG    511
#endif
#define THIN_CONNECTIONS_SIZE  300
#define THIN_MAX_HEADER_SLICES (80 + 32)
#define THIN_MAX_HEADER        1024 * THIN_MAX_HEADER_SLICES
#define THIN_BUFFER_SIZE       1024
#define THIN_BUFFER_SLICES     THIN_MAX_HEADER_SLICES /* buffer must be able to hold big header */

#define LF     (u_char) 10
#define CR     (u_char) 13
#define CRLF   "\x0d\x0a"

typedef struct thin_backend_s thin_backend_t;
typedef struct thin_buffer_s thin_buffer_t;
typedef struct thin_connection_s thin_connection_t;

struct thin_buffer_s {
  size_t  nalloc; /* num slices alloced */
  size_t  salloc; /* total size alloced */
  size_t  len;
  size_t  current;
  char   *ptr;
};

struct thin_connection_s {
  /* socket */
  unsigned            open : 1;
  int                 fd;
  struct sockaddr_in  remote_addr;
    
  /* request */
  thin_buffer_t       read_buffer;
  http_parser         parser;
  VALUE               env;
  size_t              content_length;
  
  /* response */
  thin_buffer_t       write_buffer;
  
  /* backend */
  thin_backend_t     *backend;
  pool_t             *buffer_pool;
  
  /* libev */
  struct ev_loop     *loop;
  ev_io               read_watcher;
  ev_io               write_watcher;  
};

struct thin_backend_s {
  /* socket */
  char               *address;
  unsigned            port;
  unsigned            open : 1;
  int                 fd;
  struct sockaddr_in  local_addr;
  
  /* ruby */
  VALUE               obj; /* Ruby Backend object */
  VALUE               app; /* Rack app */
  
  /* pools */
  array_t            *connections;
  pool_t             *buffer_pool;
  
  /* libev */
  ev_io               accept_watcher;
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

void thin_connection_start(thin_backend_t *backend, int fd, struct sockaddr_in remote_addr);
void thin_connection_parse(thin_connection_t *connection);
void thin_connection_process(thin_connection_t *connection);
void thin_connection_close(thin_connection_t *connection);

void thin_connections_init();
void thin_connections_create(array_t *connections, size_t num);

void thin_parser_callbacks_init(VALUE module);
void thin_setup_parser_callbacks(thin_connection_t *connection);

#endif /* _THIN_H_ */