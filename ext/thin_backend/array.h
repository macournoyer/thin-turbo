#ifndef _ARRAY_H_
#define _ARRAY_H_

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>

typedef struct thin_array_s thin_array_t;

struct thin_array_s {
  size_t  size;
  uint    nalloc;
  uint    nitems;
  void   *items;
};

/* Create an dynamic array of initialy +num+ items */
thin_array_t * thin_array_create(uint num, size_t size);

/* Push a new item to the array returning its ref.
 * If the array is not big enought its size is doubled. */
void * thin_array_push(thin_array_t *a);

/* Destroy the array and frees the memory */
void thin_array_destroy(thin_array_t *a);

#endif /* _ARRAY_H_ */
