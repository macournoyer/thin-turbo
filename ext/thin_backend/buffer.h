#ifndef _BUFFER_H_
#define _BUFFER_H_

#include "palloc.h"

#define BUFFER_POOL_SIZE        100
#define BUFFER_CHUNK_SIZE       1024
#define BUFFER_MAX_LEN          1024 * (80) /* TODO */
#define BUFFER_TMPFILE_TEMPLATE "/tmp/.thin-buffer.XXXXXX"

typedef struct buffer_s buffer_t;
typedef struct chunk_s chunk_t;

struct buffer_s {
  size_t   len;
  off_t    offset;
  char    *ptr;
  
  size_t   nalloc;
  size_t   salloc;
  
  struct {
    int    fd;
    char  *name;
  } file;
};

#define buffer_in_file(buf) (buf->file.fd != -1)

void buffer_init(buffer_t *buf);
void buffer_reset(buffer_t *buf);
int buffer_append(buffer_t *buf, const char *ptr, size_t len);

#endif /* _BUFFER_H_ */
