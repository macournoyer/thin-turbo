#include "buffer.h"

void buffer_init(buffer_t *buf)
{
  buf->len       = 0;
  buf->offset    = 0;
  buf->first     = NULL;
  buf->last      = NULL;
  buf->file.fd   = -1;
  buf->file.name = NULL;
}

void buffer_reset(buffer_t *buf)
{
  chunk_t *c = buf->first;
  
  while (c != NULL) {
    buffer_chunk_destroy(c); /* safe cause in pool */
    c = c->next;
  }
  
  if (buffer_in_file(buf)) {
    close(buf->file.fd);
    unlink(buf->file.name);
    free(buf->file.name);
  }
  
  buffer_init(buf);
}

static void buffer_append_mem(buffer_t *buf, chunk_t *chunk)
{
  buf->last->next = chunk;
  buf->last = chunk;  
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

static int buffer_append_tmpfile(buffer_t *buf, chunk_t *chunk)
{
  chunk_t *c = chunk;
  int      n, written;

  assert(buffer_in_file(buf) && "tried to appened to closed buffer file");  
  
  while (c != NULL) {
    for(written = 0; written < c->len; written += n) {
      n = write(buf->file.fd, c->ptr + written, c->len - written);

      if (n < 0)
        return -1;
    }
    
    buffer_chunk_destroy(c); /* safe cause in pool */
    c = c->next;
  }
  
  return 0;
}

/* append a chunk to the buffer, returns number of bytes appened, -1 on error */
int buffer_append_chunk(buffer_t *buf, chunk_t *chunk)
{
  size_t   written = 0;
	chunk_t *c = chunk;
  
  /* compute total bytes to be written */
  while (c != NULL) {
    written += c->len;
    c = c->next;
  }
	
	if (buffer_in_file(buf)) {
    /* if file open, write to it */
	  if (buffer_append_tmpfile(buf, chunk) < 0)
      return -1;
	}	else {
    /* else write in memory */
  	buffer_append_mem(buf, chunk);
	
    /* when max memory buffer size reached, move all to file */
  	if (buf->len > BUFFER_MAX_LEN) {
      if (buffer_open_tmpfile(buf) < 0 || buffer_append_tmpfile(buf, buf->first) < 0)
        return -1;
  	}
	}
	
	/* all successfull! */
  buf->len += written;
  return written;
}

int buffer_append(buffer_t *buf, char *ptr, size_t len)
{
  chunk_t *c = NULL;
  
  /* TODO */
  assert(len <= BUFFER_CHUNK_SIZE && "TODO");
  
  /* select which chunk to copy to */
  if (!buffer_in_file(buf) && buf->last->len + len < BUFFER_CHUNK_SIZE)
    c = buf->last;
  else
    c = buffer_chunk_create();
  
  memcpy(c->ptr + c->len, ptr, len);
  c->len += len;
  
  buffer_append_chunk(buf, c);    
  
  return 0;
}


/* chunks */

static pool_t * chunk_pool_ptr;
static pool_t * chunk_pool(void)
{
  if (!chunk_pool_ptr)
    chunk_pool_ptr = pool_create(BUFFER_CHUNK_POOL_SIZE, sizeof(chunk_t));
  
  return chunk_pool_ptr;
}

chunk_t * buffer_chunk_create(void)
{
  /* TODO change pool to free list type, last arg always 1 */
  chunk_t *c = palloc(chunk_pool(), 1);
  
  c->ptr  = NULL;
  c->len  = 0;
  c->next = NULL;
  
  return c;
}

void buffer_chunk_destroy(chunk_t *c)
{
  pfree(chunk_pool(), c);
}
