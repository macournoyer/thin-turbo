#include "thin.h"

static VALUE sInternedCall;
static VALUE sInternedKeys;
static VALUE sRackInput;

#define thin_connection_error(connection, msg) \
  rb_funcall(connection->backend->obj, rb_intern("log_error"), 1, \
             rb_exc_new(rb_eRuntimeError, msg, strlen(msg))); \
  thin_connection_close(connection)

#define thin_connection_errorno(connection) \
  thin_connection_error(connection, strerror(errno))

/* event callbacks */

static void thin_connection_closable_cb(EV_P_ struct ev_io *watcher, int revents)
{
  thin_connection_t *connection = get_ev_data(connection, watcher, write);
  
  thin_connection_close(connection);
}

static void thin_connection_writable_cb(EV_P_ struct ev_io *watcher, int revents)
{
  thin_connection_t *connection = get_ev_data(connection, watcher, write);
  int                sent;
  
  sent = send(connection->fd,
              (char *) connection->write_buffer.ptr + connection->write_buffer.current,
              connection->write_buffer.len - connection->write_buffer.current,
              0);
  
  if (sent > 0) {
    connection->write_buffer.current += sent;
  } else {
    thin_connection_errorno(connection);
    return;    
  }
  
  if (connection->write_buffer.current == connection->write_buffer.len) {
    watcher->cb = thin_connection_closable_cb;
  }
}

static void thin_connection_readable_cb(EV_P_ struct ev_io *watcher, int revents)
{
  thin_connection_t *connection = get_ev_data(connection, watcher, read);
  size_t             n;
  char               buf[THIN_BUFFER_SIZE];

  n = recv(connection->fd, buf, THIN_BUFFER_SIZE, 0);
  
  if (n == -1) {
    thin_connection_errorno(connection);
    return;
  }
  
  thin_connection_parse(connection, buf, n);
}


/* public api */

void thin_connection_start(thin_backend_t *backend, int fd, struct sockaddr_in remote_addr)
{
  thin_connection_t *connection = NULL;
  thin_connection_t *connections = backend->connections->items;
  int                i = 0;
  
  /* select the first closed connection */
  for (i = 0; i < backend->connections->nitems; i++) {
    if (!connections[i].open) {
      connection = &connections[i];
      break;
    }
  }
  
  /* no free connection found, add more */
  if (connection == NULL) {
    thin_connections_create(backend->connections, THIN_CONNECTIONS_SIZE);
    connections = backend->connections->items;
    /* FIXME: bug here on high concurrency, causes segfault on line 88 */
    connection = &connections[++i];
  }
  
  assert(connection != NULL);
  assert(!connection->open);
  
  /* init connection */
  connection->open = 1;
  connection->loop = backend->loop;
  connection->buffer_pool = backend->buffer_pool;
  connection->backend = backend;
  connection->content_length = 0;
  connection->fd = fd;
  connection->remote_addr = remote_addr;
  
  /* mark as used to Ruby GC */
  connection->env = rb_hash_new();
  rb_gc_register_address(&connection->env);
  
  /* alloc read buffer from pool */
  connection->read_buffer.ptr = palloc(connection->buffer_pool, 1);
  if (connection->read_buffer.ptr == NULL)
    rb_sys_fail("palloc");
  connection->read_buffer.nalloc = 1;
  connection->read_buffer.salloc = connection->buffer_pool->size;
  connection->read_buffer.len = 0;
  connection->read_buffer.current = 0;
  
  /* assign env[rack.input] */
  connection->input = thin_input_new(&connection->read_buffer);
  rb_gc_register_address(&connection->input);
  rb_hash_aset(connection->env, sRackInput, connection->input);
  
  connection->write_buffer.ptr = palloc(connection->buffer_pool, 1);
  if (connection->write_buffer.ptr == NULL)
    rb_sys_fail("palloc");
  connection->write_buffer.nalloc = 1;
  connection->write_buffer.salloc = connection->buffer_pool->size;
  connection->write_buffer.len = 0;
  connection->write_buffer.current = 0;
  
  /* reinit parser */
  http_parser_init(&connection->parser);
  connection->parser.data = connection;
  
  /* init libev stuff */
  watch(connection, thin_connection_readable_cb, read, EV_READ);
  
  /* TODO add timeout watcher */
}

void thin_connection_parse(thin_connection_t *connection, char *buf, int len)
{
  if (!http_parser_is_finished(&connection->parser)
      && connection->read_buffer.len + len > THIN_MAX_HEADER) {
    thin_connection_error(connection, "Header too big");
    return;
  }
  
  /* alloc more mem when buffer full */
  /* TODO extract this into buffer.c and optimize */
  /* TODO store big body in tempfile */
  if (connection->read_buffer.len >= connection->read_buffer.salloc) {
    char *new, *old;
    
    /* TODO if last alloc, just alloc next block */
    old = connection->read_buffer.ptr;
    new = (char *) palloc(connection->buffer_pool,
                          connection->read_buffer.nalloc + 1);
    if (new == NULL)
      rb_sys_fail("palloc");
      
    memcpy(new, old, connection->read_buffer.len);

    connection->read_buffer.ptr = new;
    connection->read_buffer.nalloc ++;
    connection->read_buffer.salloc += connection->buffer_pool->size;
    pfree(connection->buffer_pool, old);
  }
  
  memcpy(connection->read_buffer.ptr + connection->read_buffer.len, buf, len);
  connection->read_buffer.len += len;
  
  if (!http_parser_is_finished(&connection->parser)) {
    /* header not all received, we continue parsing ... */
    
    /* terminate string with null (required by ragel v5) */
    memset(connection->read_buffer.ptr + connection->read_buffer.len, '\0', 1);
    
    /* parse the request into connection->env */
    connection->parser.nread = http_parser_execute(&connection->parser,
                                                    connection->read_buffer.ptr,
                                                    connection->read_buffer.len,
                                                    connection->parser.nread);
  
    /* parser error */
    if (http_parser_has_error(&connection->parser)) {
      thin_connection_error(connection, "Invalid request");
      return;
    }
  }
  
  /* request fully received */
  if (http_parser_is_finished(&connection->parser) && connection->read_buffer.len >= connection->content_length) {
    unwatch(connection, read);
    thin_connection_process(connection);
  }
}

void thin_connection_send_status(thin_connection_t *connection, const int status)
{
  size_t n;
  
  n = sprintf(connection->write_buffer.ptr, "HTTP/1.1 %s" CRLF, thin_status(status));
  
  connection->write_buffer.len = n;
}

void thin_connection_send_headers(thin_connection_t *connection, VALUE headers)
{
  VALUE   hash, keys, key, value;
  size_t  i, n;
  
  keys = rb_funcall(headers, sInternedKeys, 0);
  n = 0;
  
  for (i = 0; i < RARRAY_LEN(keys); ++i) {
    key = RARRAY_PTR(keys)[i];
    value = rb_hash_aref(headers, key);
    n += sprintf((char *) connection->write_buffer.ptr + connection->write_buffer.len +  n,
                 "%s: %s" CRLF,
                 RSTRING_PTR(key),
                 RSTRING_PTR(value));
  }

  connection->write_buffer.len += n;
  
  memcpy(connection->write_buffer.ptr + connection->write_buffer.len, CRLF, 2);
  connection->write_buffer.len += 2;
}

static VALUE iter_body(VALUE chunk, VALUE *val_conn)
{
  thin_connection_t *conn = (thin_connection_t *) val_conn;
  size_t             len  = RSTRING_LEN(chunk);
  
  memcpy(conn->write_buffer.ptr + conn->write_buffer.len, RSTRING_PTR(chunk), len);
  conn->write_buffer.len += len;
  
  return Qnil;
}

int thin_connection_send_body(thin_connection_t *conn, VALUE body)
{
  if (TYPE(body) == T_STRING) {
    /* Calling String#each creates several other strings which is slower and use more mem,
     * also Ruby 1.9 doesn't define that method anymore, so it's better to send one big string. */
    size_t len = RSTRING_LEN(body);
    
    memcpy(conn->write_buffer.ptr + conn->write_buffer.len, RSTRING_PTR(body), len);
    conn->write_buffer.len += len;
    
  } else {
    /* Iterate over body#each and send each yielded chunk */
    rb_iterate(rb_each, body, iter_body, (VALUE) conn);
    
  }
}

void thin_connection_process(thin_connection_t *connection)
{
  /* Call the app to process the request */
  VALUE response = rb_funcall_rescue(connection->backend->app, sInternedCall, 1, connection->env);

  if (response == Qundef) {
    /* log any error */
    rb_funcall(connection->backend->obj, rb_intern("log_error"), 0);
    thin_connection_close(connection);
  } else {
    /* store response info and prepare for writing */
    int   status  = FIX2INT(rb_ary_entry(response, 0));
    VALUE headers = rb_ary_entry(response, 1);
    VALUE body    = rb_ary_entry(response, 2);
    
    /* TODO grow buffer if too small */
    thin_connection_send_status(connection, status);
    thin_connection_send_headers(connection, headers);
    thin_connection_send_body(connection, body);
  
    watch(connection, thin_connection_writable_cb, write, EV_WRITE);
  }
}

void thin_connection_close(thin_connection_t *connection)
{
  unwatch(connection, read);
  unwatch(connection, write);

  close(connection->fd);
  
  if (connection->read_buffer.ptr != NULL)
    pfree(connection->buffer_pool, connection->read_buffer.ptr);
  connection->read_buffer.salloc = 0;
  connection->read_buffer.nalloc = 0;
  
  if (connection->write_buffer.ptr != NULL)
    pfree(connection->buffer_pool, connection->write_buffer.ptr);
  connection->write_buffer.salloc = 0;
  connection->write_buffer.nalloc = 0;
  
  /* tell Ruby GC vars are not used anymore */
  rb_gc_unregister_address(&connection->env);
  rb_gc_unregister_address(&connection->input);
  
  connection->open = 0;
}


/* connections */

void thin_connections_init()
{
  /* Intern some Ruby string */
  sInternedCall = rb_intern("call");
  sInternedKeys = rb_intern("keys");
  sRackInput = rb_obj_freeze(rb_str_new2("rack.input"));
}

void thin_connections_create(array_t *connections, size_t num)
{
  thin_connection_t *connection;
  int                i;
  
  for (i = 0; i <= num; ++i) {
    connection = array_push(connections);
    connection->open = 0;
    thin_setup_parser_callbacks(connection);
  }
}
