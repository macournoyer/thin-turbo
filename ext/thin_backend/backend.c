#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <assert.h>
#include <arpa/inet.h>

#include <ev.h>

#include "ruby.h"
#include "ext_help.h"
#include "backend.h"
#include "connection.h"

void thin_backend_init(VALUE self, VALUE address, VALUE port)
{
  struct thin_backend * backend = NULL;
  DATA_GET(self, thin_backend, backend);
  
  backend->address = RSTRING_PTR(address);
  backend->port = FIX2INT(port);
  
  backend->local_addr.sin_family = AF_INET;
  backend->local_addr.sin_port = htons(backend->port);
  backend->local_addr.sin_addr.s_addr = inet_addr(backend->address);
}

void thin_backend_start(VALUE self)
{
  BACKEND_GET(backend);
  int sock_flags = 1;

  if ((backend->fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    perror("socket");

  // Set socket as beeing nonblocking
  assert(fcntl(backend->fd, F_SETFL, O_NONBLOCK) >= 0);

  if (setsockopt(server.fd, SOL_SOCKET, SO_REUSEADDR, &sock_flags, sizeof(int)) == -1)
    perror("setsockopt");
  
  if (bind(backend->fd, (struct sockaddr *)&backend->local_addr, sizeof backend->local_addr) == -1)
    perror("bind");

  if (listen(backend->fd, LISTEN_BACKLOG) == -1)
    perror("listen");
    
  backend->loop = ev_default_loop(0);
  
  /* initialise an io watcher, then start it */
  ev_io_init(&backend->accept_watcher, thin_server_accept, backend->fd, EV_READ | EV_ERROR);
  backend->accept_watcher.data = backend;
  ev_io_start(backend->loop, &backend->accept_watcher);
  
  /* loop till timeout or data ready */
  ev_loop(backend->loop, 0);
}

void thin_backend_accept(EV_P_ struct ev_io *watcher, int revents)
{
  struct thin_backend *server = watcher->data;
  struct sockaddr_in remote_addr;
  socklen_t sin_size = sizeof remote_addr;
  int fd;
  
  if ((fd = accept(server->fd, (struct sockaddr *)&remote_addr, &sin_size)) == -1)
    perror("accept");
  
  int flags = fcntl(fd, F_GETFL, 0);
  assert(0 <= fcntl(fd, F_SETFL, flags | O_NONBLOCK));
  
  thin_start_connection(server, fd, remote_addr);
}

void thin_backend_free(thin_backend *backend)
{
  if (backend)
    free(backend);
}

VALUE thin_backend_alloc(VALUE self)
{
  VALUE backend = Qnil;
  
  thin_backend *pbackend = ALLOC_N(thin_backend, 1);
  backend = Data_Wrap_Struct(cBackend, 0, thin_backend_free, pbackend);
  
  return backend;
}
