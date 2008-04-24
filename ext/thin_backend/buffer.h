#ifndef _BUFFER_H_
#define _BUFFER_H_

#include "palloc.h"

#define BUFFER_CHUNK_POOL_SIZE  100
#define BUFFER_CHUNK_SIZE       1024
#define BUFFER_MAX_LEN          1024
#define BUFFER_TMPFILE_TEMPLATE "/tmp/.thin-buffer.XXXXXX"

typedef struct buffer_s buffer_t;
typedef struct chunk_s chunk_t;

struct buffer_s {
  size_t   len;
  off_t    offset;

  chunk_t *first;
  chunk_t *last;
  
  struct {
    int    fd;
    char  *name;
  } file;
};

struct chunk_s {
  char     buf[BUFFER_CHUNK_SIZE];
  char    *ptr;

  size_t   len;
  chunk_t *next;
};

#define buffer_in_file(buf) (buf->file.fd != -1)

void buffer_init(buffer_t *buf);
void buffer_reset(buffer_t *buf);
int buffer_append_chunk(buffer_t *buf, chunk_t *chunk);
int buffer_append(buffer_t *buf, char *ptr, size_t len);

chunk_t * buffer_chunk_create(void);
void buffer_chunk_destroy(chunk_t *c);

#endif /* _BUFFER_H_ */
