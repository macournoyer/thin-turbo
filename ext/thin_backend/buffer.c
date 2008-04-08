#include "buffer.h"

void buffer_init(buffer_t *buf, pool_t *p)
{
  buf->pool    = p;
  buf->ptr     = palloc(p, 1);
  assert(buf->ptr);
  buf->nalloc  = 1;
  buf->salloc  = p->size;
  buf->len     = 0;
  buf->current = 0;
}

void buffer_free(buffer_t *buf)
{
  if (buf->ptr != NULL)
    pfree(buf->pool, buf->ptr);
  buf->salloc = 0;
  buf->nalloc = 0;
}

static inline void buffer_grow(buffer_t *buf, size_t size)
{
  char *new, *old;
  
  /* TODO store big body in tempfile */
  /* TODO if last alloc, just alloc next block */
  old = buf->ptr;
  new = (char *) palloc(buf->pool, buf->nalloc + 1);
  assert(new);
  
  memcpy(new, old, buf->len);

  buf->ptr     = new;
  buf->nalloc ++;
  buf->salloc += buf->pool->size;
  pfree(buf->pool, old);
}

void buffer_append(buffer_t *buf, const char *ptr, size_t len)
{
  /* alloc more mem when buffer full */
  if (buf->len >= buf->salloc)
    buffer_grow(buf, len);
  
  memcpy(buf->ptr + buf->len, ptr, len);
  buf->len += len;
}