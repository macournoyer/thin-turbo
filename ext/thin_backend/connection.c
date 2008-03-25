#include "thin.h"

static VALUE sInternedCall;


/* event callbacks */

void thin_connection_closable_cb(EV_P_ struct ev_io *watcher, int revents)
{
  thin_connection_t *connection = get_ev_data(connection, watcher, write);
  
  unwatch(connection, read);
  unwatch(connection, write);

  close(connection->fd);
  connection->open = 0;
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

  /* get rid of that malloc */
  msg = (char *) malloc(connection->body.len + 1024);
  
  n  = sprintf(msg, "HTTP/1.1 %d OK\r\n", connection->status);
  n += thin_print_headers(connection, (char *) msg + n);
  n += sprintf((char *) msg + n, "\r\n%s", connection->body.ptr);
  
  if (send(connection->fd, msg, n, 0) < 0)
    rb_sys_fail("send"); 

  watcher->cb = thin_connection_closable_cb;
}

void thin_connection_readable_cb(EV_P_ struct ev_io *watcher, int revents)
{
  thin_connection_t *connection = get_ev_data(connection, watcher, read);
  VALUE              response;
  size_t             len;
  char              *new, *old;
  
  /* TODO alloc more mem when buffer full */

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
  connection->backend = backend;
  connection->env = rb_hash_new();
  connection->content_length = 0;
  connection->fd = fd;
  connection->remote_addr = remote_addr;
  connection->open = 1;
  
  /* alloc buffer only once */
  if (connection->read_buffer.ptr == NULL) {
    connection->read_buffer.ptr = (char *) malloc(THIN_BUFFER_SIZE);
    if (connection->read_buffer.ptr == NULL)
      rb_sys_fail("malloc");
  }
  connection->read_buffer.len = 0;
  
  /* reinit parser */
  http_parser_init(&connection->parser);
  connection->parser.data = connection;
  
  /* init libev stuff */
  watch(connection, thin_connection_readable_cb, read, EV_READ);
}

void thin_connections_create(thin_array_t *connections, size_t num)
{
  thin_connection_t *connection;
  int                i;
  
  for (i = 0; i <= num; ++i) {
    connection = thin_array_push(connections);
    
    connection->read_buffer.ptr = NULL;
    thin_setup_parser_callbacks(connection);
  }
}
