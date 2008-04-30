#include "thin.h"

static void connection_send_status(connection_t *c, const int status)
{
  buffer_t *buf = &c->write_buffer;
  char     *status_line = get_status_line(status);
  #define   RESP_HTTP_VERSION "HTTP/1.1 "
  
  buffer_append(buf, RESP_HTTP_VERSION, sizeof(RESP_HTTP_VERSION) - 1);
  buffer_append(buf, status_line, strlen(status_line));
  buffer_append(buf, CRLF, sizeof(CRLF) - 1);
}

static void connection_send_headers(connection_t *c, VALUE headers)
{
  buffer_t *buf = &c->write_buffer;
  VALUE     hash, keys, key, value;
  size_t    i, n = 0;
  #define   HEADER_SEP ": "
  
  keys = rb_funcall(headers, sInternedKeys, 0);
  
  /* FIXME very big response header will cause buffer to be stored in tmpfile */
  
  for (i = 0; i < RARRAY_LEN(keys); ++i) {
    key = RARRAY_PTR(keys)[i];
    value = rb_hash_aref(headers, key);

    buffer_append(buf, RSTRING_PTR(key), RSTRING_LEN(key));
    buffer_append(buf, HEADER_SEP, sizeof(HEADER_SEP) - 1);
    buffer_append(buf, RSTRING_PTR(value), RSTRING_LEN(value));
    buffer_append(buf, CRLF, sizeof(CRLF) - 1);
  }
  
  buffer_append(buf, CRLF, sizeof(CRLF) - 1);
}

static void connection_send_chunk(connection_t *c, const char *ptr, size_t len)
{
  /* chunk too big, split it in smaller chunks and send each separately */
  if (len >= BUFFER_MAX_LEN) {
    size_t i, size = BUFFER_MAX_LEN - 1, slices = len / size + 1;
    
    for (i = 0; i < slices; ++i) {
      if (i == slices - 1)
        size = len % size;
      
      connection_send_chunk(c, (char *) ptr + i * size, size);
    }
    
    return;
  }
  
  /* if appending will overflow the buffer we wait till more is sent */
  while (c->write_buffer.len + len > BUFFER_MAX_LEN)
    ev_loop(c->loop, EVLOOP_ONESHOT);
  
  buffer_append(&c->write_buffer, ptr, len);
  
  /* If we have a good sized chunk of data to send, try to send it right away.
   * This allows streaming by going for a shot in the even loop to drain the buffer if possisble,
      this way, the chunk is sent if the socket is writable.*/
  if (c->write_buffer.len - c->write_buffer.offset >= STREAM_SIZE) {
    ev_loop(c->loop, EVLOOP_ONESHOT | EVLOOP_NONBLOCK);    
  }
}

static VALUE iter_body(VALUE chunk, VALUE *val_conn)
{
  connection_t *c = (connection_t *) val_conn;
  
  connection_send_chunk(c, RSTRING_PTR(chunk), RSTRING_LEN(chunk));
  
  return Qnil;
}

static void connection_send_body(connection_t *c, VALUE body)
{
  if (TYPE(body) == T_STRING && RSTRING_LEN(body) < BUFFER_MAX_LEN) {
    /* Calling String#each creates several other strings which is slower and use more mem,
     * also Ruby 1.9 doesn't define that method anymore, so it's better to send one big string. */    
    connection_send_chunk(c, RSTRING_PTR(body), RSTRING_LEN(body));
    
  } else {
    /* Iterate over body#each and send each yielded chunk */
    rb_iterate(rb_each, body, iter_body, (VALUE) c);
    
  }
}

VALUE connection_process(connection_t *c)
{
  /* Call the app to process the request */
  VALUE response = rb_funcall_rescue(c->backend->app, sInternedCall, 1, c->env);

  if (response == Qundef) {
    /* log any error */
    rb_funcall(c->backend->obj, rb_intern("log_last_exception"), 0);
    connection_close(c);
    
  } else {
    /* store response info and prepare for writing */
    int   status  = FIX2INT(rb_ary_entry(response, 0));
    VALUE headers = rb_ary_entry(response, 1);
    VALUE body    = rb_ary_entry(response, 2);
    
    /* read buffer no longer needed, free up now so we
     * can reuse some of it for write buffer */
    buffer_reset(&c->read_buffer);
    
    connection_watch_writable(c);
    
    connection_send_status(c, status);
    connection_send_headers(c, headers);
    connection_send_body(c, body);
    
    c->finished = 1;
    
    if (buffer_eof(&c->write_buffer))
      connection_close(c);
    
  }
  
  return Qnil;
}
