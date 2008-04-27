#include "queue.h"

/* lifo queue, push last, pop first */

pool_declare(queue, QUEUE_POOL_SIZE, sizeof(queue_item_t))

void queue_init(queue_t *q)
{
  q->first = NULL;
  q->last  = NULL;
}

void queue_reset(queue_t *q)
{
  /* drain the queue */
  while (q->first != NULL)
    queue_pop(q);
  
  queue_init(q);
}

/* push item to the end of the queue */
void queue_push(queue_t *q, void *item)
{
  queue_item_t *qi = palloc(queue_pool(), 1);
  
  qi->item = item;
  qi->next = NULL;
  
  if (q->first == NULL)
    q->first = qi;
  
  if (q->last != NULL)
    q->last->next = qi;
  
  q->last = qi;
}

/* pop the first item in the queue */
void * queue_pop(queue_t *q)
{
  if (q->first == NULL)
    return NULL;
  
  queue_item_t *qi   = q->first;
  void         *item = qi->item;
  
  q->first = qi->next;
  
  if (qi == q->last)
    q->last == NULL;
  
  pfree(queue_pool(), qi);
  
  return item;
}
