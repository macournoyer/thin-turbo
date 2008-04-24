#include "thin.h"

static VALUE sInternedCall;
static VALUE sInternedKeys;
static VALUE sRackInput;

#define connection_error(c, msg) \
  log_error(c->backend, msg); \
  connection_close(c)

#define connection_errno(c) \
  connection_error(c, strerror(errno))

/* event callbacks */

static void connection_closable_cb(EV_P_ struct ev_io *watcher, int revents)
{
  connection_t *c = get_ev_data(connection, watcher, write);
  
  connection_close(c);
}

static void connection_writable_cb(EV_P_ struct ev_io *watcher, int revents)
{
  connection_t *c = get_ev_data(connection, watcher, write);
  int           sent;
  
  sent = send(c->fd,
              (char *) c->write_buffer.ptr + c->write_buffer.offset,
              c->write_buffer.len - c->write_buffer.offset,
              0);
  ev_timer_again(c->loop, &c->timeout_watcher);
  
  if (sent > 0) {
    c->write_buffer.offset += sent;
  } else {
    connection_errno(c);
    return;
  }
  
  if (c->write_buffer.offset == c->write_buffer.len) {
    watcher->cb = connection_closable_cb;
  }
}

static void connection_readable_cb(EV_P_ struct ev_io *watcher, int revents)
{
  connection_t *c = get_ev_data(connection, watcher, read);
  size_t        n;
  char          buf[BUFFER_SIZE];
  
  n = recv(c->fd, buf, BUFFER_SIZE, 0);
  ev_timer_again(c->loop, &c->timeout_watcher);
  
  if (n == -1) {
    /* error, closing connection */
    connection_errno(c);
    return;
  }
  
  if (n == 0) {
    /* received 0 byte, read again next loop */
    return;
  }
  
  connection_parse(c, buf, n);
}

static void connection_timeout_cb(EV_P_ struct ev_timer *watcher, int revents)
{
  connection_t *c = get_ev_data(connection, watcher, timeout);
    
  connection_close(c);
}



/* public api */

void connection_start(backend_t *backend, int fd, struct sockaddr_in remote_addr)
{
  connection_t *c  = NULL;
  connection_t *cs = backend->connections->items;
  int           i  = 0;
  
  /* select the first closed connection */
  for (i = 0; i < backend->connections->nitems; i++) {
    if (!cs[i].open) {
      c = &cs[i];
      break;
    }
  }
  
  /* no free connection found, add more */
  if (c == NULL) {
    connections_create(backend->connections, CONNECTIONS_SIZE);
    cs = backend->connections->items;
    /* FIXME: bug here on high concurrency, causes segfault in libev code */
    c = &cs[++i];
  }
  
  assert(c != NULL);
  assert(!c->open);
  
  /* init connection */
  c->open           = 1;
  c->loop           = backend->loop;
  c->backend        = backend;
  c->content_length = 0;
  c->fd             = fd;
  c->remote_addr    = inet_ntoa(remote_addr.sin_addr);
  
  /* mark as used to Ruby GC */
  c->env = rb_hash_new();
  rb_gc_register_address(&c->env);
  
  /* reset buffers */
  buffer_reset(&c->read_buffer);
  buffer_reset(&c->write_buffer);
  
  /* assign env[rack.input] */
  c->input = input_new(&c->read_buffer);
  rb_gc_register_address(&c->input);
  rb_hash_aset(c->env, sRackInput, c->input);
  
  /* reinit parser */
  http_parser_init(&c->parser);
  c->parser.data = c;
  
  /* init libev stuff */
  watch(c, connection_readable_cb, read, EV_READ);
  /* timeout watcher, close connection when peer not responding */
  ev_timer_start(c->loop, &c->timeout_watcher);
}

void connection_parse(connection_t *c, char *buf, int len)
{
  if (!http_parser_is_finished(&c->parser) && c->read_buffer.len + len > MAX_HEADER) {
    connection_error(c, "Header too big");
    return;
  }
  
  buffer_append(&c->read_buffer, buf, len);
  
  if (!http_parser_is_finished(&c->parser)) {
    /* header not all received, we continue parsing ... */
    
    /* terminate string with null (required by ragel v5) */
    memset(c->read_buffer.ptr + c->read_buffer.len, '\0', 1);
    
    /* parse the request into connection->env */
    c->parser.nread = http_parser_execute(&c->parser,
                                           c->read_buffer.ptr,
                                           c->read_buffer.len,
                                           c->parser.nread);
  
    /* parser error */
    if (http_parser_has_error(&c->parser)) {
      connection_error(c, "Invalid request");
      return;
    }
  }
  
  /* request fully received */
  if (http_parser_is_finished(&c->parser) && c->read_buffer.len >= c->content_length) {
    unwatch(c, read);
    rb_thread_create(connection_process, (void*) c);
  }
}

void connection_send_status(connection_t *c, const int status)
{
  buffer_t *buf = &c->write_buffer;
  char     *status_line = get_status_line(status);
  #define   RESP_HTTP_VERSION "HTTP/1.1 "
  
  buffer_append(buf, RESP_HTTP_VERSION, sizeof(RESP_HTTP_VERSION) - 1);
  buffer_append(buf, status_line, strlen(status_line));
  buffer_append(buf, CRLF, sizeof(CRLF) - 1);
}

void connection_send_headers(connection_t *c, VALUE headers)
{
  buffer_t *buf = &c->write_buffer;
  VALUE     hash, keys, key, value;
  size_t    i, n = 0;
  #define   HEADER_SEP ": "
  
  keys = rb_funcall(headers, sInternedKeys, 0);
  
  for (i = 0; i < RARRAY_LEN(keys); ++i) {
    key = RARRAY_PTR(keys)[i];
    value = rb_hash_aref(headers, key);

    buffer_append(buf, RSTRING_PTR(key), RSTRING_LEN(key));
    buffer_append(buf, HEADER_SEP, sizeof(HEADER_SEP) - 1);
    buffer_append(buf, RSTRING_PTR(value), RSTRING_LEN(value));
    buffer_append(buf, CRLF, sizeof(CRLF) - 1);
  }
  
  buffer_append(buf, CRLF, sizeof(CRLF) - 1);
}

static VALUE iter_body(VALUE chunk, VALUE *val_conn)
{
  connection_t *c = (connection_t *) val_conn;
  
  buffer_append(&c->write_buffer, RSTRING_PTR(chunk), RSTRING_LEN(chunk));
  
  return Qnil;
}

int connection_send_body(connection_t *c, VALUE body)
{
  if (TYPE(body) == T_STRING) {
    /* Calling String#each creates several other strings which is slower and use more mem,
     * also Ruby 1.9 doesn't define that method anymore, so it's better to send one big string. */    
    buffer_append(&c->write_buffer, RSTRING_PTR(body), RSTRING_LEN(body));
    
  } else {
    /* Iterate over body#each and send each yielded chunk */
    rb_iterate(rb_each, body, iter_body, (VALUE) c);
    
  }
}

VALUE connection_process(connection_t *c)
{
  /* Call the app to process the request */
  VALUE response = rb_funcall_rescue(c->backend->app, sInternedCall, 1, c->env);

  if (response == Qundef) {
    /* log any error */
    rb_funcall(c->backend->obj, rb_intern("log_error"), 0);
    connection_close(c);
  } else {
    /* store response info and prepare for writing */
    int   status  = FIX2INT(rb_ary_entry(response, 0));
    VALUE headers = rb_ary_entry(response, 1);
    VALUE body    = rb_ary_entry(response, 2);
    
    connection_send_status(c, status);
    connection_send_headers(c, headers);
    connection_send_body(c, body);
  
    watch(c, connection_writable_cb, write, EV_WRITE);
  }
  
  return Qnil;
}

void connection_close(connection_t *c)
{
  unwatch(c, read);
  unwatch(c, write);
  ev_timer_stop(c->loop, &c->timeout_watcher);

  close(c->fd);
  
  buffer_reset(&c->read_buffer);
  buffer_reset(&c->write_buffer);
  
  /* tell Ruby GC vars are not used anymore */
  rb_gc_unregister_address(&c->env);
  rb_gc_unregister_address(&c->input);
  
  /* TODO maybe kill the thread also: rb_thread_kill(c->thread) */
  
  c->open = 0;
}


/* connections */

void connections_init()
{
  /* Intern some Ruby string */
  sInternedCall = rb_intern("call");
  sInternedKeys = rb_intern("keys");
  
  sRackInput    = rb_obj_freeze(rb_str_new2("rack.input"));
  rb_gc_register_address(&sRackInput);
}

void connections_create(array_t *connections, size_t num)
{
  connection_t *c;
  int           i;
  
  for (i = 0; i <= num; ++i) {
    c = array_push(connections);
    assert(c);
    
    c->open = 0;
    parser_callbacks_setup(c);
    
    buffer_init(&c->read_buffer);
    buffer_init(&c->write_buffer);
    
    c->timeout_watcher.data = c;
    ev_timer_init(&c->timeout_watcher, connection_timeout_cb, CONNECTION_TIMEOUT, CONNECTION_TIMEOUT);
  }
}
