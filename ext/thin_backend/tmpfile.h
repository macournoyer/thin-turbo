#ifndef _TMPFILE_H_
#define _TMPFILE_H_

#include <ev.h>

#include "buffer.h"

#define TMPFILE_TEMPLATE "/tmp/.thin-buffer.XXXXXX"

typedef struct tmpfile_s tmpfile_t;

struct tmpfile_s {
  int              fd;
  char            *fname;
  
  buffer_t        *buf;
  
  struct ev_loop  *loop;
  ev_io            read_watcher;
  ev_io            write_watcher;
}

#endif /* _TMPFILE_H_ */
