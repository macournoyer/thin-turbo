/**
 * Copyright (c) 2005 Zed A. Shaw
 * You can redistribute it and/or modify it under the same terms as Ruby.
 */
#include "parser.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define LEN(AT, FPC)       (FPC - buffer - parser->AT)
#define MARK(M,FPC)        (parser->M = (FPC) - buffer)
#define PTR_TO(F)          (buffer + parser->F)
#define PARSER_ERROR       (parser->error = 1)

#define MAX_FIELD_NAME     256
#define MAX_FIELD_VALUE    80 * 1024
#define MAX_REQUEST_URI    1024 * 12
#define MAX_FRAGMENT       1024 /* Don't know if this length is specified somewhere or not */
#define MAX_REQUEST_PATH   1024
#define MAX_QUERY_STRING   1024 * 10
#define MAX_CONTENT_LENGTH 20
#define MAX_CONTENT_TYPE   1024
#define MAX_HEADER         1024 * (80 + 32)

/** machine **/
%%{
  machine http_parser;

  action mark { MARK(mark, fpc); }

  action start_field { MARK(field_start, fpc); }
  action write_field {
    parser->field_len = LEN(field_start, fpc);
    if (parser->field_len > MAX_FIELD_NAME) { PARSER_ERROR; fbreak; }
  }

  action start_value { MARK(mark, fpc); }
  action write_value {
    size_t vlen = LEN(mark, fpc);
    
    if (vlen > MAX_FIELD_VALUE) { PARSER_ERROR; fbreak; }
    if (parser->http_field != NULL)
      parser->http_field(parser->data, PTR_TO(field_start), parser->field_len, PTR_TO(mark), vlen);
  }
  
  action content_length {
    size_t len = LEN(mark, fpc);
    
    if (len > MAX_CONTENT_LENGTH) { PARSER_ERROR; fbreak; }
    if (parser->content_length != NULL)
      parser->content_length(parser->data, PTR_TO(mark), len);
  }
  
  action content_type {
    size_t len = LEN(mark, fpc);
    
    if (len > MAX_CONTENT_TYPE) { PARSER_ERROR; fbreak; }
    if (parser->content_type != NULL)
      parser->content_type(parser->data, PTR_TO(mark), len);
  }
  
  action request_method { 
    if (parser->request_method != NULL) 
      parser->request_method(parser->data, PTR_TO(mark), LEN(mark, fpc));
  }
  action request_uri {
    size_t len = LEN(mark, fpc);
    
    if (len > MAX_REQUEST_URI) { PARSER_ERROR; fbreak; }
    if (parser->request_uri != NULL)
      parser->request_uri(parser->data, PTR_TO(mark), LEN(mark, fpc));
  }

  action start_query { MARK(query_start, fpc); }
  action query_string {
    size_t len = LEN(query_start, fpc);
    
    if (len > MAX_QUERY_STRING) { PARSER_ERROR; fbreak; } 
    if (parser->query_string != NULL)
      parser->query_string(parser->data, PTR_TO(query_start), len);
  }

  action http_version {	
    if (parser->http_version != NULL)
      parser->http_version(parser->data, PTR_TO(mark), LEN(mark, fpc));
  }

  action request_path {
    size_t len = LEN(mark, fpc);
    
    if (len > MAX_REQUEST_PATH) { PARSER_ERROR; fbreak; }
    if (parser->request_path != NULL)
      parser->request_path(parser->data, PTR_TO(mark), len);
  }
  
  action fragment {
    size_t len = LEN(mark, fpc);
    
    if (len > MAX_FRAGMENT) { PARSER_ERROR; fbreak; }
    if (parser->fragment != NULL)
      parser->fragment(parser->data, PTR_TO(mark), len);
  }  

  action done { 
    parser->body_start = fpc - buffer + 1; 
    if(parser->header_done != NULL)
      parser->header_done(parser->data, fpc + 1, pe - fpc - 1);
    fbreak;
  }


#### HTTP PROTOCOL GRAMMAR
# line endings
  CRLF = "\r\n";

# character types
  CTL = (cntrl | 127);
  safe = ("$" | "-" | "_" | ".");
  extra = ("!" | "*" | "'" | "(" | ")" | ",");
  reserved = (";" | "/" | "?" | ":" | "@" | "&" | "=" | "+");
  unsafe = (CTL | " " | "\"" | "#" | "%" | "<" | ">");
  national = any -- (alpha | digit | reserved | extra | safe | unsafe);
  unreserved = (alpha | digit | safe | extra | national);
  escape = ("%" xdigit xdigit);
  uchar = (unreserved | escape);
  pchar = (uchar | ":" | "@" | "&" | "=" | "+");
  tspecials = ("(" | ")" | "<" | ">" | "@" | "," | ";" | ":" | "\\" | "\"" | "/" | "[" | "]" | "?" | "=" | "{" | "}" | " " | "\t");

# elements
  token = (ascii -- (CTL | tspecials));

# URI schemes and absolute paths
  scheme = ( alpha | digit | "+" | "-" | "." )* ;
  absolute_uri = (scheme ":" (uchar | reserved )*);

  path = (pchar+ ( "/" pchar* )*) ;
  query = ( uchar | reserved )* %query_string ;
  param = ( pchar | "/" )* ;
  params = (param ( ";" param )*) ;
  rel_path = (path? %request_path (";" params)?) ("?" %start_query query)?;
  absolute_path = ("/"+ rel_path);

  Request_URI = ("*" | absolute_uri | absolute_path) >mark %request_uri;
  Fragment = ( uchar | reserved )* >mark %fragment;
  Method = (upper | digit | safe){1,20} >mark %request_method;

  http_number = (digit+ "." digit+) ;
  HTTP_Version = ("HTTP/" http_number) >mark %http_version ;
  Request_Line = (Method " " Request_URI " " HTTP_Version CRLF) ;

  field_name = (token -- ":")+ >start_field %write_field;

  field_value = any* >start_value %write_value;

  known_header = ( ("Content-Length:"i " "* (digit+ >mark %content_length))
                 | ("Content-Type:"i   " "* (any* >mark %content_type))
                 ) :> CRLF;
  unknown_header = (field_name ":" " "* field_value :> CRLF) -- known_header;
  
  Request = Request_Line (known_header | unknown_header)* ( CRLF @done );

main := Request;
}%%

/** Data **/
%% write data;

int http_parser_init(http_parser *parser)  {
  int cs = 0;
  %% write init;
  parser->cs = cs;
  parser->body_start = 0;
  parser->content_len = 0;
  parser->mark = 0;
  parser->nread = 0;
  parser->field_len = 0;
  parser->field_start = 0;
  parser->error = 0;

  return(1);
}


/** exec **/
size_t http_parser_execute(http_parser *parser, const char *buffer, size_t len, size_t off)  {
  const char *p, *pe;
  int cs = parser->cs;

  assert(off <= len && "offset past end of buffer");

  p = buffer+off;
  pe = buffer+len;

  assert(*pe == '\0' && "pointer does not end on NUL");
  assert(pe - p == len - off && "pointers aren't same distance");


  %% write exec;

  parser->cs = cs;
  parser->nread += p - (buffer + off);

  assert(p <= pe && "buffer overflow after parsing execute");
  assert(parser->nread <= len && "nread longer than length");
  assert(parser->body_start <= len && "body starts after buffer end");
  assert(parser->mark < len && "mark is after buffer end");
  assert(parser->field_len <= len && "field has length longer than whole buffer");
  assert(parser->field_start < len && "field starts after buffer end");

  if(parser->body_start) {
    /* final \r\n combo encountered so stop right here */
    %%write eof;
    parser->nread++;
  }

  return(parser->nread);
}

int http_parser_finish(http_parser *parser)
{
  int cs = parser->cs;

  %%write eof;

  parser->cs = cs;

  if (http_parser_has_error(parser) ) {
    return -1;
  } else if (http_parser_is_finished(parser) ) {
    return 1;
  } else {
    return 0;
  }
}

int http_parser_has_error(http_parser *parser) {
  return parser->cs == http_parser_error || parser->error == 1;
}

int http_parser_is_finished(http_parser *parser) {
  return parser->cs == http_parser_first_final;
}
