#ifndef _PALLOC_H_
#define _PALLOC_H_

/* Pooled memory allocator of predefined slice size.
 * Prevent the frequent use of malloc/free of same size
 * allocating a large amount of memory and spliting it
 * in equal slice. You can use several slice at once. */
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

typedef struct pool_s pool_t;

struct pool_s {
  size_t  num;
  size_t  size;
  void   *alloc;
  size_t *assign;
  pool_t *next;
};

/* create a pool of +num+ initial slices each of specified +size+ */
pool_t * pool_create(size_t num, size_t size);

/* frees all memory used by the pool */
void pool_destroy(pool_t *p);

/* Allocate +num+ slices of memory.
 * +num+ must not be higher then the pool predefined +num+. */
void * palloc(pool_t *p, size_t num);

/* mark the memory as free in the pool */
void pfree(pool_t *pool, void *ptr);

#endif /* _PALLOC_H_ */
