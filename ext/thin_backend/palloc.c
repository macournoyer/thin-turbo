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

void pool_destroy(pool_t *p)
{
  void   *alloc = p->alloc;
  size_t *assign = p->assign;
  
  free(alloc);
  free(assign);
  free(p);
}

void * palloc(pool_t *p, size_t num)
{
  size_t numfree, i = 0;
  
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

int pfree(pool_t *pool, void *ptr)
{
  pool_t *p = pool;
  size_t  i;
  
  while (p != NULL &&
        /* ptr in range of alloced mem? */
        (ptr < p->alloc || ptr > p->alloc + p->num * p->size))
    p = p->next;
  
  if (p == NULL)
    return -1;
  
  i = ((int) ptr) - ((int) p->alloc);
  p->assign[i] = 0;
  
  return 0;
}