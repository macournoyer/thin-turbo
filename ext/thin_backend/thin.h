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
#include "array.h"
#include "palloc.h"
#include "parser.h"
#include "status.h"

#ifdef __FreeBSD__
#define LISTEN_BACKLOG    -1
#else
#define LISTEN_BACKLOG    511
#endif
#define CONNECTIONS_SIZE  100
#define BUFFER_SLICES     (80 + 32) /* big enough so we can fit MAX_HEADER */
#define BUFFER_SIZE       1024

/* TODO move this to parser ... */
#define MAX_HEADER        1024 * (80 + 32)


#define LF     (u_char) 10
#define CR     (u_char) 13
#define CRLF   "\x0d\x0a"

typedef struct backend_s backend_t;
typedef struct buffer_s buffer_t;
typedef struct connection_s connection_t;

struct buffer_s {
  size_t  nalloc; /* num slices alloced */
  size_t  salloc; /* total size alloced */
  size_t  len;
  size_t  current;
  char   *ptr;
};

struct connection_s {
  /* socket */
  unsigned            open : 1;
  int                 fd;
  struct sockaddr_in  remote_addr;
    
  /* request */
  buffer_t            read_buffer;
  http_parser         parser;
  VALUE               env;
  VALUE               input;
  size_t              content_length;
  
  /* response */
  buffer_t            write_buffer;
  
  /* backend */
  backend_t          *backend;
  pool_t             *buffer_pool;
  
  /* libev */
  struct ev_loop     *loop;
  ev_io               read_watcher;
  ev_io               write_watcher;  
};

struct backend_s {
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
  (type##_t *) w->data; \
  assert(&((type##_t *)w->data)->event##_watcher == w);

#define log_error(b, msg) \
  rb_funcall(b->obj, rb_intern("log_error"), 1, \
             rb_exc_new(rb_eRuntimeError, msg, strlen(msg)))

#define log_errno(b) log_error(b, strerror(errno))

void backend_define(void);

void input_define(void);
VALUE input_new(buffer_t *buf);

void connection_start(backend_t *backend, int fd, struct sockaddr_in remote_addr);
void connection_parse(connection_t *connection, char *buf, int len);
void connection_process(connection_t *connection);
void connection_close(connection_t *connection);

void connections_init();
void connections_create(array_t *connections, size_t num);

void parser_callbacks_init();
void parser_callbacks_setup(connection_t *connection);

#endif /* _THIN_H_ */