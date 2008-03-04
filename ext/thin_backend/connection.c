#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/socket.h>
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>

#include "ruby.h"
#include "connection.h"
#include "parser_callbacks.c"

void thin_start_connection(struct thin_backend *server, int fd, struct sockaddr_in remote_addr)
{
  /* TODO manage a pool of reusable connections */
  struct thin_connection *connection = malloc(sizeof(struct thin_connection));
  connection->open = 0;
  connection->read_buffer.data = malloc(1024);
  connection->read_buffer.len = 1024;
  
  connection->env = rb_hash_new();
  
  connection->fd = fd;
  connection->remote_addr = remote_addr;
  connection->open = 1;
  
  /* Initialize http_parser stuff */
  http_parser_init(&(connection->parser));
  connection->parser.data = connection;
  connection->parser.http_field     = http_field_cb;
  connection->parser.request_method = request_method_cb;
  connection->parser.request_uri    = request_uri_cb;
  connection->parser.fragment       = fragment_cb;
  connection->parser.request_path   = request_path_cb;
  connection->parser.query_string   = query_string_cb;
  connection->parser.http_version   = http_version_cb;
  connection->parser.content_length = content_length_cb;
  
  /* Initialize libev stuff */
  ev_io_init(&connection->read_watcher, thin_connection_recv, connection->fd, EV_READ | EV_ERROR);
  connection->read_watcher.data = connection;
  ev_io_start(server->loop, &connection->read_watcher);
}

void thin_connection_recv(EV_P_ struct ev_io *watcher, int revents)
{
  struct thin_connection *connection = watcher->data;
  assert(&connection->read_watcher == watcher);
  
  connection->read_buffer.len = recv(connection->fd,
                                     connection->read_buffer.data,
                                     connection->read_buffer.len,
                                     0);
  
  /* mark the end of the string */
  memset(connection->read_buffer.data + connection->read_buffer.len, '\0', 1);
  // printf("%s\n", connection->read_buffer.data);
  
  http_parser_execute(&connection->parser,
                      connection->read_buffer.data,
                      connection->read_buffer.len,
                      connection->parser.nread);
  
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
  
  char *msg = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 2\r\n\r\nhi";
  struct thin_connection *connection = watcher->data;
  assert(&connection->write_watcher == watcher);

  if (send(connection->fd, msg, strlen(msg), 0) < 0)
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
