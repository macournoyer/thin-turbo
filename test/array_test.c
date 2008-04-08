#include "test.h"
#include "array.h"

test_init();

void test_push(void)
{
  array_t *a;
  char    *c;
  
  a = array_create(10, sizeof(char));
  
  c = array_push(a);
  
  assert_equal(1, a->nitems);
  assert_equal(a->items, c);
  
  c = array_push(a);
  
  assert_equal(2, a->nitems);
  assert_equal(a->items + sizeof(char), c);
  
  array_destroy(a);
}

void test_push_grow_array(void)
{
  array_t *a;
  
  a = array_create(2, sizeof(char));
  
  array_push(a);
  array_push(a);
  
  assert_equal(2, a->nalloc);
  
  array_push(a);
  
  assert_equal(4, a->nalloc);
  
  array_destroy(a);
}

int main(int argc, char const *argv[])
{
  test_start();
  
  test_push();
  test_push_grow_array();
  
  test_end();
}