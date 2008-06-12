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

#include "ext_help.h"
#include "buffer.h"
#include "palloc.h"
#include "parser.h"
#include "queue.h"
#include "status.h"

#ifdef __FreeBSD__
#define LISTEN_BACKLOG     -1        /* FreeBSD set to max when negative */
#else
#define LISTEN_BACKLOG     511       /* that's what most web server use, ie: Apache & Nginx */
#endif

/* initialize number of connections in the pool, will grow */
#define CONNECTIONS_SIZE   100

/* when write buffer reach this size, it is sent right away, it controls the speed
 * at which a response is streamed. */
#define STREAM_SIZE        1024

#define MAX_HEADER         1024 * (80 + 32)
#define RACK_VERSION       INT2FIX(3), INT2FIX(0)


#define HEADER_SEP        ": "
#define RESP_HTTP_VERSION "HTTP/1.1 "
#define CRLF              "\x0d\x0a"

typedef struct backend_s backend_t;
typedef struct connection_s connection_t;

struct connection_s {
  /* socket */
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
  unsigned            finished : 1;
  struct {
    VALUE             obj;
    unsigned          active : 1;
  } thread;
  
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
  int                 fd;
  unsigned            open : 1;
  struct sockaddr_in  local_addr;
  
  /* debugging */
  unsigned            trace : 1;
  
  /* ruby */
  VALUE               obj; /* Ruby Backend object */
  VALUE               app; /* Rack app */
  
  /* connections */
  queue_t             connections;
  size_t              thread_count;
  
  /* libev */
  ev_tstamp           timeout;
  struct ev_loop     *loop;
  ev_io               accept_watcher;
  ev_idle             idle_watcher;
  ev_prepare          prepare_watcher;
};

/* libev helpers */
#define get_ev_data(type, w, event) \
  (type##_t *) w->data; \
  assert(&((type##_t *)w->data)->event##_watcher == w);

/* log helpers */
#define log_error(b, msg) \
  rb_funcall(b->obj, rb_intern("log_error"), 4, \
             rb_str_new2(msg), rb_str_new2(__FILE__), \
             rb_str_new2(__FUNCTION__), INT2FIX(__LINE__))

#define log_errno(b) log_error(b, strerror(errno))

#define trace(b, msg, len) if (b->trace) { write(2, msg, len); }

extern VALUE cStringIO;
extern VALUE sInternedCall;
extern VALUE sInternedKeys;
extern VALUE sRackInput;

/* backend */
void backend_define(void);

/* connection */
void connection_start(backend_t *backend, int fd, struct sockaddr_in remote_addr);
void connection_close(connection_t *connection);
/* error handling macros (so we can get line number and stuff) */
#define connection_error(c, msg) log_error((c)->backend, msg); connection_close(c)
#define connection_errno(c) connection_error(c, strerror(errno))

/* request & response */
void request_parse(connection_t *connection, char *buf, int len);
void response_process(connection_t *connection);

/* connections */
void connections_init(backend_t *backend);
void connections_push(backend_t *backend);
void connections_free(backend_t *backend);

/* parser */
void parser_callbacks_define();
void parser_callbacks_setup(connection_t *connection);

#endif /* _THIN_H_ */
