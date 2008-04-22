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

void buffer_grow(buffer_t *buf, size_t len)
{
  size_t  new_len = buf->len + len;
  
  if (new_len <= buf->salloc)
    return;
  
  char   *new, *old;
  size_t  num = new_len / buf->pool->size + 1;
  
  /* TODO store big body in tempfile */
  /* TODO if free space in next blocks, alloc more and don't memcpy */
  
  old = buf->ptr;
  new = (char *) palloc(buf->pool, num);
  assert(new);
  
  memcpy(new, old, buf->len);

  buf->ptr    = new;
  buf->nalloc = num;
  buf->salloc = buf->pool->size * num;
  
  assert(buf->len + len <= buf->salloc && "Failed to allocate buffer");
  
  pfree(buf->pool, old);
}

void buffer_append(buffer_t *buf, const char *ptr, size_t len)
{
  buffer_grow(buf, len);
  
  memcpy(buf->ptr + buf->len, ptr, len);
  buf->len += len;
}
