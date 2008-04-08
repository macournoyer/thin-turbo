#ifndef _BUFFER_H_
#define _BUFFER_H_

#include "palloc.h"

typedef struct buffer_s buffer_t;

struct buffer_s {
  size_t  nalloc; /* num slices alloced */
  size_t  salloc; /* total size alloced */
  size_t  len;
  size_t  current;
  pool_t *pool;
  char   *ptr;
};

void buffer_init(buffer_t *buf, pool_t *p);
void buffer_free(buffer_t *buf);
void buffer_append(buffer_t *buf, const char *ptr, size_t len);

#endif /* _BUFFER_H_ */
