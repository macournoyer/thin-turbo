#ifndef _QUEUE_H_
#define _QUEUE_H_

#include "palloc.h"

#define QUEUE_POOL_SIZE 100

typedef struct queue_s queue_t;
typedef struct queue_item_s queue_item_t;

struct queue_s {
  queue_item_t *first;
  queue_item_t *last;
};

struct queue_item_s {
  void         *item;
  queue_item_t *next;
};

void queue_init(queue_t *q);
void queue_reset(queue_t *q);
void queue_push(queue_t *q, void *item);
void * queue_pop(queue_t *q);

#endif /* _QUEUE_H_ */
