#include "palloc.h"

pool_t * pool_create(size_t num, size_t size)
{
  pool_t *p;
  size_t  i;
  
  p = malloc(sizeof(pool_t));
  if (p == NULL)
    return NULL;
  
  p->alloc = malloc(num * size);
  if (p->alloc == NULL) {
    free(p);
    return NULL;
  }
  
  p->assign = (size_t *) malloc(sizeof(size_t) * num);
  if (p->assign == NULL) {
    free(p);
    free(p->alloc);
    return NULL;
  }
  
  for (i = 0; i < num; ++i)
    p->assign[i] = 0;
  
  p->num = num;
  p->size = size;
  p->next = NULL;
  
  return p;
}

void pool_destroy(pool_t *pool)
{
  pool_t *p = pool, *next;
  
  while (p->next != NULL) {
    next = p->next;
    
    free(p->alloc);
    free(p->assign);
    free(p);
    
    p = next;
  }
}

void * palloc(pool_t *p, size_t num)
{
  size_t numfree, i = 0;
  
  /* TODO we should build a custom pool in that case... */
  assert(num <= p->num);
  
  for (;;) {
    /* skip all following used blocks */
    i += p->assign[i];
    
    /* check for enought free slices */
    for (numfree = 0; (numfree < num) && (i + numfree <= p->num - 1); ++numfree)
      if (p->assign[i + numfree] != 0)
        break;
    
    if (numfree == num)
      break;
    else
      i += numfree;
    
    /* continue w/ next pool */
    if (i >= p->num - 1) {
      i = 0;
      
      if (p->next == NULL) {
        p->next = pool_create(p->num, p->size);
        p = p->next;
        break;
      } else {
        p = p->next;
      }
    }    
  }
  
  p->assign[i] = num;
  
  return (u_char *) p->alloc + p->size * i;
}

void pfree(pool_t *pool, void *ptr)
{
  pool_t *p = pool;
  size_t  pool_i = 0, i;
  
  while (p != NULL &&
        /* ptr in range of alloced mem? */
        (ptr < p->alloc || ptr > p->alloc + (p->num - 1) * p->size)) {
    p = p->next;
    pool_i++;
  }
  
  assert(p != NULL && "memory not allocated from a pool");
  assert(ptr >= p->alloc && "ptr must be inside pool");
  
  i = (((int) ptr) - ((int) p->alloc)) / p->size;
  p->assign[i] = 0;
}