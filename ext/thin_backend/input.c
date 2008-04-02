#include "thin.h"

static VALUE cInput;

#define thin_buffer_to_s(buf) rb_str_new(buf->ptr, buf->len)

VALUE thin_input_gets(VALUE self)
{
  thin_buffer_t *buf = NULL;
  DATA_GET(self, thin_buffer_t, buf);
  
  return thin_buffer_to_s(buf);
}

VALUE thin_input_read(int argc, VALUE *argv, VALUE self)
{
  thin_buffer_t *buf = NULL;
  DATA_GET(self, thin_buffer_t, buf);
  VALUE vlen, str;
  int   maxlen, len;
  
  if (buf->current == buf->len)
    return Qnil;
  
  maxlen = buf->len - buf->current;

  if (rb_scan_args(argc, argv, "01", &vlen) == 0) {
    len = maxlen;
  } else {
    len = FIX2INT(vlen);
    if (len > maxlen)
      len = maxlen;
  }
  
  str = rb_str_new((char *) buf->ptr + buf->current, len);
  buf->current += len;
  
  return str;
}

VALUE thin_input_each(VALUE self)
{
  thin_buffer_t *buf = NULL;
  DATA_GET(self, thin_buffer_t, buf);
  
  rb_yield(thin_buffer_to_s(buf));
  
  return Qnil;
}

VALUE thin_input_new(thin_buffer_t *buf)
{
  return Data_Wrap_Struct(cInput, NULL, NULL, buf);
}

void thin_input_define(void)
{
  /* Plug our C stuff into the Ruby world */
  VALUE mThin = rb_define_module("Thin");
  cInput = rb_define_class_under(mThin, "Input", rb_cObject);
  
  rb_define_method(cInput, "gets", thin_input_gets, 0);
  rb_define_method(cInput, "read", thin_input_read, -1);
  rb_define_method(cInput, "each", thin_input_each, 0);
}