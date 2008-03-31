#include "thin.h"

void Init_thin_backend()
{
  VALUE mThin = Qnil;
  VALUE cBackend = Qnil;
    
  /* Plug our C stuff into the Ruby world */
  mThin = rb_define_module("Thin");
  cBackend = rb_define_class_under(mThin, "Backend", rb_cObject);
  rb_define_alloc_func(cBackend, thin_backend_alloc);
  rb_define_method(cBackend, "initialize", thin_backend_init, 3);
  rb_define_protected_method(cBackend, "listen", thin_backend_listen, 0);
  rb_define_protected_method(cBackend, "loop!", thin_backend_loop, 0);
  rb_define_protected_method(cBackend, "close", thin_backend_close, 0);
  
  /* Initialize internal stuff */
  thin_connections_init();
  thin_parser_callbacks_init(mThin);
}