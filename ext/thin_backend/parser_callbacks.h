#ifndef _PARSER_CALLBACKS_H_
#define _PARSER_CALLBACKS_H_

#include "ruby.h"
#include "connection.h"

void thin_parser_callbacks_init(VALUE module);
void thin_setup_parser_callbacks(thin_connection *connection);

#endif /* _PARSER_CALLBACKS_H_ */
