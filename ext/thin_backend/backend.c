#define EV_STANDALONE 1
#include <ev.c>

#include "thin.h"


/* event callbacks */

static void thin_backend_accept_cb(EV_P_ struct ev_io *watcher, int revents)
{
  thin_backend_t    *server = get_ev_data(backend, watcher, accept);
  struct sockaddr_in remote_addr;
  socklen_t          sin_size = sizeof(remote_addr);
  int                fd, flags;
  
  fd = accept(server->fd, (struct sockaddr *)&remote_addr, &sin_size);
  if (fd == -1)
    rb_sys_fail("accept");
  
  flags = fcntl(fd, F_GETFL, 0);
  if ((fcntl(fd, F_SETFL, flags | O_NONBLOCK)) < 0)
    rb_sys_fail("fcntl");
  
  thin_connection_start(server, fd, remote_addr);
}


/* public api */

VALUE thin_backend_init(VALUE self, VALUE address, VALUE port, VALUE app)
{
  thin_backend_t *backend = NULL;
  DATA_GET(self, thin_backend_t, backend);
    
  backend->address = RSTRING_PTR(address);
  backend->port = FIX2INT(port);
  backend->app = app;
  
  backend->local_addr.sin_family = AF_INET;
  backend->local_addr.sin_port = htons(backend->port);
  backend->local_addr.sin_addr.s_addr = inet_addr(backend->address);
  
  backend->loop = ev_default_loop(0);
  
  return self;
}

VALUE thin_backend_listen(VALUE self)
{
  thin_backend_t *backend = NULL;
  DATA_GET(self, thin_backend_t, backend);
  int sock_flags;

  if ((backend->fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    rb_sys_fail("socket");

  /* set socket as beeing nonblocking */
  if (fcntl(backend->fd, F_SETFL, O_NONBLOCK) < 0)
    rb_sys_fail("fcntl");

  sock_flags = 1;
  if (setsockopt(backend->fd, SOL_SOCKET, SO_REUSEADDR, &sock_flags, sizeof(sock_flags)) == -1)
    rb_sys_fail("setsockopt(SO_REUSEADDR)");
    
  if (bind(backend->fd, (struct sockaddr *)&backend->local_addr, sizeof backend->local_addr) == -1)
    rb_sys_fail("bind");

  if (listen(backend->fd, THIN_LISTEN_BACKLOG) == -1)
    rb_sys_fail("listen");
  
  /* initialise io watchers */
  watch(backend, thin_backend_accept_cb, accept, EV_READ);  
  
  backend->open = 1;
  
  return Qtrue;
}

VALUE thin_backend_loop(VALUE self)
{
  thin_backend_t *backend = NULL;
  DATA_GET(self, thin_backend_t, backend);
  
  ev_loop(backend->loop, EVLOOP_ONESHOT);
  
  return Qtrue;
}

VALUE thin_backend_close(VALUE self)
{
  thin_backend_t *backend = NULL;
  DATA_GET(self, thin_backend_t, backend);
  
  backend->open = 0;
  ev_io_stop(backend->loop, &backend->accept_watcher);
  close(backend->fd);
  
  return Qtrue;
}

static void thin_backend_free(thin_backend_t *backend)
{
  if (backend) {
    array_destroy(backend->connections);
    pool_destroy(backend->buffer_pool);
    free(backend);
  }
}

VALUE thin_backend_alloc(VALUE klass)
{
  thin_backend_t *backend = ALLOC_N(thin_backend_t, 1);
  VALUE obj = Data_Wrap_Struct(klass, NULL, thin_backend_free, backend);
  
  backend->connections = array_create(THIN_CONNECTIONS_SIZE, sizeof(thin_connection_t));
  thin_connections_create(backend->connections, THIN_CONNECTIONS_SIZE);
  
  backend->buffer_pool = pool_create(THIN_BUFFER_SLICES, THIN_BUFFER_SIZE);
  
  backend->obj = obj;
  
  return obj;
}

void thin_backend_define(void)
{
  /* Plug our C stuff into the Ruby world */
  VALUE mThin = rb_define_module("Thin");
  VALUE cBackend = rb_define_class_under(mThin, "Backend", rb_cObject);
  
  rb_define_alloc_func(cBackend, thin_backend_alloc);
  rb_define_method(cBackend, "initialize", thin_backend_init, 3);
  rb_define_protected_method(cBackend, "listen", thin_backend_listen, 0);
  rb_define_protected_method(cBackend, "loop!", thin_backend_loop, 0);
  rb_define_protected_method(cBackend, "close", thin_backend_close, 0);
}
