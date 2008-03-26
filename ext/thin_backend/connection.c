#include "thin.h"

static VALUE sInternedCall;


/* event callbacks */

void thin_connection_closable_cb(EV_P_ struct ev_io *watcher, int revents)
{
  thin_connection_t *connection = get_ev_data(connection, watcher, write);
  
  thin_connection_close(connection);
}

int thin_print_headers(thin_connection_t *connection, char *str)
{
  VALUE  hash, keys, key, value;
  size_t i;
  int    n;
  
  hash = connection->headers;
  /* wth this does not work?
  keys = rb_hash_keys(hash); */
  keys = rb_funcall(hash, rb_intern("keys"), 0);
  n = 0;
  
  for (i = 0; i < RARRAY_LEN(keys); ++i) {
    key = RARRAY_PTR(keys)[i];
    value = rb_hash_aref(hash, key);
    n += sprintf((char *) str + n,
                 "%s: %s\r\n",
                 RSTRING_PTR(key),
                 RSTRING_PTR(value));
  }
  
  return n;
}

void thin_connection_writable_cb(EV_P_ struct ev_io *watcher, int revents)
{
  thin_connection_t *connection = get_ev_data(connection, watcher, write);
  char              *msg;
  int                n;
  
  /* TODO write in chunk */

  /* TODO get rid of that malloc or do something less stupid! */
  msg = (char *) malloc(connection->body.len + 1024);
  
  n  = sprintf(msg, "HTTP/1.1 %d OK\r\n", connection->status);
  n += thin_print_headers(connection, (char *) msg + n);
  n += sprintf((char *) msg + n, "\r\n%s", connection->body.ptr);
  
  if (send(connection->fd, msg, n, 0) < 0)
    rb_sys_fail("send"); 
  
  free(msg); /* see, doesn't make sense... */
  
  watcher->cb = thin_connection_closable_cb;
}

void thin_connection_readable_cb(EV_P_ struct ev_io *watcher, int revents)
{
  thin_connection_t *connection = get_ev_data(connection, watcher, read);
  VALUE              response;
  size_t             len;
  char              *new, *old;
  
  /* alloc more mem when buffer full */
  if (connection->read_buffer.len == connection->read_buffer.nalloc) {
    /* TODO refactor this into a buffer.c file? */
    old = connection->read_buffer.ptr;
    new = (char *) malloc(connection->read_buffer.nalloc * 2);
    if (new == NULL)
      rb_sys_fail("malloc");
    
    connection->read_buffer.ptr = new;
    connection->read_buffer.nalloc *= 2;
    free(old);
  }

  len = recv(connection->fd,
             connection->read_buffer.ptr + connection->read_buffer.len,
             THIN_BUFFER_SIZE - connection->read_buffer.len,
             0);
  
  connection->read_buffer.len += len;
  
  /* terminate string with null */
  memset(connection->read_buffer.ptr + connection->read_buffer.len, '\0', 1);
  
  /* parse the request into connection->env */
  len = http_parser_execute(&connection->parser,
                            connection->read_buffer.ptr,
                            connection->read_buffer.len,
                            connection->parser.nread);
  
  connection->parser.nread = len;

  if (http_parser_is_finished(&connection->parser) &&
      connection->read_buffer.len >= connection->content_length) {
    
    /* Call the app to process the request */
    response = rb_funcall(connection->backend->app, sInternedCall, 1, connection->env);
  
    connection->status   = FIX2INT(rb_ary_entry(response, 0));
    connection->headers  = rb_ary_entry(response, 1);
    connection->body.ptr = RSTRING_PTR(rb_ary_entry(response, 2));
    connection->body.len = RSTRING_LEN(rb_ary_entry(response, 2));
  
    unwatch(connection, read);
    watch(connection, thin_connection_writable_cb, write, EV_WRITE);    
  }
}


/* public api */

void thin_connection_init()
{
  /* Intern some Ruby string */
  sInternedCall = rb_intern("call");
}

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
    connection = &connections[++i];
  }
  
  assert(connection != NULL);
  assert(!connection->open);
  
  /* init connection */
  connection->loop = backend->loop;
  connection->buffer_pool = backend->buffer_pool;
  connection->backend = backend;
  connection->env = rb_hash_new();
  connection->content_length = 0;
  connection->fd = fd;
  connection->remote_addr = remote_addr;
  connection->open = 1;
  
  /* alloc read buffer from pool */
  connection->read_buffer.ptr = palloc(connection->buffer_pool, 1);
  if (connection->read_buffer.ptr == NULL)
    rb_sys_fail("palloc");
  connection->read_buffer.nalloc = 1;
  connection->read_buffer.len = 0;
  
  /* reinit parser */
  http_parser_init(&connection->parser);
  connection->parser.data = connection;
  
  /* init libev stuff */
  watch(connection, thin_connection_readable_cb, read, EV_READ);
}

void thin_connections_create(array_t *connections, size_t num)
{
  thin_connection_t *connection;
  int                i;
  
  for (i = 0; i <= num; ++i) {
    connection = array_push(connections);
    
    thin_setup_parser_callbacks(connection);
  }
}

void thin_connection_close(thin_connection_t *connection)
{
  unwatch(connection, read);
  unwatch(connection, write);

  close(connection->fd);
  
  if (connection->read_buffer.ptr != NULL)
    pfree(connection->buffer_pool, connection->read_buffer.ptr);
  
  connection->open = 0; 
}