#include "thin.h"

#define EV_STANDALONE 1
#include <ev.c>

VALUE cStringIO;
VALUE sInternedCall;
VALUE sInternedKeys;
VALUE sRackInput;

void Init_thin_backend()
{  
  /* Intern some Ruby strings and symbols */
  cStringIO     = rb_define_class("StringIO", rb_cData);
  sInternedCall = rb_intern("call");
  sInternedKeys = rb_intern("keys");
  sRackInput    = rb_obj_freeze(rb_str_new2("rack.input"));
  rb_gc_register_address(&sRackInput);
  
  /* Initialize internal stuff */
  backend_define();
  parser_callbacks_define();
}
