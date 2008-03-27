#include "thin.h"

static VALUE global_empty;
static VALUE global_http_prefix;
static VALUE global_request_method;
static VALUE global_request_uri;
static VALUE global_fragment;
static VALUE global_query_string;
static VALUE global_http_version;
static VALUE global_content_length;
static VALUE global_http_content_length;
static VALUE global_request_path;
static VALUE global_content_type;
static VALUE global_http_content_type;
static VALUE global_gateway_interface;
static VALUE global_gateway_interface_value;
static VALUE global_server_name;
static VALUE global_server_port;
static VALUE global_server_protocol;
static VALUE global_server_protocol_value;
static VALUE global_http_host;
static VALUE global_port_80;
static VALUE global_http_body;
static VALUE global_url_scheme;
static VALUE global_url_scheme_value;
static VALUE global_script_name;
static VALUE global_path_info;

/** Defines common length and error messages for input length validation. */
#define DEF_MAX_LENGTH(N,length) const size_t MAX_##N##_LENGTH = length; const char *MAX_##N##_LENGTH_ERR = "HTTP element " # N  " is longer than the " # length " allowed length."

/** Validates the max length of given input and throws an HttpParserError exception if over. */
#define VALIDATE_MAX_LENGTH(len, N) if(len > MAX_##N##_LENGTH) { /* TODO */ puts(MAX_##N##_LENGTH_ERR); }

/** Defines global strings in the init method. */
#define DEF_GLOBAL(N, val)   global_##N = rb_obj_freeze(rb_str_new2(val)); rb_global_variable(&global_##N)

/* Defines the maximum allowed lengths for various input elements.*/
DEF_MAX_LENGTH(FIELD_NAME, 256);
DEF_MAX_LENGTH(FIELD_VALUE, 80 * 1024);
DEF_MAX_LENGTH(REQUEST_URI, 1024 * 12);
DEF_MAX_LENGTH(FRAGMENT, 1024); /* Don't know if this length is specified somewhere or not */
DEF_MAX_LENGTH(REQUEST_PATH, 1024);
DEF_MAX_LENGTH(QUERY_STRING, (1024 * 10));
DEF_MAX_LENGTH(HEADER, (1024 * (80 + 32)));


void http_field(void *data, const char *field, size_t flen, const char *value, size_t vlen)
{
  char *ch, *end;
  VALUE req = ((thin_connection_t*) data)->env;
  VALUE v = Qnil;
  VALUE f = Qnil;

  VALIDATE_MAX_LENGTH(flen, FIELD_NAME);
  VALIDATE_MAX_LENGTH(vlen, FIELD_VALUE);

  v = rb_str_new(value, vlen);
  f = rb_str_dup(global_http_prefix);
  f = rb_str_buf_cat(f, field, flen); 

  for(ch = RSTRING_PTR(f), end = ch + RSTRING_LEN(f); ch < end; ch++) {
    if(*ch == '-') {
      *ch = '_';
    } else {
      *ch = toupper(*ch);
    }
  }

  rb_hash_aset(req, f, v);
}

void request_method(void *data, const char *at, size_t length)
{
  VALUE req = ((thin_connection_t*) data)->env;
  VALUE val = Qnil;

  val = rb_str_new(at, length);
  rb_hash_aset(req, global_request_method, val);
}

void request_uri(void *data, const char *at, size_t length)
{
  VALUE req = ((thin_connection_t*) data)->env;
  VALUE val = Qnil;

  VALIDATE_MAX_LENGTH(length, REQUEST_URI);

  val = rb_str_new(at, length);
  rb_hash_aset(req, global_request_uri, val);
}

void fragment(void *data, const char *at, size_t length)
{
  VALUE req = ((thin_connection_t*) data)->env;
  VALUE val = Qnil;

  VALIDATE_MAX_LENGTH(length, FRAGMENT);

  val = rb_str_new(at, length);
  rb_hash_aset(req, global_fragment, val);
}

void request_path(void *data, const char *at, size_t length)
{
  VALUE req = ((thin_connection_t*) data)->env;
  VALUE val = Qnil;

  VALIDATE_MAX_LENGTH(length, REQUEST_PATH);

  val = rb_str_new(at, length);
  rb_hash_aset(req, global_request_path, val);
  rb_hash_aset(req, global_path_info, val);
}

void query_string(void *data, const char *at, size_t length)
{
  VALUE req = ((thin_connection_t*) data)->env;
  VALUE val = Qnil;

  VALIDATE_MAX_LENGTH(length, QUERY_STRING);

  val = rb_str_new(at, length);
  rb_hash_aset(req, global_query_string, val);
}

void http_version(void *data, const char *at, size_t length)
{
  VALUE req = ((thin_connection_t*) data)->env;
  VALUE val = rb_str_new(at, length);
  rb_hash_aset(req, global_http_version, val);
}

/** Finalizes the request header to have a bunch of stuff that's
  needed. */

void header_done(void *data, const char *at, size_t length)
{
  VALUE env = ((thin_connection_t*) data)->env;
  VALUE temp = Qnil;
  VALUE ctype = Qnil;
  VALUE body = Qnil;
  char *colon = NULL;

  ctype = rb_hash_aref(env, global_http_content_type);
  if(ctype != Qnil) {
    rb_hash_aset(env, global_content_type, ctype);
    rb_hash_delete(env, global_http_content_type);
  }

  rb_hash_aset(env, global_gateway_interface, global_gateway_interface_value);
  if((temp = rb_hash_aref(env, global_http_host)) != Qnil) {
    /* ruby better close strings off with a '\0' dammit */
    colon = strchr(RSTRING_PTR(temp), ':');
    if(colon != NULL) {
      rb_hash_aset(env, global_server_name, rb_str_substr(temp, 0, colon - RSTRING_PTR(temp)));
      rb_hash_aset(env, global_server_port, 
          rb_str_substr(temp, colon - RSTRING_PTR(temp)+1, 
            RSTRING_LEN(temp)));
    } else {
      rb_hash_aset(env, global_server_name, temp);
      rb_hash_aset(env, global_server_port, global_port_80);
    }
  }

  /* grab the initial body and stuff it into the hash */
  if(length > 0) {
    body = rb_hash_aref(env, global_http_body);
    rb_io_write(body, rb_str_new(at, length));
  }
  
  /* according to Rack specs, query string must be empty string if none */
  if (rb_hash_aref(env, global_query_string) == Qnil) {
    rb_hash_aset(env, global_query_string, global_empty);
  }
  
  /* set some constants */
  rb_hash_aset(env, global_server_protocol, global_server_protocol_value);
  rb_hash_aset(env, global_url_scheme, global_url_scheme_value);
  rb_hash_aset(env, global_script_name, global_empty);
}

void content_length(void *data, const char *at, size_t length)
{
  thin_connection_t *connection = (thin_connection_t*)(data);
  int                i, mult;
  
  connection->content_length = 0;
  for (mult=1, i = length - 1; i >= 0; i--, mult *= 10)
    connection->content_length += (at[i] - '0') * mult;

  VALUE val = rb_str_new(at, length);
  rb_hash_aset(connection->env, global_content_length, val);
}

void thin_parser_callbacks_init(VALUE module)
{
  DEF_GLOBAL(empty, "");
  DEF_GLOBAL(http_prefix, "HTTP_");
  DEF_GLOBAL(request_method, "REQUEST_METHOD");
  DEF_GLOBAL(request_uri, "REQUEST_URI");
  DEF_GLOBAL(fragment, "FRAGMENT");
  DEF_GLOBAL(query_string, "QUERY_STRING");
  DEF_GLOBAL(http_version, "HTTP_VERSION");
  DEF_GLOBAL(request_path, "REQUEST_PATH");
  DEF_GLOBAL(content_length, "CONTENT_LENGTH");
  DEF_GLOBAL(http_content_length, "HTTP_CONTENT_LENGTH");
  DEF_GLOBAL(content_type, "CONTENT_TYPE");
  DEF_GLOBAL(http_content_type, "HTTP_CONTENT_TYPE");
  DEF_GLOBAL(gateway_interface, "GATEWAY_INTERFACE");
  DEF_GLOBAL(gateway_interface_value, "CGI/1.2");
  DEF_GLOBAL(server_name, "SERVER_NAME");
  DEF_GLOBAL(server_port, "SERVER_PORT");
  DEF_GLOBAL(server_protocol, "SERVER_PROTOCOL");
  DEF_GLOBAL(server_protocol_value, "HTTP/1.1");
  DEF_GLOBAL(http_host, "HTTP_HOST");
  DEF_GLOBAL(port_80, "80");
  DEF_GLOBAL(http_body, "rack.input");
  DEF_GLOBAL(url_scheme, "rack.url_scheme");
  DEF_GLOBAL(url_scheme_value, "http");
  DEF_GLOBAL(script_name, "SCRIPT_NAME");
  DEF_GLOBAL(path_info, "PATH_INFO");
}

void thin_setup_parser_callbacks(thin_connection_t *connection)
{
  http_parser_init(&(connection->parser));
  connection->parser.data = connection;
  connection->parser.http_field     = http_field;
  connection->parser.request_method = request_method;
  connection->parser.request_uri    = request_uri;
  connection->parser.fragment       = fragment;
  connection->parser.request_path   = request_path;
  connection->parser.query_string   = query_string;
  connection->parser.http_version   = http_version;
  connection->parser.header_done    = header_done;
  connection->parser.content_length = content_length;  
}
