#include "thin.h"

void Init_thin_backend()
{  
  /* Initialize internal stuff */
  backend_define();
  input_define();
  connections_init();
  parser_callbacks_init();
}
