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

static void connection_writable_cb(EV_P_ struct ev_io *watcher, int revents)
{
  connection_t *c = get_ev_data(connection, watcher, write);
  int           sent;
  
  if (c->write_buffer.len == 0)
    return;
  
  sent = send(c->fd,
              (char *) c->write_buffer.ptr + c->write_buffer.offset,
              c->write_buffer.len - c->write_buffer.offset,
              0);
  ev_timer_again(c->loop, &c->timeout_watcher);
  
  if (sent >= 0) {
    c->write_buffer.offset += sent;
    
  } else { /* sent < 0 => error */
    connection_errno(c);
    return;
    
  }
  
  if (buffer_eof(&c->write_buffer)) {
    /* if all the buffer is written we can clear it from memory */
    buffer_reset(&c->write_buffer);
    
    /* we can stream the request, so do do close it unless it's marked */
    if (c->finished)
      connection_close(c);
  }
}

static void connection_readable_cb(EV_P_ struct ev_io *watcher, int revents)
{
  connection_t *c = get_ev_data(connection, watcher, read);
  size_t        n;
  char          buf[BUFFER_CHUNK_SIZE];
  
  n = recv(c->fd, buf, BUFFER_CHUNK_SIZE, 0);
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
  c->finished       = 0;
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
  
  /* reinit parser */
  http_parser_init(&c->parser);
  c->parser.data = c;
  
  /* init libev stuff */
  c->timeout_watcher.data = c;
  watch(c, connection_readable_cb, read, EV_READ);
  /* timeout watcher, close connection when peer not responding */
  ev_timer_start(c->loop, &c->timeout_watcher);
}

static VALUE buffer_to_ruby_obj(buffer_t *buf)
{
  /* TODO move values to static */
  
  if (buffer_in_file(buf)) {
    /* close the fd and reopen in a Ruby File object */
    close(buf->file.fd);
    VALUE fname = rb_str_new2(buf->file.name);
    return rb_class_new_instance(1, &fname, rb_cFile);
    
  } else {
    /* no ref to StringIO, redefine to get ref */
    /* TODO if read_buffer empty use a generic empty StringIO ? */
    VALUE cStringIO = rb_define_class("StringIO", rb_cData);
    return rb_funcall(cStringIO, rb_intern("new"), 1, rb_str_new(buf->ptr, buf->len));
    
  }
}

void connection_parse(connection_t *c, char *buf, int len)
{
  if (!http_parser_is_finished(&c->parser) && c->read_buffer.len + len > MAX_HEADER) {
    connection_error(c, "Header too big");
    return;
  }
  
  if (buffer_append(&c->read_buffer, buf, len) < 0) {
    connection_error(c, "Error writing to buffer");
    return;
  }
  
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
    
    /* assign env[rack.input] */
    rb_hash_aset(c->env, sRackInput, buffer_to_ruby_obj(&c->read_buffer));
    
    /* call the Rack app in a Ruby green thread */
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
  
  /* FIXME very big response header w/ cause buffer to be stored in tmpfile */
  
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

static void connection_send_chunk(connection_t *c, const char *ptr, size_t len)
{
  /* chunk too big, split it in smaller chunks and send each separately */
  if (len >= BUFFER_MAX_LEN) {
    size_t i, size = BUFFER_MAX_LEN - 1, slices = len / size + 1;
    
    for (i = 0; i < slices; ++i) {
      if (i == slices - 1)
        size = len % size;
      
      connection_send_chunk(c, (char *) ptr + i * size, size);
    }
    
    return;
  }
  
  /* if appending will overflow the buffer we wait till more is sent */
  while (c->write_buffer.len + len > BUFFER_MAX_LEN)
    ev_loop(c->loop, EVLOOP_ONESHOT);
  
  buffer_append(&c->write_buffer, ptr, len);
  
  /* If we have a good sized chunk of data to send, try to send it right away.
   * This allows streaming by going for a shot in the even loop to drain the buffer if possisble,
      this way, the chunk is sent if the socket is writable.*/
  if (c->write_buffer.len - c->write_buffer.offset >= STREAM_SIZE) {
    ev_loop(c->loop, EVLOOP_ONESHOT | EVLOOP_NONBLOCK);    
  }
}

static VALUE iter_body(VALUE chunk, VALUE *val_conn)
{
  connection_t *c = (connection_t *) val_conn;
  
  connection_send_chunk(c, RSTRING_PTR(chunk), RSTRING_LEN(chunk));
  
  return Qnil;
}

int connection_send_body(connection_t *c, VALUE body)
{
  if (TYPE(body) == T_STRING && RSTRING_LEN(body) < BUFFER_MAX_LEN) {
    /* Calling String#each creates several other strings which is slower and use more mem,
     * also Ruby 1.9 doesn't define that method anymore, so it's better to send one big string. */    
    connection_send_chunk(c, RSTRING_PTR(body), RSTRING_LEN(body));
    
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
    rb_funcall(c->backend->obj, rb_intern("log_last_exception"), 0);
    connection_close(c);
    
  } else {
    /* store response info and prepare for writing */
    int   status  = FIX2INT(rb_ary_entry(response, 0));
    VALUE headers = rb_ary_entry(response, 1);
    VALUE body    = rb_ary_entry(response, 2);
    
    /* read buffer no longer needed, free up now so we
     * can reuse some of it for write buffer */
    buffer_reset(&c->read_buffer);
    
    watch(c, connection_writable_cb, write, EV_WRITE);
    
    connection_send_status(c, status);
    connection_send_headers(c, headers);
    connection_send_body(c, body);
    
    c->finished = 1;
    
    if (buffer_eof(&c->write_buffer))
      connection_close(c);
    
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
    
    ev_timer_init(&c->timeout_watcher, connection_timeout_cb, CONNECTION_TIMEOUT, CONNECTION_TIMEOUT);
  }
}
