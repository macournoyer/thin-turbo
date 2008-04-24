#ifndef _THIN_H_
#define _THIN_H_

#include <ruby.h>
#include <ev.h>

/* TODO ifdef some of this? */
#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

/* includes for internal threading stuff */
#ifdef RUBY_19
  /* TODO find an alternative for Ruby 1.9 */
#else
# include "node.h"
#endif

#include "ext_help.h"
#include "array.h"
#include "buffer.h"
#include "palloc.h"
#include "parser.h"
#include "status.h"

#ifdef __FreeBSD__
#define LISTEN_BACKLOG     -1        /* FreeBSD set to max when negative */
#else
#define LISTEN_BACKLOG     511       /* that's what most web server use, ie: Apache & Nginx */
#endif
#define CONNECTIONS_SIZE   100              /* initialize size, will grow */
#define CONNECTION_TIMEOUT 30.0
#define BUFFER_SLICES      (80 + 32)        /* big enough so we can fit MAX_HEADER */
#define BUFFER_SIZE        1024             /* size of one chunk in the buffer pool */
#define MAX_BUFFER         1024             /* max size before transfered to tempfile */
#define MAX_HEADER         1024 * (80 + 32)
#define RACK_VERSION       INT2FIX(3), INT2FIX(0)


#define LF     (u_char) 10
#define CR     (u_char) 13
#define CRLF   "\x0d\x0a"

typedef struct backend_s backend_t;
typedef struct connection_s connection_t;

struct connection_s {
  /* socket */
  unsigned            open : 1;
  int                 fd;
  char               *remote_addr;
  
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
  
  /* libev */
  struct ev_loop     *loop;
  ev_io               read_watcher;
  ev_io               write_watcher;  
  ev_timer            timeout_watcher;
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
  
  /* libev */
  struct ev_loop     *loop;
  ev_io               accept_watcher;
  ev_idle             idle_watcher;
  ev_prepare          prepare_watcher;
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
VALUE connection_process(connection_t *connection);
void connection_close(connection_t *connection);

void connections_init();
void connections_create(array_t *connections, size_t num);

void parser_callbacks_init();
void parser_callbacks_setup(connection_t *connection);

#endif /* _THIN_H_ */
