/* Functions to handle the requesting of a resource, before the Rack application is called.
 * Basically:
 * 1) request_parse is called each time a chunk of data is red from the socket.
 * 3) request_parse then parse the header into c->env, a Ruby hash.
 * 4) When the header is all parsed, the body is stored into c->read_buffer.
 * 5) c->read_buffer is converted into a Ruby class:
 *    - StringIO if < BUFFER_MAX_LEN
 *    - File if stored in a tempfile
 * 6) The response is processed by calling response_process.
 *
 * Note that c->read_buffer is also used to store the header and then
 * cleared before storing the body.
 */
#include "thin.h"

static VALUE buffer_to_ruby_obj(buffer_t *buf)
{
  if (buffer_in_file(buf)) {
    /* close the fd and reopen in a Ruby File object */
    close(buf->file.fd);
    VALUE fname = rb_str_new2(buf->file.name);
    return rb_class_new_instance(1, &fname, rb_cFile);
    
  } else {
    VALUE str = rb_str_new(buf->ptr, buf->len);
    return rb_class_new_instance(1, &str, cStringIO);
    
  }
}

void request_parse(connection_t *c, char *buf, int len)
{
  if (!http_parser_is_finished(&c->parser) && c->read_buffer.len + len > MAX_HEADER) {
    connection_error(c, "Header too big");
    return;
  }
  
  if (buffer_append(&c->read_buffer, buf, len) < 0) {
    connection_error(c, "Error writing to buffer");
    return;
  }
  
  if (!http_parser_is_finished(&c->parser)) {
    /* header not all received, we continue parsing ... */
    
    /* terminate string with null (required by ragel v5) */
    memset(c->read_buffer.ptr + c->read_buffer.len, '\0', 1);
    
    /* parse the request into connection->env */
    c->parser.nread = http_parser_execute(&c->parser,
                                           c->read_buffer.ptr,
                                           c->read_buffer.len,
                                           c->parser.nread);
  
    /* parser error */
    if (http_parser_has_error(&c->parser)) {
      connection_error(c, "Invalid request");
      return;
    }
  }
  
  /* request fully received */
  if (http_parser_is_finished(&c->parser) && c->read_buffer.len >= c->content_length) {
    ev_io_stop(c->loop, &c->read_watcher);
    
    /* assign env[rack.input] */
    rb_hash_aset(c->env, sRackInput, buffer_to_ruby_obj(&c->read_buffer));
    
    /* call the Rack app */
    response_process(c);
  }
}
