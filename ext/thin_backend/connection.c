/* Functions to handle a connection to a client.
 * backend.c calls connection_start when a new connection arrives.
 * connection.c takes care of reading data to c->read_buffer
 * and writing data from c->write_buffer.
 */
#include "thin.h"

/* event callbacks */

static void connection_writable_cb(EV_P_ struct ev_io *watcher, int revents)
{
  connection_t *c = get_ev_data(connection, watcher, write);
  int           sent;
  
  if (EV_ERROR & revents) {
    connection_error(c, "Error writing on connection socket");
    return;
  }
  
  if (c->write_buffer.len == 0)
    return;
  
  sent = send(c->fd,
              (char *) c->write_buffer.ptr + c->write_buffer.offset,
              c->write_buffer.len - c->write_buffer.offset,
              0);
  
  ev_timer_again(c->loop, &c->timeout_watcher);
  
  if (sent < 0) {
    /* error, closing connection */
    connection_errno(c);
    return;
  }
  
  c->write_buffer.offset += sent;
  
  if (buffer_eof(&c->write_buffer)) {
    /* if all the buffer is written we can clear it from memory */
    buffer_reset(&c->write_buffer);
    
    /* we can stream the request, so do do close it unless it's marked */
    if (c->finished)
      connection_close(c);
  }
}

void connection_watch_writable(connection_t *c)
{
  ev_io_start(c->loop, &c->write_watcher);
  ev_timer_start(c->loop, &c->timeout_watcher);
}

static void connection_readable_cb(EV_P_ struct ev_io *watcher, int revents)
{
  connection_t *c = get_ev_data(connection, watcher, read);
  size_t        n;
  char          buf[BUFFER_CHUNK_SIZE];
  
  if (EV_ERROR & revents) {
    connection_error(c, "Error reading on connection socket");
    return;
  }
  
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
  
  request_parse(c, buf, n);
}

static void connection_timeout_cb(EV_P_ struct ev_timer *watcher, int revents)
{
  connection_t *c = get_ev_data(connection, watcher, timeout);
    
  connection_close(c);
}


/* public api */

void connection_start(backend_t *backend, int fd, struct sockaddr_in remote_addr)
{
  connection_t *c = (connection_t *) queue_pop(&backend->connections);
  
  /* no free connection found, add more */
  if (c == NULL) {
    connections_push(backend);
    c = (connection_t *) queue_pop(&backend->connections);
  }
  
  assert(c != NULL);
  
  /* init connection */
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
  c->read_watcher.data    = c;
  c->write_watcher.data   = c;
  c->timeout_watcher.data = c;
  ev_io_init(&c->read_watcher, connection_readable_cb, c->fd, EV_READ | EV_ERROR);
  ev_io_init(&c->write_watcher, connection_writable_cb, c->fd, EV_WRITE | EV_ERROR);
  ev_timer_init(&c->timeout_watcher, connection_timeout_cb, backend->timeout, backend->timeout);
  
  /* start event watchers */
  ev_timer_start(c->loop, &c->timeout_watcher);
  ev_io_start(c->loop, &c->read_watcher);
}

void connection_close(connection_t *c)
{
  ev_io_stop(c->loop, &c->read_watcher);
  ev_io_stop(c->loop, &c->write_watcher);
  ev_timer_stop(c->loop, &c->timeout_watcher);

  close(c->fd);
  
  buffer_reset(&c->read_buffer);
  buffer_reset(&c->write_buffer);
  
  /* tell Ruby GC vars are not used anymore */
  rb_gc_unregister_address(&c->env);
  rb_gc_unregister_address(&c->input);
  
  /* TODO maybe kill the thread also: rb_thread_kill(c->thread) */
  
  /* put back in the queue of unused connections */
  queue_push(&c->backend->connections, c);
}


/* connections */

void connections_init(backend_t *backend)
{
  queue_init(&backend->connections);
  connections_push(backend);
}

void connections_push(backend_t *backend)
{
  size_t        i;
  connection_t *c;
  
  for (i = 0; i < CONNECTIONS_SIZE; ++i) {
    c = (connection_t *) malloc(sizeof(connection_t));
    assert(c);
    
    parser_callbacks_setup(c);
    
    buffer_init(&c->read_buffer);
    buffer_init(&c->write_buffer);
    
    queue_push(&backend->connections, c);
  }
}

void connections_free(backend_t *backend)
{
  connection_t *c = queue_pop(&backend->connections);
  
  while (c != NULL) {
    free(c);
    c = queue_pop(&backend->connections);
  }
}
