#line 1 "parser.rl"
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
#line 165 "parser.rl"


/** Data **/

#line 35 "parser.c"
static const int http_parser_start = 1;
static const int http_parser_first_final = 114;
static const int http_parser_error = 0;

static const int http_parser_en_main = 1;

#line 169 "parser.rl"

int http_parser_init(http_parser *parser)  {
  int cs = 0;
  
#line 47 "parser.c"
	{
	cs = http_parser_start;
	}
#line 173 "parser.rl"
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


  
#line 80 "parser.c"
	{
	if ( p == pe )
		goto _out;
	switch ( cs )
	{
case 1:
	switch( (*p) ) {
		case 36: goto tr0;
		case 95: goto tr0;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto tr0;
	} else if ( (*p) > 57 ) {
		if ( 65 <= (*p) && (*p) <= 90 )
			goto tr0;
	} else
		goto tr0;
	goto st0;
st0:
	goto _out0;
tr0:
#line 31 "parser.rl"
	{ MARK(mark, p); }
	goto st2;
st2:
	if ( ++p == pe )
		goto _out2;
case 2:
#line 110 "parser.c"
	switch( (*p) ) {
		case 32: goto tr2;
		case 36: goto st95;
		case 95: goto st95;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st95;
	} else if ( (*p) > 57 ) {
		if ( 65 <= (*p) && (*p) <= 90 )
			goto st95;
	} else
		goto st95;
	goto st0;
tr2:
#line 64 "parser.rl"
	{ 
    if (parser->request_method != NULL) 
      parser->request_method(parser->data, PTR_TO(mark), LEN(mark, p));
  }
	goto st3;
st3:
	if ( ++p == pe )
		goto _out3;
case 3:
#line 136 "parser.c"
	switch( (*p) ) {
		case 42: goto tr4;
		case 43: goto tr5;
		case 47: goto tr6;
		case 58: goto tr7;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto tr5;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr5;
	} else
		goto tr5;
	goto st0;
tr4:
#line 31 "parser.rl"
	{ MARK(mark, p); }
	goto st4;
st4:
	if ( ++p == pe )
		goto _out4;
case 4:
#line 160 "parser.c"
	if ( (*p) == 32 )
		goto tr8;
	goto st0;
tr8:
#line 68 "parser.rl"
	{
    size_t len = LEN(mark, p);
    
    if (len > MAX_REQUEST_URI) { PARSER_ERROR; goto _out5; }
    if (parser->request_uri != NULL)
      parser->request_uri(parser->data, PTR_TO(mark), LEN(mark, p));
  }
	goto st5;
tr104:
#line 90 "parser.rl"
	{
    size_t len = LEN(mark, p);
    
    if (len > MAX_REQUEST_PATH) { PARSER_ERROR; goto _out5; }
    if (parser->request_path != NULL)
      parser->request_path(parser->data, PTR_TO(mark), len);
  }
#line 68 "parser.rl"
	{
    size_t len = LEN(mark, p);
    
    if (len > MAX_REQUEST_URI) { PARSER_ERROR; goto _out5; }
    if (parser->request_uri != NULL)
      parser->request_uri(parser->data, PTR_TO(mark), LEN(mark, p));
  }
	goto st5;
tr114:
#line 76 "parser.rl"
	{ MARK(query_start, p); }
#line 77 "parser.rl"
	{
    size_t len = LEN(query_start, p);
    
    if (len > MAX_QUERY_STRING) { PARSER_ERROR; goto _out5; } 
    if (parser->query_string != NULL)
      parser->query_string(parser->data, PTR_TO(query_start), len);
  }
#line 68 "parser.rl"
	{
    size_t len = LEN(mark, p);
    
    if (len > MAX_REQUEST_URI) { PARSER_ERROR; goto _out5; }
    if (parser->request_uri != NULL)
      parser->request_uri(parser->data, PTR_TO(mark), LEN(mark, p));
  }
	goto st5;
tr117:
#line 77 "parser.rl"
	{
    size_t len = LEN(query_start, p);
    
    if (len > MAX_QUERY_STRING) { PARSER_ERROR; goto _out5; } 
    if (parser->query_string != NULL)
      parser->query_string(parser->data, PTR_TO(query_start), len);
  }
#line 68 "parser.rl"
	{
    size_t len = LEN(mark, p);
    
    if (len > MAX_REQUEST_URI) { PARSER_ERROR; goto _out5; }
    if (parser->request_uri != NULL)
      parser->request_uri(parser->data, PTR_TO(mark), LEN(mark, p));
  }
	goto st5;
st5:
	if ( ++p == pe )
		goto _out5;
case 5:
#line 234 "parser.c"
	if ( (*p) == 72 )
		goto tr9;
	goto st0;
tr9:
#line 31 "parser.rl"
	{ MARK(mark, p); }
	goto st6;
st6:
	if ( ++p == pe )
		goto _out6;
case 6:
#line 246 "parser.c"
	if ( (*p) == 84 )
		goto st7;
	goto st0;
st7:
	if ( ++p == pe )
		goto _out7;
case 7:
	if ( (*p) == 84 )
		goto st8;
	goto st0;
st8:
	if ( ++p == pe )
		goto _out8;
case 8:
	if ( (*p) == 80 )
		goto st9;
	goto st0;
st9:
	if ( ++p == pe )
		goto _out9;
case 9:
	if ( (*p) == 47 )
		goto st10;
	goto st0;
st10:
	if ( ++p == pe )
		goto _out10;
case 10:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st11;
	goto st0;
st11:
	if ( ++p == pe )
		goto _out11;
case 11:
	if ( (*p) == 46 )
		goto st12;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st11;
	goto st0;
st12:
	if ( ++p == pe )
		goto _out12;
case 12:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st13;
	goto st0;
st13:
	if ( ++p == pe )
		goto _out13;
case 13:
	if ( (*p) == 13 )
		goto tr17;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st13;
	goto st0;
tr17:
#line 85 "parser.rl"
	{	
    if (parser->http_version != NULL)
      parser->http_version(parser->data, PTR_TO(mark), LEN(mark, p));
  }
	goto st14;
tr27:
#line 40 "parser.rl"
	{
    size_t vlen = LEN(mark, p);
    
    if (vlen > MAX_FIELD_VALUE) { PARSER_ERROR; goto _out14; }
    if (parser->http_field != NULL)
      parser->http_field(parser->data, PTR_TO(field_start), parser->field_len, PTR_TO(mark), vlen);
  }
	goto st14;
tr89:
#line 48 "parser.rl"
	{
    size_t len = LEN(mark, p);
    
    if (len > MAX_CONTENT_LENGTH) { PARSER_ERROR; goto _out14; }
    if (parser->content_length != NULL)
      parser->content_length(parser->data, PTR_TO(mark), len);
  }
	goto st14;
tr96:
#line 56 "parser.rl"
	{
    size_t len = LEN(mark, p);
    
    if (len > MAX_CONTENT_TYPE) { PARSER_ERROR; goto _out14; }
    if (parser->content_type != NULL)
      parser->content_type(parser->data, PTR_TO(mark), len);
  }
	goto st14;
st14:
	if ( ++p == pe )
		goto _out14;
case 14:
#line 344 "parser.c"
	if ( (*p) == 10 )
		goto st15;
	goto st0;
st15:
	if ( ++p == pe )
		goto _out15;
case 15:
	switch( (*p) ) {
		case 13: goto st16;
		case 33: goto tr20;
		case 67: goto tr21;
		case 99: goto tr21;
		case 124: goto tr20;
		case 126: goto tr20;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto tr20;
		} else if ( (*p) >= 35 )
			goto tr20;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr20;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto tr20;
		} else
			goto tr20;
	} else
		goto tr20;
	goto st0;
st16:
	if ( ++p == pe )
		goto _out16;
case 16:
	if ( (*p) == 10 )
		goto tr22;
	goto st0;
tr22:
#line 106 "parser.rl"
	{ 
    parser->body_start = p - buffer + 1; 
    if(parser->header_done != NULL)
      parser->header_done(parser->data, p + 1, pe - p - 1);
    goto _out114;
  }
	goto st114;
st114:
	if ( ++p == pe )
		goto _out114;
case 114:
#line 398 "parser.c"
	goto st0;
tr20:
#line 33 "parser.rl"
	{ MARK(field_start, p); }
	goto st17;
st17:
	if ( ++p == pe )
		goto _out17;
case 17:
#line 408 "parser.c"
	switch( (*p) ) {
		case 33: goto st17;
		case 58: goto tr24;
		case 67: goto st40;
		case 99: goto st40;
		case 124: goto st17;
		case 126: goto st17;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st17;
		} else if ( (*p) >= 35 )
			goto st17;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st17;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st17;
		} else
			goto st17;
	} else
		goto st17;
	goto st0;
tr24:
#line 34 "parser.rl"
	{
    parser->field_len = LEN(field_start, p);
    if (parser->field_len > MAX_FIELD_NAME) { PARSER_ERROR; goto _out18; }
  }
	goto st18;
tr28:
#line 39 "parser.rl"
	{ MARK(mark, p); }
	goto st18;
st18:
	if ( ++p == pe )
		goto _out18;
case 18:
#line 450 "parser.c"
	switch( (*p) ) {
		case 13: goto tr27;
		case 32: goto tr28;
		case 67: goto tr29;
		case 99: goto tr29;
	}
	goto tr26;
tr26:
#line 39 "parser.rl"
	{ MARK(mark, p); }
	goto st19;
st19:
	if ( ++p == pe )
		goto _out19;
case 19:
#line 466 "parser.c"
	switch( (*p) ) {
		case 13: goto tr27;
		case 67: goto st20;
		case 99: goto st20;
	}
	goto st19;
tr29:
#line 39 "parser.rl"
	{ MARK(mark, p); }
	goto st20;
st20:
	if ( ++p == pe )
		goto _out20;
case 20:
#line 481 "parser.c"
	switch( (*p) ) {
		case 13: goto tr27;
		case 67: goto st20;
		case 79: goto st21;
		case 99: goto st20;
		case 111: goto st21;
	}
	goto st19;
st21:
	if ( ++p == pe )
		goto _out21;
case 21:
	switch( (*p) ) {
		case 13: goto tr27;
		case 67: goto st20;
		case 78: goto st22;
		case 99: goto st20;
		case 110: goto st22;
	}
	goto st19;
st22:
	if ( ++p == pe )
		goto _out22;
case 22:
	switch( (*p) ) {
		case 13: goto tr27;
		case 67: goto st20;
		case 84: goto st23;
		case 99: goto st20;
		case 116: goto st23;
	}
	goto st19;
st23:
	if ( ++p == pe )
		goto _out23;
case 23:
	switch( (*p) ) {
		case 13: goto tr27;
		case 67: goto st20;
		case 69: goto st24;
		case 99: goto st20;
		case 101: goto st24;
	}
	goto st19;
st24:
	if ( ++p == pe )
		goto _out24;
case 24:
	switch( (*p) ) {
		case 13: goto tr27;
		case 67: goto st20;
		case 78: goto st25;
		case 99: goto st20;
		case 110: goto st25;
	}
	goto st19;
st25:
	if ( ++p == pe )
		goto _out25;
case 25:
	switch( (*p) ) {
		case 13: goto tr27;
		case 67: goto st20;
		case 84: goto st26;
		case 99: goto st20;
		case 116: goto st26;
	}
	goto st19;
st26:
	if ( ++p == pe )
		goto _out26;
case 26:
	switch( (*p) ) {
		case 13: goto tr27;
		case 45: goto st27;
		case 67: goto st20;
		case 99: goto st20;
	}
	goto st19;
st27:
	if ( ++p == pe )
		goto _out27;
case 27:
	switch( (*p) ) {
		case 13: goto tr27;
		case 67: goto st20;
		case 76: goto st28;
		case 84: goto st36;
		case 99: goto st20;
		case 108: goto st28;
		case 116: goto st36;
	}
	goto st19;
st28:
	if ( ++p == pe )
		goto _out28;
case 28:
	switch( (*p) ) {
		case 13: goto tr27;
		case 67: goto st20;
		case 69: goto st29;
		case 99: goto st20;
		case 101: goto st29;
	}
	goto st19;
st29:
	if ( ++p == pe )
		goto _out29;
case 29:
	switch( (*p) ) {
		case 13: goto tr27;
		case 67: goto st20;
		case 78: goto st30;
		case 99: goto st20;
		case 110: goto st30;
	}
	goto st19;
st30:
	if ( ++p == pe )
		goto _out30;
case 30:
	switch( (*p) ) {
		case 13: goto tr27;
		case 67: goto st20;
		case 71: goto st31;
		case 99: goto st20;
		case 103: goto st31;
	}
	goto st19;
st31:
	if ( ++p == pe )
		goto _out31;
case 31:
	switch( (*p) ) {
		case 13: goto tr27;
		case 67: goto st20;
		case 84: goto st32;
		case 99: goto st20;
		case 116: goto st32;
	}
	goto st19;
st32:
	if ( ++p == pe )
		goto _out32;
case 32:
	switch( (*p) ) {
		case 13: goto tr27;
		case 67: goto st20;
		case 72: goto st33;
		case 99: goto st20;
		case 104: goto st33;
	}
	goto st19;
st33:
	if ( ++p == pe )
		goto _out33;
case 33:
	switch( (*p) ) {
		case 13: goto tr27;
		case 58: goto st34;
		case 67: goto st20;
		case 99: goto st20;
	}
	goto st19;
st34:
	if ( ++p == pe )
		goto _out34;
case 34:
	switch( (*p) ) {
		case 13: goto tr27;
		case 32: goto st34;
		case 67: goto st20;
		case 99: goto st20;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr47;
	goto st19;
tr47:
#line 31 "parser.rl"
	{ MARK(mark, p); }
	goto st35;
tr68:
#line 39 "parser.rl"
	{ MARK(mark, p); }
#line 31 "parser.rl"
	{ MARK(mark, p); }
	goto st35;
st35:
	if ( ++p == pe )
		goto _out35;
case 35:
#line 673 "parser.c"
	switch( (*p) ) {
		case 13: goto st0;
		case 67: goto st20;
		case 99: goto st20;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st35;
	goto st19;
st36:
	if ( ++p == pe )
		goto _out36;
case 36:
	switch( (*p) ) {
		case 13: goto tr27;
		case 67: goto st20;
		case 89: goto st37;
		case 99: goto st20;
		case 121: goto st37;
	}
	goto st19;
st37:
	if ( ++p == pe )
		goto _out37;
case 37:
	switch( (*p) ) {
		case 13: goto tr27;
		case 67: goto st20;
		case 80: goto st38;
		case 99: goto st20;
		case 112: goto st38;
	}
	goto st19;
st38:
	if ( ++p == pe )
		goto _out38;
case 38:
	switch( (*p) ) {
		case 13: goto tr27;
		case 67: goto st20;
		case 69: goto st39;
		case 99: goto st20;
		case 101: goto st39;
	}
	goto st19;
st39:
	if ( ++p == pe )
		goto _out39;
case 39:
	switch( (*p) ) {
		case 13: goto tr27;
		case 58: goto st0;
		case 67: goto st20;
		case 99: goto st20;
	}
	goto st19;
st40:
	if ( ++p == pe )
		goto _out40;
case 40:
	switch( (*p) ) {
		case 33: goto st17;
		case 58: goto tr24;
		case 67: goto st40;
		case 79: goto st41;
		case 99: goto st40;
		case 111: goto st41;
		case 124: goto st17;
		case 126: goto st17;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st17;
		} else if ( (*p) >= 35 )
			goto st17;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st17;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st17;
		} else
			goto st17;
	} else
		goto st17;
	goto st0;
st41:
	if ( ++p == pe )
		goto _out41;
case 41:
	switch( (*p) ) {
		case 33: goto st17;
		case 58: goto tr24;
		case 67: goto st40;
		case 78: goto st42;
		case 99: goto st40;
		case 110: goto st42;
		case 124: goto st17;
		case 126: goto st17;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st17;
		} else if ( (*p) >= 35 )
			goto st17;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st17;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st17;
		} else
			goto st17;
	} else
		goto st17;
	goto st0;
st42:
	if ( ++p == pe )
		goto _out42;
case 42:
	switch( (*p) ) {
		case 33: goto st17;
		case 58: goto tr24;
		case 67: goto st40;
		case 84: goto st43;
		case 99: goto st40;
		case 116: goto st43;
		case 124: goto st17;
		case 126: goto st17;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st17;
		} else if ( (*p) >= 35 )
			goto st17;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st17;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st17;
		} else
			goto st17;
	} else
		goto st17;
	goto st0;
st43:
	if ( ++p == pe )
		goto _out43;
case 43:
	switch( (*p) ) {
		case 33: goto st17;
		case 58: goto tr24;
		case 67: goto st40;
		case 69: goto st44;
		case 99: goto st40;
		case 101: goto st44;
		case 124: goto st17;
		case 126: goto st17;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st17;
		} else if ( (*p) >= 35 )
			goto st17;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st17;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st17;
		} else
			goto st17;
	} else
		goto st17;
	goto st0;
st44:
	if ( ++p == pe )
		goto _out44;
case 44:
	switch( (*p) ) {
		case 33: goto st17;
		case 58: goto tr24;
		case 67: goto st40;
		case 78: goto st45;
		case 99: goto st40;
		case 110: goto st45;
		case 124: goto st17;
		case 126: goto st17;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st17;
		} else if ( (*p) >= 35 )
			goto st17;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st17;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st17;
		} else
			goto st17;
	} else
		goto st17;
	goto st0;
st45:
	if ( ++p == pe )
		goto _out45;
case 45:
	switch( (*p) ) {
		case 33: goto st17;
		case 58: goto tr24;
		case 67: goto st40;
		case 84: goto st46;
		case 99: goto st40;
		case 116: goto st46;
		case 124: goto st17;
		case 126: goto st17;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st17;
		} else if ( (*p) >= 35 )
			goto st17;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st17;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st17;
		} else
			goto st17;
	} else
		goto st17;
	goto st0;
st46:
	if ( ++p == pe )
		goto _out46;
case 46:
	switch( (*p) ) {
		case 33: goto st17;
		case 45: goto st47;
		case 46: goto st17;
		case 58: goto tr24;
		case 67: goto st40;
		case 99: goto st40;
		case 124: goto st17;
		case 126: goto st17;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st17;
		} else if ( (*p) >= 35 )
			goto st17;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st17;
		} else if ( (*p) >= 65 )
			goto st17;
	} else
		goto st17;
	goto st0;
st47:
	if ( ++p == pe )
		goto _out47;
case 47:
	switch( (*p) ) {
		case 33: goto st17;
		case 58: goto tr24;
		case 67: goto st40;
		case 76: goto st48;
		case 84: goto st55;
		case 99: goto st40;
		case 108: goto st48;
		case 116: goto st55;
		case 124: goto st17;
		case 126: goto st17;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st17;
		} else if ( (*p) >= 35 )
			goto st17;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st17;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st17;
		} else
			goto st17;
	} else
		goto st17;
	goto st0;
st48:
	if ( ++p == pe )
		goto _out48;
case 48:
	switch( (*p) ) {
		case 33: goto st17;
		case 58: goto tr24;
		case 67: goto st40;
		case 69: goto st49;
		case 99: goto st40;
		case 101: goto st49;
		case 124: goto st17;
		case 126: goto st17;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st17;
		} else if ( (*p) >= 35 )
			goto st17;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st17;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st17;
		} else
			goto st17;
	} else
		goto st17;
	goto st0;
st49:
	if ( ++p == pe )
		goto _out49;
case 49:
	switch( (*p) ) {
		case 33: goto st17;
		case 58: goto tr24;
		case 67: goto st40;
		case 78: goto st50;
		case 99: goto st40;
		case 110: goto st50;
		case 124: goto st17;
		case 126: goto st17;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st17;
		} else if ( (*p) >= 35 )
			goto st17;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st17;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st17;
		} else
			goto st17;
	} else
		goto st17;
	goto st0;
st50:
	if ( ++p == pe )
		goto _out50;
case 50:
	switch( (*p) ) {
		case 33: goto st17;
		case 58: goto tr24;
		case 67: goto st40;
		case 71: goto st51;
		case 99: goto st40;
		case 103: goto st51;
		case 124: goto st17;
		case 126: goto st17;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st17;
		} else if ( (*p) >= 35 )
			goto st17;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st17;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st17;
		} else
			goto st17;
	} else
		goto st17;
	goto st0;
st51:
	if ( ++p == pe )
		goto _out51;
case 51:
	switch( (*p) ) {
		case 33: goto st17;
		case 58: goto tr24;
		case 67: goto st40;
		case 84: goto st52;
		case 99: goto st40;
		case 116: goto st52;
		case 124: goto st17;
		case 126: goto st17;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st17;
		} else if ( (*p) >= 35 )
			goto st17;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st17;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st17;
		} else
			goto st17;
	} else
		goto st17;
	goto st0;
st52:
	if ( ++p == pe )
		goto _out52;
case 52:
	switch( (*p) ) {
		case 33: goto st17;
		case 58: goto tr24;
		case 67: goto st40;
		case 72: goto st53;
		case 99: goto st40;
		case 104: goto st53;
		case 124: goto st17;
		case 126: goto st17;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st17;
		} else if ( (*p) >= 35 )
			goto st17;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st17;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st17;
		} else
			goto st17;
	} else
		goto st17;
	goto st0;
st53:
	if ( ++p == pe )
		goto _out53;
case 53:
	switch( (*p) ) {
		case 33: goto st17;
		case 58: goto tr66;
		case 67: goto st40;
		case 99: goto st40;
		case 124: goto st17;
		case 126: goto st17;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st17;
		} else if ( (*p) >= 35 )
			goto st17;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st17;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st17;
		} else
			goto st17;
	} else
		goto st17;
	goto st0;
tr66:
#line 34 "parser.rl"
	{
    parser->field_len = LEN(field_start, p);
    if (parser->field_len > MAX_FIELD_NAME) { PARSER_ERROR; goto _out54; }
  }
	goto st54;
tr67:
#line 39 "parser.rl"
	{ MARK(mark, p); }
	goto st54;
st54:
	if ( ++p == pe )
		goto _out54;
case 54:
#line 1189 "parser.c"
	switch( (*p) ) {
		case 13: goto tr27;
		case 32: goto tr67;
		case 67: goto tr29;
		case 99: goto tr29;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr68;
	goto tr26;
st55:
	if ( ++p == pe )
		goto _out55;
case 55:
	switch( (*p) ) {
		case 33: goto st17;
		case 58: goto tr24;
		case 67: goto st40;
		case 89: goto st56;
		case 99: goto st40;
		case 121: goto st56;
		case 124: goto st17;
		case 126: goto st17;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st17;
		} else if ( (*p) >= 35 )
			goto st17;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st17;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st17;
		} else
			goto st17;
	} else
		goto st17;
	goto st0;
st56:
	if ( ++p == pe )
		goto _out56;
case 56:
	switch( (*p) ) {
		case 33: goto st17;
		case 58: goto tr24;
		case 67: goto st40;
		case 80: goto st57;
		case 99: goto st40;
		case 112: goto st57;
		case 124: goto st17;
		case 126: goto st17;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st17;
		} else if ( (*p) >= 35 )
			goto st17;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st17;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st17;
		} else
			goto st17;
	} else
		goto st17;
	goto st0;
st57:
	if ( ++p == pe )
		goto _out57;
case 57:
	switch( (*p) ) {
		case 33: goto st17;
		case 58: goto tr24;
		case 67: goto st40;
		case 69: goto st58;
		case 99: goto st40;
		case 101: goto st58;
		case 124: goto st17;
		case 126: goto st17;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st17;
		} else if ( (*p) >= 35 )
			goto st17;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st17;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st17;
		} else
			goto st17;
	} else
		goto st17;
	goto st0;
st58:
	if ( ++p == pe )
		goto _out58;
case 58:
	switch( (*p) ) {
		case 33: goto st17;
		case 67: goto st40;
		case 99: goto st40;
		case 124: goto st17;
		case 126: goto st17;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st17;
		} else if ( (*p) >= 35 )
			goto st17;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st17;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st17;
		} else
			goto st17;
	} else
		goto st17;
	goto st0;
tr21:
#line 33 "parser.rl"
	{ MARK(field_start, p); }
	goto st59;
st59:
	if ( ++p == pe )
		goto _out59;
case 59:
#line 1332 "parser.c"
	switch( (*p) ) {
		case 33: goto st17;
		case 58: goto tr24;
		case 67: goto st40;
		case 79: goto st60;
		case 99: goto st40;
		case 111: goto st60;
		case 124: goto st17;
		case 126: goto st17;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st17;
		} else if ( (*p) >= 35 )
			goto st17;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st17;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st17;
		} else
			goto st17;
	} else
		goto st17;
	goto st0;
st60:
	if ( ++p == pe )
		goto _out60;
case 60:
	switch( (*p) ) {
		case 33: goto st17;
		case 58: goto tr24;
		case 67: goto st40;
		case 78: goto st61;
		case 99: goto st40;
		case 110: goto st61;
		case 124: goto st17;
		case 126: goto st17;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st17;
		} else if ( (*p) >= 35 )
			goto st17;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st17;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st17;
		} else
			goto st17;
	} else
		goto st17;
	goto st0;
st61:
	if ( ++p == pe )
		goto _out61;
case 61:
	switch( (*p) ) {
		case 33: goto st17;
		case 58: goto tr24;
		case 67: goto st40;
		case 84: goto st62;
		case 99: goto st40;
		case 116: goto st62;
		case 124: goto st17;
		case 126: goto st17;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st17;
		} else if ( (*p) >= 35 )
			goto st17;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st17;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st17;
		} else
			goto st17;
	} else
		goto st17;
	goto st0;
st62:
	if ( ++p == pe )
		goto _out62;
case 62:
	switch( (*p) ) {
		case 33: goto st17;
		case 58: goto tr24;
		case 67: goto st40;
		case 69: goto st63;
		case 99: goto st40;
		case 101: goto st63;
		case 124: goto st17;
		case 126: goto st17;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st17;
		} else if ( (*p) >= 35 )
			goto st17;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st17;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st17;
		} else
			goto st17;
	} else
		goto st17;
	goto st0;
st63:
	if ( ++p == pe )
		goto _out63;
case 63:
	switch( (*p) ) {
		case 33: goto st17;
		case 58: goto tr24;
		case 67: goto st40;
		case 78: goto st64;
		case 99: goto st40;
		case 110: goto st64;
		case 124: goto st17;
		case 126: goto st17;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st17;
		} else if ( (*p) >= 35 )
			goto st17;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st17;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st17;
		} else
			goto st17;
	} else
		goto st17;
	goto st0;
st64:
	if ( ++p == pe )
		goto _out64;
case 64:
	switch( (*p) ) {
		case 33: goto st17;
		case 58: goto tr24;
		case 67: goto st40;
		case 84: goto st65;
		case 99: goto st40;
		case 116: goto st65;
		case 124: goto st17;
		case 126: goto st17;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st17;
		} else if ( (*p) >= 35 )
			goto st17;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st17;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st17;
		} else
			goto st17;
	} else
		goto st17;
	goto st0;
st65:
	if ( ++p == pe )
		goto _out65;
case 65:
	switch( (*p) ) {
		case 33: goto st17;
		case 45: goto st66;
		case 46: goto st17;
		case 58: goto tr24;
		case 67: goto st40;
		case 99: goto st40;
		case 124: goto st17;
		case 126: goto st17;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st17;
		} else if ( (*p) >= 35 )
			goto st17;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st17;
		} else if ( (*p) >= 65 )
			goto st17;
	} else
		goto st17;
	goto st0;
st66:
	if ( ++p == pe )
		goto _out66;
case 66:
	switch( (*p) ) {
		case 33: goto st17;
		case 58: goto tr24;
		case 67: goto st40;
		case 76: goto st67;
		case 84: goto st75;
		case 99: goto st40;
		case 108: goto st67;
		case 116: goto st75;
		case 124: goto st17;
		case 126: goto st17;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st17;
		} else if ( (*p) >= 35 )
			goto st17;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st17;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st17;
		} else
			goto st17;
	} else
		goto st17;
	goto st0;
st67:
	if ( ++p == pe )
		goto _out67;
case 67:
	switch( (*p) ) {
		case 33: goto st17;
		case 58: goto tr24;
		case 67: goto st40;
		case 69: goto st68;
		case 99: goto st40;
		case 101: goto st68;
		case 124: goto st17;
		case 126: goto st17;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st17;
		} else if ( (*p) >= 35 )
			goto st17;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st17;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st17;
		} else
			goto st17;
	} else
		goto st17;
	goto st0;
st68:
	if ( ++p == pe )
		goto _out68;
case 68:
	switch( (*p) ) {
		case 33: goto st17;
		case 58: goto tr24;
		case 67: goto st40;
		case 78: goto st69;
		case 99: goto st40;
		case 110: goto st69;
		case 124: goto st17;
		case 126: goto st17;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st17;
		} else if ( (*p) >= 35 )
			goto st17;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st17;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st17;
		} else
			goto st17;
	} else
		goto st17;
	goto st0;
st69:
	if ( ++p == pe )
		goto _out69;
case 69:
	switch( (*p) ) {
		case 33: goto st17;
		case 58: goto tr24;
		case 67: goto st40;
		case 71: goto st70;
		case 99: goto st40;
		case 103: goto st70;
		case 124: goto st17;
		case 126: goto st17;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st17;
		} else if ( (*p) >= 35 )
			goto st17;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st17;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st17;
		} else
			goto st17;
	} else
		goto st17;
	goto st0;
st70:
	if ( ++p == pe )
		goto _out70;
case 70:
	switch( (*p) ) {
		case 33: goto st17;
		case 58: goto tr24;
		case 67: goto st40;
		case 84: goto st71;
		case 99: goto st40;
		case 116: goto st71;
		case 124: goto st17;
		case 126: goto st17;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st17;
		} else if ( (*p) >= 35 )
			goto st17;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st17;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st17;
		} else
			goto st17;
	} else
		goto st17;
	goto st0;
st71:
	if ( ++p == pe )
		goto _out71;
case 71:
	switch( (*p) ) {
		case 33: goto st17;
		case 58: goto tr24;
		case 67: goto st40;
		case 72: goto st72;
		case 99: goto st40;
		case 104: goto st72;
		case 124: goto st17;
		case 126: goto st17;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st17;
		} else if ( (*p) >= 35 )
			goto st17;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st17;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st17;
		} else
			goto st17;
	} else
		goto st17;
	goto st0;
st72:
	if ( ++p == pe )
		goto _out72;
case 72:
	switch( (*p) ) {
		case 33: goto st17;
		case 58: goto tr86;
		case 67: goto st40;
		case 99: goto st40;
		case 124: goto st17;
		case 126: goto st17;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st17;
		} else if ( (*p) >= 35 )
			goto st17;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st17;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st17;
		} else
			goto st17;
	} else
		goto st17;
	goto st0;
tr86:
#line 34 "parser.rl"
	{
    parser->field_len = LEN(field_start, p);
    if (parser->field_len > MAX_FIELD_NAME) { PARSER_ERROR; goto _out73; }
  }
	goto st73;
tr87:
#line 39 "parser.rl"
	{ MARK(mark, p); }
	goto st73;
st73:
	if ( ++p == pe )
		goto _out73;
case 73:
#line 1789 "parser.c"
	switch( (*p) ) {
		case 13: goto tr27;
		case 32: goto tr87;
		case 67: goto tr29;
		case 99: goto tr29;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr88;
	goto tr26;
tr88:
#line 31 "parser.rl"
	{ MARK(mark, p); }
#line 39 "parser.rl"
	{ MARK(mark, p); }
	goto st74;
st74:
	if ( ++p == pe )
		goto _out74;
case 74:
#line 1809 "parser.c"
	switch( (*p) ) {
		case 13: goto tr89;
		case 67: goto st20;
		case 99: goto st20;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st74;
	goto st19;
st75:
	if ( ++p == pe )
		goto _out75;
case 75:
	switch( (*p) ) {
		case 33: goto st17;
		case 58: goto tr24;
		case 67: goto st40;
		case 89: goto st76;
		case 99: goto st40;
		case 121: goto st76;
		case 124: goto st17;
		case 126: goto st17;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st17;
		} else if ( (*p) >= 35 )
			goto st17;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st17;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st17;
		} else
			goto st17;
	} else
		goto st17;
	goto st0;
st76:
	if ( ++p == pe )
		goto _out76;
case 76:
	switch( (*p) ) {
		case 33: goto st17;
		case 58: goto tr24;
		case 67: goto st40;
		case 80: goto st77;
		case 99: goto st40;
		case 112: goto st77;
		case 124: goto st17;
		case 126: goto st17;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st17;
		} else if ( (*p) >= 35 )
			goto st17;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st17;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st17;
		} else
			goto st17;
	} else
		goto st17;
	goto st0;
st77:
	if ( ++p == pe )
		goto _out77;
case 77:
	switch( (*p) ) {
		case 33: goto st17;
		case 58: goto tr24;
		case 67: goto st40;
		case 69: goto st78;
		case 99: goto st40;
		case 101: goto st78;
		case 124: goto st17;
		case 126: goto st17;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st17;
		} else if ( (*p) >= 35 )
			goto st17;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st17;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st17;
		} else
			goto st17;
	} else
		goto st17;
	goto st0;
st78:
	if ( ++p == pe )
		goto _out78;
case 78:
	switch( (*p) ) {
		case 33: goto st17;
		case 58: goto st79;
		case 67: goto st40;
		case 99: goto st40;
		case 124: goto st17;
		case 126: goto st17;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st17;
		} else if ( (*p) >= 35 )
			goto st17;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st17;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st17;
		} else
			goto st17;
	} else
		goto st17;
	goto st0;
tr97:
#line 31 "parser.rl"
	{ MARK(mark, p); }
	goto st79;
st79:
	if ( ++p == pe )
		goto _out79;
case 79:
#line 1952 "parser.c"
	switch( (*p) ) {
		case 13: goto tr96;
		case 32: goto tr97;
	}
	goto tr95;
tr95:
#line 31 "parser.rl"
	{ MARK(mark, p); }
	goto st80;
st80:
	if ( ++p == pe )
		goto _out80;
case 80:
#line 1966 "parser.c"
	if ( (*p) == 13 )
		goto tr96;
	goto st80;
tr5:
#line 31 "parser.rl"
	{ MARK(mark, p); }
	goto st81;
st81:
	if ( ++p == pe )
		goto _out81;
case 81:
#line 1978 "parser.c"
	switch( (*p) ) {
		case 43: goto st81;
		case 58: goto st82;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st81;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st81;
		} else if ( (*p) >= 65 )
			goto st81;
	} else
		goto st81;
	goto st0;
tr7:
#line 31 "parser.rl"
	{ MARK(mark, p); }
	goto st82;
st82:
	if ( ++p == pe )
		goto _out82;
case 82:
#line 2003 "parser.c"
	switch( (*p) ) {
		case 32: goto tr8;
		case 37: goto st83;
		case 60: goto st0;
		case 62: goto st0;
		case 127: goto st0;
	}
	if ( (*p) > 31 ) {
		if ( 34 <= (*p) && (*p) <= 35 )
			goto st0;
	} else if ( (*p) >= 0 )
		goto st0;
	goto st82;
st83:
	if ( ++p == pe )
		goto _out83;
case 83:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st84;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st84;
	} else
		goto st84;
	goto st0;
st84:
	if ( ++p == pe )
		goto _out84;
case 84:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st82;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st82;
	} else
		goto st82;
	goto st0;
tr6:
#line 31 "parser.rl"
	{ MARK(mark, p); }
	goto st85;
st85:
	if ( ++p == pe )
		goto _out85;
case 85:
#line 2051 "parser.c"
	switch( (*p) ) {
		case 32: goto tr104;
		case 37: goto st86;
		case 59: goto tr106;
		case 60: goto st0;
		case 62: goto st0;
		case 63: goto tr107;
		case 127: goto st0;
	}
	if ( (*p) > 31 ) {
		if ( 34 <= (*p) && (*p) <= 35 )
			goto st0;
	} else if ( (*p) >= 0 )
		goto st0;
	goto st85;
st86:
	if ( ++p == pe )
		goto _out86;
case 86:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st87;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st87;
	} else
		goto st87;
	goto st0;
st87:
	if ( ++p == pe )
		goto _out87;
case 87:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st85;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st85;
	} else
		goto st85;
	goto st0;
tr106:
#line 90 "parser.rl"
	{
    size_t len = LEN(mark, p);
    
    if (len > MAX_REQUEST_PATH) { PARSER_ERROR; goto _out88; }
    if (parser->request_path != NULL)
      parser->request_path(parser->data, PTR_TO(mark), len);
  }
	goto st88;
st88:
	if ( ++p == pe )
		goto _out88;
case 88:
#line 2107 "parser.c"
	switch( (*p) ) {
		case 32: goto tr8;
		case 37: goto st89;
		case 60: goto st0;
		case 62: goto st0;
		case 63: goto st91;
		case 127: goto st0;
	}
	if ( (*p) > 31 ) {
		if ( 34 <= (*p) && (*p) <= 35 )
			goto st0;
	} else if ( (*p) >= 0 )
		goto st0;
	goto st88;
st89:
	if ( ++p == pe )
		goto _out89;
case 89:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st90;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st90;
	} else
		goto st90;
	goto st0;
st90:
	if ( ++p == pe )
		goto _out90;
case 90:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st88;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st88;
	} else
		goto st88;
	goto st0;
tr107:
#line 90 "parser.rl"
	{
    size_t len = LEN(mark, p);
    
    if (len > MAX_REQUEST_PATH) { PARSER_ERROR; goto _out91; }
    if (parser->request_path != NULL)
      parser->request_path(parser->data, PTR_TO(mark), len);
  }
	goto st91;
st91:
	if ( ++p == pe )
		goto _out91;
case 91:
#line 2162 "parser.c"
	switch( (*p) ) {
		case 32: goto tr114;
		case 37: goto tr115;
		case 60: goto st0;
		case 62: goto st0;
		case 127: goto st0;
	}
	if ( (*p) > 31 ) {
		if ( 34 <= (*p) && (*p) <= 35 )
			goto st0;
	} else if ( (*p) >= 0 )
		goto st0;
	goto tr113;
tr113:
#line 76 "parser.rl"
	{ MARK(query_start, p); }
	goto st92;
st92:
	if ( ++p == pe )
		goto _out92;
case 92:
#line 2184 "parser.c"
	switch( (*p) ) {
		case 32: goto tr117;
		case 37: goto st93;
		case 60: goto st0;
		case 62: goto st0;
		case 127: goto st0;
	}
	if ( (*p) > 31 ) {
		if ( 34 <= (*p) && (*p) <= 35 )
			goto st0;
	} else if ( (*p) >= 0 )
		goto st0;
	goto st92;
tr115:
#line 76 "parser.rl"
	{ MARK(query_start, p); }
	goto st93;
st93:
	if ( ++p == pe )
		goto _out93;
case 93:
#line 2206 "parser.c"
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st94;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st94;
	} else
		goto st94;
	goto st0;
st94:
	if ( ++p == pe )
		goto _out94;
case 94:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st92;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st92;
	} else
		goto st92;
	goto st0;
st95:
	if ( ++p == pe )
		goto _out95;
case 95:
	switch( (*p) ) {
		case 32: goto tr2;
		case 36: goto st96;
		case 95: goto st96;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st96;
	} else if ( (*p) > 57 ) {
		if ( 65 <= (*p) && (*p) <= 90 )
			goto st96;
	} else
		goto st96;
	goto st0;
st96:
	if ( ++p == pe )
		goto _out96;
case 96:
	switch( (*p) ) {
		case 32: goto tr2;
		case 36: goto st97;
		case 95: goto st97;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st97;
	} else if ( (*p) > 57 ) {
		if ( 65 <= (*p) && (*p) <= 90 )
			goto st97;
	} else
		goto st97;
	goto st0;
st97:
	if ( ++p == pe )
		goto _out97;
case 97:
	switch( (*p) ) {
		case 32: goto tr2;
		case 36: goto st98;
		case 95: goto st98;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st98;
	} else if ( (*p) > 57 ) {
		if ( 65 <= (*p) && (*p) <= 90 )
			goto st98;
	} else
		goto st98;
	goto st0;
st98:
	if ( ++p == pe )
		goto _out98;
case 98:
	switch( (*p) ) {
		case 32: goto tr2;
		case 36: goto st99;
		case 95: goto st99;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st99;
	} else if ( (*p) > 57 ) {
		if ( 65 <= (*p) && (*p) <= 90 )
			goto st99;
	} else
		goto st99;
	goto st0;
st99:
	if ( ++p == pe )
		goto _out99;
case 99:
	switch( (*p) ) {
		case 32: goto tr2;
		case 36: goto st100;
		case 95: goto st100;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st100;
	} else if ( (*p) > 57 ) {
		if ( 65 <= (*p) && (*p) <= 90 )
			goto st100;
	} else
		goto st100;
	goto st0;
st100:
	if ( ++p == pe )
		goto _out100;
case 100:
	switch( (*p) ) {
		case 32: goto tr2;
		case 36: goto st101;
		case 95: goto st101;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st101;
	} else if ( (*p) > 57 ) {
		if ( 65 <= (*p) && (*p) <= 90 )
			goto st101;
	} else
		goto st101;
	goto st0;
st101:
	if ( ++p == pe )
		goto _out101;
case 101:
	switch( (*p) ) {
		case 32: goto tr2;
		case 36: goto st102;
		case 95: goto st102;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st102;
	} else if ( (*p) > 57 ) {
		if ( 65 <= (*p) && (*p) <= 90 )
			goto st102;
	} else
		goto st102;
	goto st0;
st102:
	if ( ++p == pe )
		goto _out102;
case 102:
	switch( (*p) ) {
		case 32: goto tr2;
		case 36: goto st103;
		case 95: goto st103;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st103;
	} else if ( (*p) > 57 ) {
		if ( 65 <= (*p) && (*p) <= 90 )
			goto st103;
	} else
		goto st103;
	goto st0;
st103:
	if ( ++p == pe )
		goto _out103;
case 103:
	switch( (*p) ) {
		case 32: goto tr2;
		case 36: goto st104;
		case 95: goto st104;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st104;
	} else if ( (*p) > 57 ) {
		if ( 65 <= (*p) && (*p) <= 90 )
			goto st104;
	} else
		goto st104;
	goto st0;
st104:
	if ( ++p == pe )
		goto _out104;
case 104:
	switch( (*p) ) {
		case 32: goto tr2;
		case 36: goto st105;
		case 95: goto st105;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st105;
	} else if ( (*p) > 57 ) {
		if ( 65 <= (*p) && (*p) <= 90 )
			goto st105;
	} else
		goto st105;
	goto st0;
st105:
	if ( ++p == pe )
		goto _out105;
case 105:
	switch( (*p) ) {
		case 32: goto tr2;
		case 36: goto st106;
		case 95: goto st106;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st106;
	} else if ( (*p) > 57 ) {
		if ( 65 <= (*p) && (*p) <= 90 )
			goto st106;
	} else
		goto st106;
	goto st0;
st106:
	if ( ++p == pe )
		goto _out106;
case 106:
	switch( (*p) ) {
		case 32: goto tr2;
		case 36: goto st107;
		case 95: goto st107;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st107;
	} else if ( (*p) > 57 ) {
		if ( 65 <= (*p) && (*p) <= 90 )
			goto st107;
	} else
		goto st107;
	goto st0;
st107:
	if ( ++p == pe )
		goto _out107;
case 107:
	switch( (*p) ) {
		case 32: goto tr2;
		case 36: goto st108;
		case 95: goto st108;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st108;
	} else if ( (*p) > 57 ) {
		if ( 65 <= (*p) && (*p) <= 90 )
			goto st108;
	} else
		goto st108;
	goto st0;
st108:
	if ( ++p == pe )
		goto _out108;
case 108:
	switch( (*p) ) {
		case 32: goto tr2;
		case 36: goto st109;
		case 95: goto st109;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st109;
	} else if ( (*p) > 57 ) {
		if ( 65 <= (*p) && (*p) <= 90 )
			goto st109;
	} else
		goto st109;
	goto st0;
st109:
	if ( ++p == pe )
		goto _out109;
case 109:
	switch( (*p) ) {
		case 32: goto tr2;
		case 36: goto st110;
		case 95: goto st110;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st110;
	} else if ( (*p) > 57 ) {
		if ( 65 <= (*p) && (*p) <= 90 )
			goto st110;
	} else
		goto st110;
	goto st0;
st110:
	if ( ++p == pe )
		goto _out110;
case 110:
	switch( (*p) ) {
		case 32: goto tr2;
		case 36: goto st111;
		case 95: goto st111;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st111;
	} else if ( (*p) > 57 ) {
		if ( 65 <= (*p) && (*p) <= 90 )
			goto st111;
	} else
		goto st111;
	goto st0;
st111:
	if ( ++p == pe )
		goto _out111;
case 111:
	switch( (*p) ) {
		case 32: goto tr2;
		case 36: goto st112;
		case 95: goto st112;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st112;
	} else if ( (*p) > 57 ) {
		if ( 65 <= (*p) && (*p) <= 90 )
			goto st112;
	} else
		goto st112;
	goto st0;
st112:
	if ( ++p == pe )
		goto _out112;
case 112:
	switch( (*p) ) {
		case 32: goto tr2;
		case 36: goto st113;
		case 95: goto st113;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st113;
	} else if ( (*p) > 57 ) {
		if ( 65 <= (*p) && (*p) <= 90 )
			goto st113;
	} else
		goto st113;
	goto st0;
st113:
	if ( ++p == pe )
		goto _out113;
case 113:
	if ( (*p) == 32 )
		goto tr2;
	goto st0;
	}
	_out0: cs = 0; goto _out; 
	_out2: cs = 2; goto _out; 
	_out3: cs = 3; goto _out; 
	_out4: cs = 4; goto _out; 
	_out5: cs = 5; goto _out; 
	_out6: cs = 6; goto _out; 
	_out7: cs = 7; goto _out; 
	_out8: cs = 8; goto _out; 
	_out9: cs = 9; goto _out; 
	_out10: cs = 10; goto _out; 
	_out11: cs = 11; goto _out; 
	_out12: cs = 12; goto _out; 
	_out13: cs = 13; goto _out; 
	_out14: cs = 14; goto _out; 
	_out15: cs = 15; goto _out; 
	_out16: cs = 16; goto _out; 
	_out114: cs = 114; goto _out; 
	_out17: cs = 17; goto _out; 
	_out18: cs = 18; goto _out; 
	_out19: cs = 19; goto _out; 
	_out20: cs = 20; goto _out; 
	_out21: cs = 21; goto _out; 
	_out22: cs = 22; goto _out; 
	_out23: cs = 23; goto _out; 
	_out24: cs = 24; goto _out; 
	_out25: cs = 25; goto _out; 
	_out26: cs = 26; goto _out; 
	_out27: cs = 27; goto _out; 
	_out28: cs = 28; goto _out; 
	_out29: cs = 29; goto _out; 
	_out30: cs = 30; goto _out; 
	_out31: cs = 31; goto _out; 
	_out32: cs = 32; goto _out; 
	_out33: cs = 33; goto _out; 
	_out34: cs = 34; goto _out; 
	_out35: cs = 35; goto _out; 
	_out36: cs = 36; goto _out; 
	_out37: cs = 37; goto _out; 
	_out38: cs = 38; goto _out; 
	_out39: cs = 39; goto _out; 
	_out40: cs = 40; goto _out; 
	_out41: cs = 41; goto _out; 
	_out42: cs = 42; goto _out; 
	_out43: cs = 43; goto _out; 
	_out44: cs = 44; goto _out; 
	_out45: cs = 45; goto _out; 
	_out46: cs = 46; goto _out; 
	_out47: cs = 47; goto _out; 
	_out48: cs = 48; goto _out; 
	_out49: cs = 49; goto _out; 
	_out50: cs = 50; goto _out; 
	_out51: cs = 51; goto _out; 
	_out52: cs = 52; goto _out; 
	_out53: cs = 53; goto _out; 
	_out54: cs = 54; goto _out; 
	_out55: cs = 55; goto _out; 
	_out56: cs = 56; goto _out; 
	_out57: cs = 57; goto _out; 
	_out58: cs = 58; goto _out; 
	_out59: cs = 59; goto _out; 
	_out60: cs = 60; goto _out; 
	_out61: cs = 61; goto _out; 
	_out62: cs = 62; goto _out; 
	_out63: cs = 63; goto _out; 
	_out64: cs = 64; goto _out; 
	_out65: cs = 65; goto _out; 
	_out66: cs = 66; goto _out; 
	_out67: cs = 67; goto _out; 
	_out68: cs = 68; goto _out; 
	_out69: cs = 69; goto _out; 
	_out70: cs = 70; goto _out; 
	_out71: cs = 71; goto _out; 
	_out72: cs = 72; goto _out; 
	_out73: cs = 73; goto _out; 
	_out74: cs = 74; goto _out; 
	_out75: cs = 75; goto _out; 
	_out76: cs = 76; goto _out; 
	_out77: cs = 77; goto _out; 
	_out78: cs = 78; goto _out; 
	_out79: cs = 79; goto _out; 
	_out80: cs = 80; goto _out; 
	_out81: cs = 81; goto _out; 
	_out82: cs = 82; goto _out; 
	_out83: cs = 83; goto _out; 
	_out84: cs = 84; goto _out; 
	_out85: cs = 85; goto _out; 
	_out86: cs = 86; goto _out; 
	_out87: cs = 87; goto _out; 
	_out88: cs = 88; goto _out; 
	_out89: cs = 89; goto _out; 
	_out90: cs = 90; goto _out; 
	_out91: cs = 91; goto _out; 
	_out92: cs = 92; goto _out; 
	_out93: cs = 93; goto _out; 
	_out94: cs = 94; goto _out; 
	_out95: cs = 95; goto _out; 
	_out96: cs = 96; goto _out; 
	_out97: cs = 97; goto _out; 
	_out98: cs = 98; goto _out; 
	_out99: cs = 99; goto _out; 
	_out100: cs = 100; goto _out; 
	_out101: cs = 101; goto _out; 
	_out102: cs = 102; goto _out; 
	_out103: cs = 103; goto _out; 
	_out104: cs = 104; goto _out; 
	_out105: cs = 105; goto _out; 
	_out106: cs = 106; goto _out; 
	_out107: cs = 107; goto _out; 
	_out108: cs = 108; goto _out; 
	_out109: cs = 109; goto _out; 
	_out110: cs = 110; goto _out; 
	_out111: cs = 111; goto _out; 
	_out112: cs = 112; goto _out; 
	_out113: cs = 113; goto _out; 

	_out: {}
	}
#line 201 "parser.rl"

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
    
#line 2693 "parser.c"
#line 215 "parser.rl"
    parser->nread++;
  }

  return(parser->nread);
}

int http_parser_finish(http_parser *parser)
{
  int cs = parser->cs;

  
#line 2706 "parser.c"
#line 226 "parser.rl"

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
  return parser->cs == http_parser_error;
}

int http_parser_is_finished(http_parser *parser) {
  return parser->cs == http_parser_first_final;
}
