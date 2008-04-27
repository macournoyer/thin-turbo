#include "buffer.h"

pool_declare(buffer, BUFFER_POOL_SIZE, BUFFER_CHUNK_SIZE)

void buffer_init(buffer_t *buf)
{
  buf->ptr         = palloc(buffer_pool(), 1);
  buf->nalloc      = 1;
  buf->salloc      = 0;
  buf->len         = 0;
  buf->offset      = 0;
  buf->file.fd     = -1;
  buf->file.name   = NULL;
}

void buffer_reset(buffer_t *buf)
{
  if (buf->ptr != NULL)
    pfree(buffer_pool(), buf->ptr);
  
  if (buffer_in_file(buf)) {
    close(buf->file.fd);
    unlink(buf->file.name);
    free(buf->file.name);
  }
  
  buffer_init(buf);
}

static int buffer_open_tmpfile(buffer_t *buf)
{
  assert(!buffer_in_file(buf) && "buffer file already opened");
  
  buf->file.name = strdup(BUFFER_TMPFILE_TEMPLATE);
  buf->file.fd   = mkstemp(buf->file.name);

  if (buf->file.fd < 0) {
    /* error opening file */
    free(buf->file.name);
    buf->file.fd = -1;
    buf->file.name = NULL;
  }
  
  return buf->file.fd;
}

static int buffer_append_to_file(buffer_t *buf, const char *ptr, size_t len)
{
  int n = 0, written = 0;
  
  assert(buffer_in_file(buf) && "tried to append to a closed buffer file");  
  
  for (written = 0; written < len; written += n) {
    n = write(buf->file.fd, ptr + written, len - written);
    
    if (n < 0)
      return -1;
    
    buf->len += n;
  }
  
  return 0;
}

static int buffer_move_to_tmpfile(buffer_t *buf, const char *ptr, size_t len)
{
  if (buffer_open_tmpfile(buf) < 0)
    return -1;
  
  /* reset len of buffer */
  size_t mem_len = buf->len;
  buf->len = 0;
  
  if (buffer_append_to_file(buf, buf->ptr, mem_len) < 0) {
    buf->len = mem_len;
    return -1;
  }
  
  /* remove buffer from memory */
  pfree(buffer_pool(), buf->ptr);
  buf->ptr = NULL;
  
  if (buffer_append_to_file(buf, ptr, len) < 0)
    return -1;
  
  return 0;
}

static void buffer_adjust_len(buffer_t *buf, size_t len)
{
  size_t  new_len = buf->len + len;
  pool_t *pool = buffer_pool();
  
  assert(!buffer_in_file(buf));
  
  if (new_len <= buf->salloc)
    return;
  
  char   *new, *old;
  size_t  num = new_len / pool->size + 1;
  
  /* TODO if free space in next blocks, alloc more and don't memcpy */
  old = buf->ptr;
  new = (char *) palloc(pool, num);
  assert(new);
  
  memcpy(new, old, buf->len);

  buf->ptr    = new;
  buf->nalloc = num;
  buf->salloc = pool->size * num;
  
  assert(buf->len + len <= buf->salloc && "Failed to allocate buffer");
  
  pfree(pool, old);
}

int buffer_append(buffer_t *buf, const char *ptr, size_t len)
{
  if (buffer_in_file(buf))
    return buffer_append_to_file(buf, ptr, len);
  
  /* if too large we move the whole thing to a tmp file */
  if (buf->len + len > BUFFER_MAX_LEN)
    return buffer_move_to_tmpfile(buf, ptr, len);
  
  /* make sure enough mem allocated */
  buffer_adjust_len(buf, len);
  
  memcpy(buf->ptr + buf->len, ptr, len);
  buf->len += len;
  
  return 0;
}
