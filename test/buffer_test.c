#include "test.h"
#include "buffer.h"

test_init();

void test_buffer_init(void)
{
  pool_t   *p = pool_create(10, 512);
  buffer_t  b;
  
  buffer_init(&b, p);
  
  assert_equal(p, b.pool);
  assert_equal(1, b.nalloc);
  assert_equal(512, b.salloc);
  
  pool_destroy(p);
}

void test_buffer_append(void)
{
  pool_t   *p = pool_create(10, 1024);
  buffer_t  b;
  
  buffer_init(&b, p);
  
  buffer_append(&b, "hi", 2);
  assert_str_equal("hi", b.ptr);
  
  buffer_append(&b, " you", 4);
  assert_str_equal("hi you", b.ptr);
  assert_equal(6, b.len);
  assert_equal(1, b.nalloc);
  
  pool_destroy(p);
}

void test_buffer_grow_and_append(void)
{
  pool_t   *p = pool_create(10, 2);
  buffer_t  b;
  
  buffer_init(&b, p);
  
  buffer_append(&b, "hi", 2);
  assert_equal(1, b.nalloc);
  assert_equal(2, b.salloc);
  
  buffer_append(&b, " you", 4);
  assert_equal(6, b.len);
  assert_equal(3, b.nalloc);
  assert_equal(6, b.salloc);

  buffer_append(&b, " ! ", 3); /* odd num */
  assert_equal(9, b.len);
  assert_equal(4, b.nalloc);
  assert_equal(8, b.salloc);

  assert_str_equal("hi you ! ", b.ptr);
  
  pool_destroy(p);
}

void test_buffer_free(void)
{
  pool_t   *p = pool_create(10, 2);
  buffer_t  b;
  
  buffer_init(&b, p);
  buffer_free(&b);
  
  pool_destroy(p);
}

int main(int argc, char const *argv[])
{
  test_start();
  
  test_buffer_init();
  test_buffer_append();
  test_buffer_grow_and_append();
  test_buffer_free();
  
  test_end();
}