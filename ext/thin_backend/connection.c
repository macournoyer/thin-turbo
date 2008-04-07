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
              (char *) c->write_buffer.ptr + c->write_buffer.current,
              c->write_buffer.len - c->write_buffer.current,
              0);
  
  if (sent > 0) {
    c->write_buffer.current += sent;
  } else {
    connection_errno(c);
    return;    
  }
  
  if (c->write_buffer.current == c->write_buffer.len) {
    watcher->cb = connection_closable_cb;
  }
}

static void connection_readable_cb(EV_P_ struct ev_io *watcher, int revents)
{
  connection_t *c = get_ev_data(connection, watcher, read);
  size_t        n;
  char          buf[BUFFER_SIZE];

  n = recv(c->fd, buf, BUFFER_SIZE, 0);
  
  if (n == -1) {
    connection_errno(c);
    return;
  }
  
  connection_parse(c, buf, n);
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
    /* FIXME: bug here on high concurrency, causes segfault on line 88 */
    c = &cs[++i];
  }
  
  assert(c != NULL);
  assert(!c->open);
  
  /* init connection */
  c->open           = 1;
  c->loop           = backend->loop;
  c->buffer_pool    = backend->buffer_pool;
  c->backend        = backend;
  c->content_length = 0;
  c->fd             = fd;
  c->remote_addr    = remote_addr;
  
  /* mark as used to Ruby GC */
  c->env = rb_hash_new();
  rb_gc_register_address(&c->env);
  
  /* alloc read buffer from pool */
  c->read_buffer.ptr     = palloc(c->buffer_pool, 1);
  assert(c->read_buffer.ptr);
  c->read_buffer.nalloc  = 1;
  c->read_buffer.salloc  = c->buffer_pool->size;
  c->read_buffer.len     = 0;
  c->read_buffer.current = 0;
  
  /* assign env[rack.input] */
  c->input = input_new(&c->read_buffer);
  rb_gc_register_address(&c->input);
  rb_hash_aset(c->env, sRackInput, c->input);
  
  /* alloc write buffer from pool */
  c->write_buffer.ptr     = palloc(c->buffer_pool, 1);
  assert(c->write_buffer.ptr);
  c->write_buffer.nalloc  = 1;
  c->write_buffer.salloc  = c->buffer_pool->size;
  c->write_buffer.len     = 0;
  c->write_buffer.current = 0;
  
  /* reinit parser */
  http_parser_init(&c->parser);
  c->parser.data = c;
  
  /* init libev stuff */
  watch(c, connection_readable_cb, read, EV_READ);
  
  /* TODO add timeout watcher */
}

void connection_parse(connection_t *c, char *buf, int len)
{
  if (!http_parser_is_finished(&c->parser) && c->read_buffer.len + len > MAX_HEADER) {
    connection_error(c, "Header too big");
    return;
  }
  
  /* alloc more mem when buffer full */
  /* TODO extract this into buffer.c and optimize */
  /* TODO store big body in tempfile */
  if (c->read_buffer.len >= c->read_buffer.salloc) {
    char *new, *old;
    
    /* TODO if last alloc, just alloc next block */
    old = c->read_buffer.ptr;
    new = (char *) palloc(c->buffer_pool, c->read_buffer.nalloc + 1);
    assert(new);
    
    memcpy(new, old, c->read_buffer.len);

    c->read_buffer.ptr     = new;
    c->read_buffer.nalloc ++;
    c->read_buffer.salloc += c->buffer_pool->size;
    pfree(c->buffer_pool, old);
  }
  
  memcpy(c->read_buffer.ptr + c->read_buffer.len, buf, len);
  c->read_buffer.len += len;
  
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
    connection_process(c);
  }
}

void connection_send_status(connection_t *c, const int status)
{
  size_t n;
  
  n = sprintf(c->write_buffer.ptr, "HTTP/1.1 %s" CRLF, get_status_line(status));
  
  c->write_buffer.len = n;
}

void connection_send_headers(connection_t *c, VALUE headers)
{
  VALUE   hash, keys, key, value;
  size_t  i, n;
  
  keys = rb_funcall(headers, sInternedKeys, 0);
  n = 0;
  
  for (i = 0; i < RARRAY_LEN(keys); ++i) {
    key = RARRAY_PTR(keys)[i];
    value = rb_hash_aref(headers, key);
    n += sprintf((char *) c->write_buffer.ptr + c->write_buffer.len +  n,
                 "%s: %s" CRLF,
                 RSTRING_PTR(key),
                 RSTRING_PTR(value));
  }
  c->write_buffer.len += n;
  
  memcpy(c->write_buffer.ptr + c->write_buffer.len, CRLF, 2);
  c->write_buffer.len += 2;
}

static VALUE iter_body(VALUE chunk, VALUE *val_conn)
{
  connection_t *c = (connection_t *) val_conn;
  size_t        len  = RSTRING_LEN(chunk);
  
  memcpy(c->write_buffer.ptr + c->write_buffer.len, RSTRING_PTR(chunk), len);
  c->write_buffer.len += len;
  
  return Qnil;
}

int connection_send_body(connection_t *c, VALUE body)
{
  if (TYPE(body) == T_STRING) {
    /* Calling String#each creates several other strings which is slower and use more mem,
     * also Ruby 1.9 doesn't define that method anymore, so it's better to send one big string. */
    size_t len = RSTRING_LEN(body);
    
    memcpy(c->write_buffer.ptr + c->write_buffer.len, RSTRING_PTR(body), len);
    c->write_buffer.len += len;
    
  } else {
    /* Iterate over body#each and send each yielded chunk */
    rb_iterate(rb_each, body, iter_body, (VALUE) c);
    
  }
}

void connection_process(connection_t *c)
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
    
    /* TODO grow buffer if too small */
    connection_send_status(c, status);
    connection_send_headers(c, headers);
    connection_send_body(c, body);
  
    watch(c, connection_writable_cb, write, EV_WRITE);
  }
}

void connection_close(connection_t *c)
{
  unwatch(c, read);
  unwatch(c, write);

  close(c->fd);
  
  if (c->read_buffer.ptr != NULL)
    pfree(c->buffer_pool, c->read_buffer.ptr);
  c->read_buffer.salloc = 0;
  c->read_buffer.nalloc = 0;
  
  if (c->write_buffer.ptr != NULL)
    pfree(c->buffer_pool, c->write_buffer.ptr);
  c->write_buffer.salloc = 0;
  c->write_buffer.nalloc = 0;
  
  /* tell Ruby GC vars are not used anymore */
  rb_gc_unregister_address(&c->env);
  rb_gc_unregister_address(&c->input);
  
  c->open = 0;
}


/* connections */

void connections_init()
{
  /* Intern some Ruby string */
  sInternedCall = rb_intern("call");
  sInternedKeys = rb_intern("keys");
  sRackInput = rb_obj_freeze(rb_str_new2("rack.input"));
}

void connections_create(array_t *connections, size_t num)
{
  connection_t *connection;
  int                i;
  
  for (i = 0; i <= num; ++i) {
    connection = array_push(connections);
    connection->open = 0;
    parser_callbacks_setup(connection);
  }
}
