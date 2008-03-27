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
  char              *resp;
  int                n, sent;
  
  /* TODO write in chunk */

  resp = (char *) palloc(connection->buffer_pool,
                         connection->body.len / connection->buffer_pool->size + 1);
  
  n  = sprintf(resp, "HTTP/1.1 %d OK\r\n", connection->status);
  n += thin_print_headers(connection, (char *) resp + n);
  n += sprintf((char *) resp + n, "\r\n%s", connection->body.ptr);
  
  sent = send(connection->fd, resp, n, 0);
  /* TODO do something w/ sent, maybe buffer, not all sent? */
  
  pfree(connection->buffer_pool, resp);
  watcher->cb = thin_connection_closable_cb;
}

void thin_connection_readable_cb(EV_P_ struct ev_io *watcher, int revents)
{
  thin_connection_t *connection = get_ev_data(connection, watcher, read);
  VALUE              response;
  size_t             len;
  char              *new, *old;
  
  /* alloc more mem when buffer full */
  if (connection->read_buffer.len >= connection->read_buffer.salloc) {
    old = connection->read_buffer.ptr;
    new = (char *) palloc(connection->buffer_pool,
                          connection->read_buffer.nalloc * 2);
    if (new == NULL)
      rb_sys_fail("palloc");
    
    connection->read_buffer.ptr = new;
    connection->read_buffer.nalloc *= 2;
    connection->read_buffer.salloc *= 2;
    pfree(connection->buffer_pool, old);
  }

  len = recv(connection->fd,
             connection->read_buffer.ptr + connection->read_buffer.len,
             connection->read_buffer.salloc - connection->read_buffer.len,
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
    
    unwatch(connection, read);
    
    /* Call the app to process the request */
    response = rb_funcall_rescue(connection->backend->app, sInternedCall, 1, connection->env);
    
    if (response == Qundef) {
      /* log any error */
      rb_funcall(connection->backend->obj, rb_intern("log_error"), 0);
      thin_connection_close(connection);
    } else {
      /* store response info and prepare for writing */
      connection->status   = FIX2INT(rb_ary_entry(response, 0));
      connection->headers  = rb_ary_entry(response, 1);
      connection->rbbody   = rb_ary_entry(response, 2);
      /* FIXME use body#each to output */
      connection->body.ptr = RSTRING_PTR(connection->rbbody);
      connection->body.len = RSTRING_LEN(connection->rbbody);
      
      /* mark as used to Ruby GC */
      rb_gc_register_address(&connection->headers);
      rb_gc_register_address(&connection->rbbody);
      
      watch(connection, thin_connection_writable_cb, write, EV_WRITE);
    }
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
  connection->read_buffer.salloc = 0;
  connection->read_buffer.nalloc = 0;
  
  /* tell Ruby GC vars are not used anymore */
  rb_gc_unregister_address(&connection->headers);
  rb_gc_unregister_address(&connection->rbbody);
  rb_gc_unregister_address(&connection->env);
  
  connection->open = 0; 
}