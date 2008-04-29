#include "test.h"
#include "buffer.h"

test_init();

void test_buffer_init(void)
{
  buffer_t  b;
  
  buffer_init(&b);
  
  assert_equal(1, b.nalloc);
  assert_equal(BUFFER_CHUNK_SIZE, b.salloc);
}

void test_buffer_append(void)
{
  buffer_t  b;
  
  buffer_init(&b);
  
  buffer_append(&b, "hi", 2);
  assert_str_equal("hi", b.ptr);
  
  buffer_append(&b, " you", 4);
  assert_str_equal("hi you", b.ptr);
  assert_equal(6, b.len);
  assert_equal(1, b.nalloc);
}

void test_buffer_grow_and_append(void)
{
  buffer_t  b;
  
  buffer_init(&b);
  
  buffer_append(&b, "hi", 2);
  assert_equal(1, b.nalloc);
  assert_equal(BUFFER_CHUNK_SIZE, b.salloc);

  buffer_append(&b, " you", BUFFER_CHUNK_SIZE * 2);
  assert_equal(BUFFER_CHUNK_SIZE * 2 + 2, b.len);
  assert_equal(3, b.nalloc);
  assert_equal(BUFFER_CHUNK_SIZE * 3, b.salloc);

  buffer_append(&b, " ! ", BUFFER_CHUNK_SIZE / 2 * 3); /* odd num */
  assert_equal(BUFFER_CHUNK_SIZE * 2 + 2 + (BUFFER_CHUNK_SIZE / 2 * 3), b.len);
  assert_equal(4, b.nalloc);

  assert_equal(BUFFER_CHUNK_SIZE * 4, b.salloc);  
}

void test_buffer_reset(void)
{
  buffer_t b;
  
  buffer_init(&b);
  buffer_reset(&b);
}

int main(int argc, char const *argv[])
{
  test_start();
  
  test_buffer_init();
  test_buffer_append();
  test_buffer_grow_and_append();
  test_buffer_reset();
  
  test_end();
}