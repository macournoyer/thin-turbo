/** Defines common length and error messages for input length validation. */
#define DEF_MAX_LENGTH(N,length) const size_t MAX_##N##_LENGTH = length; const char *MAX_##N##_LENGTH_ERR = "HTTP element " # N  " is longer than the " # length " allowed length."

/** Validates the max length of given input and throws an HttpParserError exception if over. */
#define VALIDATE_MAX_LENGTH(len, N) if(len > MAX_##N##_LENGTH) { rb_raise(eParserError, MAX_##N##_LENGTH_ERR); }

/** Defines global strings in the init method. */
#define DEF_GLOBAL(N, val) global_##N = rb_obj_freeze(rb_str_new2(val)); rb_global_variable(&global_##N)

/* Defines the maximum allowed lengths for various input elements.*/
DEF_MAX_LENGTH(FIELD_NAME, 256);
DEF_MAX_LENGTH(FIELD_VALUE, 80 * 1024);
DEF_MAX_LENGTH(REQUEST_URI, 1024 * 12);
DEF_MAX_LENGTH(FRAGMENT, 1024); /* Don't know if this length is specified somewhere or not */
DEF_MAX_LENGTH(REQUEST_PATH, 1024);
DEF_MAX_LENGTH(QUERY_STRING, (1024 * 10));
DEF_MAX_LENGTH(HEADER, (1024 * (80 + 32)));

static VALUE eParserError;

static VALUE global_http_prefix;
static VALUE global_request_method;
static VALUE global_request_uri;
static VALUE global_fragment;
static VALUE global_request_path;
static VALUE global_query_string;
static VALUE global_http_version;
static VALUE global_request_body;
static VALUE global_server_name;
static VALUE global_server_port;
static VALUE global_path_info;
static VALUE global_content_length;
static VALUE global_http_host;

void thin_parser_cb_init(VALUE module)
{
  eParserError = rb_define_class_under(module, "InvalidRequest", rb_eIOError);
  
  DEF_GLOBAL(http_prefix, "HTTP_");
  DEF_GLOBAL(request_method, "REQUEST_METHOD");  
  DEF_GLOBAL(request_uri, "REQUEST_URI");
  DEF_GLOBAL(fragment, "FRAGMENT");
  DEF_GLOBAL(request_path, "REQUEST_PATH");
  DEF_GLOBAL(query_string, "QUERY_STRING");
  DEF_GLOBAL(http_version, "HTTP_VERSION");
  DEF_GLOBAL(request_body, "REQUEST_BODY");
  DEF_GLOBAL(server_name, "SERVER_NAME");
  DEF_GLOBAL(server_port, "SERVER_PORT");
  DEF_GLOBAL(path_info, "PATH_INFO");
  DEF_GLOBAL(content_length, "CONTENT_LENGTH");
  DEF_GLOBAL(http_host, "HTTP_HOST");
}

void http_field_cb(void *data, const char *field, size_t flen, const char *value, size_t vlen)
{
  thin_connection *connection = (thin_connection*)(data);  
  char *ch, *end;
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

  rb_hash_aset(connection->env, f, v);
}

void request_method_cb(void *data, const char *at, size_t length)
{
  thin_connection *connection = (thin_connection*)(data);
  VALUE val = Qnil;

  val = rb_str_new(at, length);
  rb_hash_aset(connection->env, global_request_method, val);
}

void request_uri_cb(void *data, const char *at, size_t length)
{
  thin_connection *connection = (thin_connection*)(data);
  VALUE val = Qnil;

  VALIDATE_MAX_LENGTH(length, REQUEST_URI);

  val = rb_str_new(at, length);
  rb_hash_aset(connection->env, global_request_uri, val);
}

void fragment_cb(void *data, const char *at, size_t length)
{
  thin_connection *connection = (thin_connection*)(data);
  VALUE val = Qnil;

  VALIDATE_MAX_LENGTH(length, FRAGMENT);

  val = rb_str_new(at, length);
  rb_hash_aset(connection->env, global_fragment, val);
}

void request_path_cb(void *data, const char *at, size_t length)
{
  thin_connection *connection = (thin_connection*)(data);
  VALUE val = Qnil;

  VALIDATE_MAX_LENGTH(length, REQUEST_PATH);

  val = rb_str_new(at, length);
  rb_hash_aset(connection->env, global_request_path, val);
  rb_hash_aset(connection->env, global_path_info, val);
}

void query_string_cb(void *data, const char *at, size_t length)
{
  thin_connection *connection = (thin_connection*)(data);
  VALUE val = Qnil;

  VALIDATE_MAX_LENGTH(length, QUERY_STRING);

  val = rb_str_new(at, length);
  rb_hash_aset(connection->env, global_query_string, val);
}

void http_version_cb(void *data, const char *at, size_t length)
{
  thin_connection *connection = (thin_connection*)(data);
  VALUE val = rb_str_new(at, length);
  rb_hash_aset(connection->env, global_http_version, val);
}

void content_length_cb(void *data, const char *at, size_t length)
{
  thin_connection *connection = (thin_connection*)(data);
  VALUE val = rb_str_new(at, length);
  rb_hash_aset(connection->env, global_content_length, val);
}
