#define EV_STANDALONE 1
#include <ev.c>

#include "thin.h"


/* event callbacks */

static void backend_accept_cb(EV_P_ struct ev_io *watcher, int revents)
{
  backend_t         *backend = get_ev_data(backend, watcher, accept);
  struct sockaddr_in remote_addr;
  socklen_t          sin_size = sizeof(remote_addr);
  int                fd, flags;
  
  fd = accept(backend->fd, (struct sockaddr *)&remote_addr, &sin_size);
  if (fd == -1) {
    log_errno(backend);
    return;
  }
  
  flags = fcntl(fd, F_GETFL, 0);
  if ((fcntl(fd, F_SETFL, flags | O_NONBLOCK)) < 0) {
    log_errno(backend);
    return;
  }
  
  connection_start(backend, fd, remote_addr);
}


/* public api */

VALUE backend_set_app(VALUE self, VALUE app)
{
  backend_t *backend = NULL;
  DATA_GET(self, backend_t, backend);
  
  backend->app = app;
  
  return app;
}

VALUE backend_listen_on_port(VALUE self, VALUE address, VALUE port)
{
  backend_t *backend = NULL;
  DATA_GET(self, backend_t, backend);
  int sock_flags;
  
  backend->address = RSTRING_PTR(address);
  backend->port = FIX2INT(port);
  
  backend->local_addr.sin_family = AF_INET;
  backend->local_addr.sin_port = htons(backend->port);
  backend->local_addr.sin_addr.s_addr = inet_addr(backend->address);
  
  backend->loop = ev_default_loop(0);

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

  if (listen(backend->fd, LISTEN_BACKLOG) == -1)
    rb_sys_fail("listen");
  
  /* initialise io watchers */
  watch(backend, backend_accept_cb, accept, EV_READ);  
  
  backend->open = 1;
  
  return Qtrue;
}

VALUE backend_loop(VALUE self)
{
  backend_t *backend = NULL;
  DATA_GET(self, backend_t, backend);
  
  ev_loop(backend->loop, EVLOOP_ONESHOT);
  
  return Qtrue;
}

VALUE backend_close(VALUE self)
{
  backend_t *backend = NULL;
  DATA_GET(self, backend_t, backend);
  
  if (backend->open) {
    backend->open = 0;
    ev_io_stop(backend->loop, &backend->accept_watcher);
    close(backend->fd);    
  }
  
  return Qtrue;
}

static void backend_free(backend_t *backend)
{
  if (backend) {
    array_destroy(backend->connections);
    pool_destroy(backend->buffer_pool);
    free(backend);
  }
}

VALUE backend_alloc(VALUE klass)
{
  backend_t *backend = ALLOC_N(backend_t, 1);
  VALUE obj = Data_Wrap_Struct(klass, NULL, backend_free, backend);
  
  backend->connections = array_create(CONNECTIONS_SIZE, sizeof(connection_t));
  connections_create(backend->connections, CONNECTIONS_SIZE);
  
  backend->buffer_pool = pool_create(BUFFER_SLICES, BUFFER_SIZE);
  
  backend->obj = obj;
  
  return obj;
}

void backend_define(void)
{
  /* Plug our C stuff into the Ruby world */
  VALUE mThin = rb_define_module_under(rb_define_module("Thin"), "Backends");
  VALUE cBackend = rb_define_class_under(mThin, "Turbo", rb_cObject);
  
  rb_define_alloc_func(cBackend, backend_alloc);
  rb_define_method(cBackend, "app=", backend_set_app, 1);
  rb_define_protected_method(cBackend, "listen_on_port", backend_listen_on_port, 2);
  rb_define_protected_method(cBackend, "loop!", backend_loop, 0);
  rb_define_protected_method(cBackend, "close", backend_close, 0);
}
