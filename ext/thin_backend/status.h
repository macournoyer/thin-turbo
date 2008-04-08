#ifndef _STATUS_H_
#define _STATUS_H_

static char *status_lines[] = {
  "100 Continue", 
  "101 Switching Protocols", 
  "200 OK", 
  "201 Created", 
  "202 Accepted", 
  "203 Non-Authoritative Information", 
  "204 No Content", 
  "205 Reset Content", 
  "206 Partial Content", 
  "300 Multiple Choices", 
  "301 Moved Permanently", 
  "302 Moved Temporarily", 
  "303 See Other", 
  "304 Not Modified", 
  "305 Use Proxy", 
  "400 Bad Request", 
  "401 Unauthorized", 
  "402 Payment Required", 
  "403 Forbidden", 
  "404 Not Found", 
  "405 Method Not Allowed", 
  "406 Not Acceptable", 
  "407 Proxy Authentication Required", 
  "408 Request Time-out", 
  "409 Conflict", 
  "410 Gone", 
  "411 Length Required", 
  "412 Precondition Failed", 
  "413 Request Entity Too Large", 
  "414 Request-URI Too Large", 
  "415 Unsupported Media Type", 
  "500 Internal Server Error", 
  "501 Not Implemented", 
  "502 Bad Gateway", 
  "503 Service Unavailable", 
  "504 Gateway Time-out", 
  "505 HTTP Version not supported"
};

#define def_status_code(index, code) case code: i = index; break

static inline char * get_status_line(int code)
{
  int i = 2; /* default 200 OK */

  switch (code) {
    def_status_code(0,  100);
    def_status_code(1,  101);
    def_status_code(2,  200);
    def_status_code(3,  201);
    def_status_code(4,  202);
    def_status_code(5,  203);
    def_status_code(6,  204);
    def_status_code(7,  205);
    def_status_code(8,  206);
    def_status_code(9,  300);
    def_status_code(10, 301);
    def_status_code(11, 302);
    def_status_code(12, 303);
    def_status_code(13, 304);
    def_status_code(14, 305);
    def_status_code(15, 400);
    def_status_code(16, 401);
    def_status_code(17, 402);
    def_status_code(18, 403);
    def_status_code(19, 404);
    def_status_code(20, 405);
    def_status_code(21, 406);
    def_status_code(22, 407);
    def_status_code(23, 408);
    def_status_code(24, 409);
    def_status_code(25, 410);
    def_status_code(26, 411);
    def_status_code(27, 412);
    def_status_code(28, 413);
    def_status_code(29, 414);
    def_status_code(30, 415);
    def_status_code(31, 500);
    def_status_code(32, 501);
    def_status_code(33, 502);
    def_status_code(34, 503);
    def_status_code(35, 504);
    def_status_code(36, 505);
  };
  
  return status_lines[i];
}

#endif /* _STATUS_H_ */
