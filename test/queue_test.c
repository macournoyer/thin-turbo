#include "test.h"
#include "queue.h"

test_init();

void test_push_pop(void)
{
  queue_t q;
  
  queue_init(&q);
  
  queue_push(&q, "your");
  queue_push(&q, "muffin");
  queue_push(&q, "stink");
  
  assert_str_equal("your", queue_pop(&q));
  assert_str_equal("muffin", queue_pop(&q));
  assert_str_equal("stink", queue_pop(&q));

  assert_equal(NULL, queue_pop(&q));
  
  queue_reset(&q);
}

int main(int argc, char const *argv[])
{
  test_start();
  
  test_push_pop();
  
  test_end();
}