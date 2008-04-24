/* Allow reading connection->read_buffer from Ruby */

#include "thin.h"

static VALUE cInput;

#define buffer_to_s(buf) rb_str_new(buf->ptr, buf->len)

static VALUE input_gets(VALUE self)
{
  buffer_t *buf = NULL;
  DATA_GET(self, buffer_t, buf);
  
  return buffer_to_s(buf);
}

static VALUE input_read(int argc, VALUE *argv, VALUE self)
{
  buffer_t *buf = NULL;
  DATA_GET(self, buffer_t, buf);
  VALUE vlen, str;
  int   maxlen, len;
  
  if (buf->offset == buf->len)
    return Qnil;
  
  maxlen = buf->len - buf->offset;

  if (rb_scan_args(argc, argv, "01", &vlen) == 0) {
    len = maxlen;
  } else {
    len = FIX2INT(vlen);
    if (len > maxlen)
      len = maxlen;
  }
  
  str = rb_str_new((char *) buf->ptr + buf->offset, len);
  buf->offset += len;
  
  return str;
}

static VALUE input_each(VALUE self)
{
  buffer_t *buf = NULL;
  DATA_GET(self, buffer_t, buf);
  
  rb_yield(buffer_to_s(buf));
  
  return Qnil;
}

VALUE input_new(buffer_t *buf)
{
  return Data_Wrap_Struct(cInput, NULL, NULL, buf);
}

void input_define(void)
{
  /* Plug our C stuff into the Ruby world */
  VALUE mThin = rb_define_module("Thin");
  cInput = rb_define_class_under(mThin, "Input", rb_cObject);
  
  rb_define_method(cInput, "gets", input_gets, 0);
  rb_define_method(cInput, "read", input_read, -1);
  rb_define_method(cInput, "each", input_each, 0);
}
