#include "thin.h"

#define EV_STANDALONE 1
#include <ev.c>

VALUE sInternedCall;
VALUE sInternedKeys;
VALUE sRackInput;

void Init_thin_backend()
{  
  /* Intern some Ruby strings and symbols */
  sInternedCall = rb_intern("call");
  sInternedKeys = rb_intern("keys");
  sRackInput    = rb_obj_freeze(rb_str_new2("rack.input"));
  rb_gc_register_address(&sRackInput);
  
  /* Initialize internal stuff */
  backend_define();
  connections_init();
  parser_callbacks_init();
}
