#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/socket.h>
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>

#include "ruby.h"
#include "backend.h"
#include "connection.h"
#include "parser.h"
#include "parser_callbacks.h"

static VALUE sInternedProcess;

void thin_connection_init()
{
  /* Intern some Ruby string */
  sInternedProcess = rb_intern("process");
}

void thin_start_connection(struct thin_backend *backend, int fd, struct sockaddr_in remote_addr)
{
  struct thin_connection *connection = NULL;
  int i = 0;
  
  /* select the first closed connection */
  while(connection == NULL || connection->open)
    connection = &backend->connections[i++];
  
  connection->backend = backend;
  connection->env = rb_hash_new();
  
  connection->fd = fd;
  connection->remote_addr = remote_addr;
  connection->open = 1;
  
  /* Initialize http_parser stuff */
  thin_setup_parser_callbacks(connection);
  
  /* Initialize libev stuff */
  ev_io_init(&connection->read_watcher, thin_connection_recv, connection->fd, EV_READ | EV_ERROR);
  connection->read_watcher.data = connection;
  ev_io_start(backend->loop, &connection->read_watcher);
}

void thin_connection_recv(EV_P_ struct ev_io *watcher, int revents)
{
  struct thin_connection *connection = watcher->data;
  assert(&connection->read_watcher == watcher);
  VALUE response = Qnil;
  
  connection->read_buffer.len = recv(connection->fd,
                                     connection->read_buffer.data,
                                     BUFFER_SIZE,
                                     0);
  
  /* mark the end of the string */
  connection->read_buffer.data[connection->read_buffer.len] = '\0';
  
  /* parse the damn thing to connection->env */
  http_parser_execute(&connection->parser,
                      connection->read_buffer.data,
                      connection->read_buffer.len,
                      connection->parser.nread);
  
  /* Call the backend to process the request */
  /* TODO use the returned response! */
  response = rb_funcall(connection->backend->rb_obj, sInternedProcess, 1, connection->env);
  
  /* start watching writable state */
  ev_io_init(&connection->write_watcher, thin_connection_send, connection->fd, EV_WRITE | EV_ERROR);
  connection->write_watcher.data = connection;
  ev_io_start(EV_A_ &connection->write_watcher);
}

void thin_connection_send(EV_P_ struct ev_io *watcher, int revents)
{
  if(EV_ERROR & revents) {
    perror("write event");
    return;
  }
  
  struct thin_connection *connection = watcher->data;
  assert(&connection->write_watcher == watcher);

  VALUE val = rb_str_new("PATH_INFO", 9);
  VALUE inspect = rb_hash_aref(connection->env, val);
  char *msg = RSTRING(inspect)->ptr;
  int len = RSTRING(inspect)->len;
  // char *msg = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 2\r\n\r\nhi";
  // int len = strlen(msg);
  
  if (send(connection->fd, msg, len, 0) < 0)
    perror("send");

  watcher->cb = thin_connection_close;
}

void thin_connection_close(EV_P_ struct ev_io *watcher, int revents)
{
  struct thin_connection *connection = watcher->data;  
  assert(&connection->write_watcher == watcher);
  
  ev_io_stop(EV_A_ &connection->read_watcher);
  ev_io_stop(EV_A_ &connection->write_watcher);
  shutdown(watcher->fd, SHUT_RDWR);
  close(watcher->fd);
  
  connection->open = 0;
}
