#include "thin.h"

#define EV_STANDALONE 1
#include <ev.c>

void Init_thin_backend()
{  
  /* Initialize internal stuff */
  backend_define();
  connections_init();
  parser_callbacks_init();
}
