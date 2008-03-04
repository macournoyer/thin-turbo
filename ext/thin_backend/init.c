#include "ruby.h"
#include "backend.h"
#include "connection.h"

void Init_thin_backend()
{
  VALUE mThin = rb_define_module("Thin");
  
  cBackend = rb_define_class_under(mThin, "Backend", rb_cObject);
  rb_define_alloc_func(cBackend, thin_backend_alloc);
  rb_define_method(cBackend, "initialize", thin_backend_init, 2);
  rb_define_method(cBackend, "start", thin_backend_start, 0);
  rb_define_method(cBackend, "stop", thin_backend_stop, 0);
  rb_define_method(cBackend, "process", thin_backend_process, 0);
}