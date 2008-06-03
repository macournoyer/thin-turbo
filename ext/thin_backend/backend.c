#include "thin.h"

/* event callbacks */

static void backend_accept_cb(EV_P_ struct ev_io *watcher, int revents)
{
  backend_t         *backend = get_ev_data(backend, watcher, accept);
  struct sockaddr_in remote_addr;
  socklen_t          sin_size = sizeof(remote_addr);
  int                fd, flags;
  
  if (EV_ERROR & revents) {
    rb_sys_fail("Error on acceptor socket");
    return;
  }
  
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

static void backend_idle_cb(EV_P_ struct ev_idle *w, int revents)
{
  ev_idle_stop(EV_A, w);
}

static void backend_prepare_cb(EV_P_ ev_prepare *w, int revents)
{
  backend_t *backend = get_ev_data(backend, w, prepare);
  
  if (backend->thread_count == 0)
    return;
  
  /* run Ruby scheduler and give active threads a kick in the ass */
  rb_thread_schedule();

  /* if still some runnable threads, poll anyways, but do not block
   * inspired by http://lists.schmorp.de/pipermail/libev/2008q2/000237.html */
  if (backend->thread_count > 0 && !ev_is_active(&backend->idle_watcher))
    ev_idle_start(backend->loop, &backend->idle_watcher);
}


/* public api */

VALUE backend_listen_on_port(VALUE self, VALUE address, VALUE port)
{
  backend_t *backend = NULL;
  int        sock_flags = 1;

  DATA_GET(self, backend_t, backend);
  
  backend->app = rb_ivar_get(self, rb_intern("@app"));
  backend->timeout = NUM2DBL(rb_ivar_get(self, rb_intern("@timeout")));
  backend->address = RSTRING_PTR(address);
  backend->port = FIX2INT(port);
  
  memset(&backend->local_addr, 0, sizeof(backend->local_addr));
  backend->local_addr.sin_family = AF_INET;
  backend->local_addr.sin_port = htons(backend->port);
  backend->local_addr.sin_addr.s_addr = inet_addr(backend->address);
  
  backend->loop = ev_default_loop(0);
  backend->thread_count = 0;
  
  /* If running in a thread or if there are other threads we have to run
   * the scheduler on each loop call. */
  if (!rb_thread_alone())
    backend->thread_count++;
  
  if ((backend->fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    rb_sys_fail("socket");

  /* set socket as beeing nonblocking */
  if (fcntl(backend->fd, F_SETFL, O_NONBLOCK) < 0)
    rb_sys_fail("fcntl");

  if (setsockopt(backend->fd, SOL_SOCKET, SO_REUSEADDR, &sock_flags, sizeof(sock_flags)) == -1)
    rb_sys_fail("setsockopt(SO_REUSEADDR)");
  
  if (bind(backend->fd, (struct sockaddr *)&backend->local_addr, sizeof(backend->local_addr)) == -1)
    rb_sys_fail("bind");

  if (listen(backend->fd, LISTEN_BACKLOG) == -1)
    rb_sys_fail("listen");
  
  /* initialise watchers */
  backend->accept_watcher.data = backend;
  ev_io_init(&backend->accept_watcher, backend_accept_cb, backend->fd, EV_READ | EV_ERROR);
  
  backend->prepare_watcher.data = backend;
  ev_prepare_init(&backend->prepare_watcher, backend_prepare_cb);
  ev_set_priority(&backend->prepare_watcher, EV_MINPRI);
  ev_prepare_start(backend->loop, &backend->prepare_watcher);
  ev_unref(backend->loop);

  backend->idle_watcher.data = backend;
  ev_idle_init(&backend->idle_watcher, backend_idle_cb);
  ev_set_priority(&backend->idle_watcher, EV_MINPRI);
  
  backend->open = 1;
  
  ev_io_start(backend->loop, &backend->accept_watcher);
  
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

VALUE backend_set_trace(VALUE self, VALUE state)
{
  backend_t *backend = NULL;
  DATA_GET(self, backend_t, backend);
  
  backend->trace = (state == Qtrue);
  
  return state;
}

VALUE backend_set_maxfds(VALUE self, VALUE max)
{
  struct rlimit rlim;
  int imax = NUM2INT(max);
  
  getrlimit(RLIMIT_NOFILE, &rlim);
  
  rlim.rlim_cur = imax;
	if (imax > rlim.rlim_max)
		rlim.rlim_max = imax;

  setrlimit(RLIMIT_NOFILE, &rlim);
  /* ignore errors, max wont be set */

	return max;
}

VALUE backend_get_maxfds(VALUE self)
{
  struct rlimit rlim;
  
  getrlimit(RLIMIT_NOFILE, &rlim);

	return INT2FIX(rlim.rlim_cur);
}

static void backend_free(backend_t *backend)
{
  if (backend) {
    connections_free(backend);    
    free(backend);
  }
}

VALUE backend_alloc(VALUE klass)
{
  backend_t *backend = ALLOC_N(backend_t, 1);
  VALUE obj = Data_Wrap_Struct(klass, NULL, backend_free, backend);  
  
  backend->address = NULL;
  backend->port    = 3000;
  backend->fd      = -1;
  backend->open    = 0;
  
  backend->trace   = 0;
  
  backend->obj     = obj;
  backend->app     = Qnil;
  
  connections_init(backend);
  backend->thread_count = 0;
  
  backend->loop    = NULL;
  
  return obj;
}

void backend_define(void)
{
  /* Plug our C stuff into the Ruby world */
  VALUE mThin = rb_define_module_under(rb_define_module("Thin"), "Backends");
  VALUE cBackend = rb_define_class_under(mThin, "Turbo", rb_cObject);
  
  rb_define_alloc_func(cBackend, backend_alloc);
  rb_define_protected_method(cBackend, "listen_on_port", backend_listen_on_port, 2);
  rb_define_protected_method(cBackend, "loop!", backend_loop, 0);
  rb_define_protected_method(cBackend, "close", backend_close, 0);
  rb_define_protected_method(cBackend, "trace=", backend_set_trace, 1);
  rb_define_protected_method(cBackend, "maxfds=", backend_set_maxfds, 1);
  rb_define_protected_method(cBackend, "maxfds", backend_get_maxfds, 0);
}
