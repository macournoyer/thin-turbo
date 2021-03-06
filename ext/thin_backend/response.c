/* Functions to produce and send the HTTP response
 * Life of a response:
 * 1) response_process is called when the request is ready to be processed
 * 2) A Ruby thread is created and response_run is called inside it.
 * 3) response_run calls the Rack application (c->backend->app)
 * 4) response_send_status, response_send_headers, response_send_body is called
 *    when the response is received from the Rackk application.
 * 5) Each of these 3 function call response_send_chunk w/ each chunk of data
 *    that needs to be sent.
 * 6) For each chunk of data in response_send_chunk:
 *    - If the chunk is to big to fit in the send buffer, it's splitted in smaller ones
 *    - If the send buffer is full, the event loop is called until space if available
 *    - If the send buffer contains enought data we go for 1 iteration in the event loop
 *      to stream the response.
 */
#include "thin.h"

static void response_send_chunk(connection_t *c, const char *ptr, size_t len)
{
  /* chunk too big, split it in smaller chunks and send each separately */
  if (len >= BUFFER_MAX_LEN) {
    size_t size   = BUFFER_MAX_LEN - 1;
    size_t slices = len / size + 1;
    size_t i, offset = 0;
    
    for (i = 0; i < slices; ++i) {
      /* last slice can be smaller */
      if (i == slices - 1)
        size = len % size;
      
      response_send_chunk(c, (char *) ptr + offset, size);
      
      offset += size;      
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
  if (c->write_buffer.len - c->write_buffer.offset >= STREAM_SIZE)
    ev_loop(c->loop, EVLOOP_ONESHOT | EVLOOP_NONBLOCK);    
}

static void response_send_status(connection_t *c, const int status)
{
  buffer_t *buf = &c->write_buffer;
  char     *status_line = get_status_line(status);
  
  buffer_append(buf, RESP_HTTP_VERSION, sizeof(RESP_HTTP_VERSION) - 1);
  buffer_append(buf, status_line, strlen(status_line));
  buffer_append(buf, CRLF, sizeof(CRLF) - 1);
}

static VALUE iter_header(VALUE value, VALUE *args)
{
  connection_t *c   = (connection_t *) args[0];
  VALUE         key = (VALUE) args[1];
  
  response_send_chunk(c, RSTRING_PTR(key), RSTRING_LEN(key));
  response_send_chunk(c, HEADER_SEP, sizeof(HEADER_SEP) - 1);
  
  /* if value ends w/ line break w/ chomp it! */
  size_t len = RSTRING_LEN(value);
  if (RSTRING_PTR(value)[RSTRING_LEN(value) - 1] == '\n')
    len--;
  
  response_send_chunk(c, RSTRING_PTR(value), len);  
  response_send_chunk(c, CRLF, sizeof(CRLF) - 1);
  
  return Qnil;
}

static void response_send_headers(connection_t *c, VALUE headers)
{
  VALUE     hash, keys, key, value;
  size_t    i;
  
  keys = rb_funcall(headers, sInternedKeys, 0);
  
  for (i = 0; i < RARRAY_LEN(keys); ++i) {
    key   = RARRAY_PTR(keys)[i];
    value = rb_hash_aref(headers, key);
    
    VALUE args[2] = { (VALUE) c, key };
    
    rb_iterate(rb_each, value, iter_header, (VALUE) args);
  }
  
  response_send_chunk(c, CRLF, sizeof(CRLF) - 1);
}

static VALUE iter_body(VALUE chunk, VALUE *val_conn)
{
  connection_t *c = (connection_t *) val_conn;
  
  response_send_chunk(c, RSTRING_PTR(chunk), RSTRING_LEN(chunk));
  
  return Qnil;
}

static void response_send_body(connection_t *c, VALUE body)
{
  if (TYPE(body) == T_STRING && RSTRING_LEN(body) < BUFFER_MAX_LEN) {
    /* Calling String#each creates several other strings which is slower and use more mem,
     * also Ruby 1.9 doesn't define that method anymore, so it's better to send one big string. */    
    response_send_chunk(c, RSTRING_PTR(body), RSTRING_LEN(body));
    
  } else {
    /* Iterate over body#each and send each yielded chunk */
    rb_iterate(rb_each, body, iter_body, (VALUE) c);
    
  }
}

static VALUE response_run(connection_t *c)
{
  /* Call the app to process the request */  
  VALUE response = rb_funcall_rescue(c->backend->app, sInternedCall, 1, c->env);
  unsigned sent = 0;
  
  if (response == Qundef) {
    /* log any error */
    rb_funcall(c->backend->obj, rb_intern("log_last_exception"), 0);
    sent = 1;
    
  } else {
    /* store response info and prepare for writing */
    int   status  = FIX2INT(rb_ary_entry(response, 0));
    VALUE headers = rb_ary_entry(response, 1);
    VALUE body    = rb_ary_entry(response, 2);
    
    /* read buffer no longer needed, free up now so we
     * can reuse some of it for write buffer */
    buffer_reset(&c->read_buffer);
    
    connection_watch_writable(c);
    
    response_send_status(c, status);
    response_send_headers(c, headers);
    response_send_body(c, body);
    
    c->finished = 1;
    
    if (buffer_eof(&c->write_buffer))
      sent = 1;
    
  }
  
  c->backend->thread_count--;
  c->thread.active = 0;
  
  if (sent)
    connection_close(c);
  
  return Qnil;
}

void response_process(connection_t *c)
{
  c->backend->thread_count++;
  /* call the Rack app in a Ruby green thread */
  c->thread.active = 1;
  c->thread.obj = rb_thread_create(response_run, (void*) c);
}
