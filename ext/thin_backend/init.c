#include "thin.h"

void Init_thin_backend()
{  
  /* Initialize internal stuff */
  thin_backend_define();
  thin_input_define();
  thin_connections_init();
  thin_parser_callbacks_init();
}