#include "thin.h"

void Init_backend()
{  
  /* Initialize internal stuff */
  backend_define();
  input_define();
  connections_init();
  parser_callbacks_init();
}