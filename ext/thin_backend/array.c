#include "array.h"

array_t * array_create(uint num, size_t size)
{
  array_t *a = (array_t *) malloc(sizeof(array_t));
  if (a == NULL)
    return NULL;
  
  a->size   = size;
  a->nalloc = num;
  a->nitems = 0;
  a->items  = malloc(num * size);
  
  if (a->items == NULL) {
    free(a);
    return NULL;
  }

  return a;
}

void * array_push(array_t *a)
{
  void   *item, *old, *new;
  size_t  size;
  
  if (a->nitems == a->nalloc) {
    /* array is full, double the size */
    size = a->size * a->nalloc;

    new = malloc(2 * size);
    if (new == NULL)
      return NULL;

    memcpy(new, a->items, size);
    old = a->items;
    a->items = new;
    a->nalloc *= 2;
    
    free(old);
  }
  
  item = (u_char *) a->items + a->size * a->nitems;
  a->nitems++;
  
  return item;
}

void array_destroy(array_t *a)
{
  void *items;
  
  items = a->items;
  
  free(items);
  free(a);
}