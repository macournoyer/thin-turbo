#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>

#include "palloc.h"

/* Number of chunk in the buffer pool */
#define BUFFER_POOL_SIZE        (80 + 32 + 1)

/* Size of one chunk in the buffer pool */
#define BUFFER_CHUNK_SIZE       1024

/* Size at which the buffer is moved to a file.
   it must be greater then MAX_HEADER defined in thin.h */
#define BUFFER_MAX_LEN          (BUFFER_POOL_SIZE * BUFFER_CHUNK_SIZE)

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
#define buffer_eof(buf) (buf->offset == buf->len)

void buffer_init(buffer_t *buf);
void buffer_reset(buffer_t *buf);
int buffer_append(buffer_t *buf, const char *ptr, size_t len);

#endif /* _BUFFER_H_ */
