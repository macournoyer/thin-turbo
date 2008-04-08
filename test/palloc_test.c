#include "test.h"
#include "palloc.h"

test_init();

#define assert_assign(p, a) _assert_assign(p, a, __LINE__)
void _assert_assign(pool_t *pool, char *assign, int line)
{
  pool_t *p = pool;
  int     i, len = 0;
  char    actual[1024];
  
  while (p != NULL) {
    for (i = 0; i < p->num; ++i)
      len += sprintf((char *) actual + len, "%d ", p->assign[i]);
    p = p->next;
  }
  
  actual[len - 1] = '\0';
  
  assertions++;
  if (strcmp(assign, actual) != 0) {
    printf("assert_assign failed at line %d\nexpected [%s]\n but was [%s]\n", line, assign, actual);
    failures++;
  }
}

void test_alloc_same_size(void)
{
  pool_t *p;
  char   *s1, *s2;
  
  p = pool_create(5, 1024);
  
  s1 = palloc(p, 1);
  s2 = palloc(p, 1);
  assert_assign(p, "1 1 0 0 0");
  
  pfree(p, s1);
  assert_assign(p, "0 1 0 0 0");
  
  s1 = palloc(p, 1);
  assert_assign(p, "1 1 0 0 0");
  
  pfree(p, s1);
  pfree(p, s2);
  
  assert_assign(p, "0 0 0 0 0");
  
  pool_destroy(p);
}

void test_alloc_diff_size(void)
{
  pool_t *p;
  char   *s1, *s2, *s3;
  
  p = pool_create(5, 1024);
  
  s1 = palloc(p, 2); 
  s2 = palloc(p, 1);
  assert_assign(p, "2 0 1 0 0");
  
  pfree(p, s1);
  s1 = palloc(p, 1);
  assert_assign(p, "1 0 1 0 0");
  
  s3 = palloc(p, 2);
  assert_assign(p, "1 0 1 2 0");
  
  pfree(p, s1);
  pfree(p, s2);
  pfree(p, s3);
  
  assert_assign(p, "0 0 0 0 0");
  
  pool_destroy(p);
}

void test_grow_pool(void)
{
  pool_t *p;
  char   *s1, *s2, *s3, *s4, *s5, *s6;
  
  p = pool_create(2, 1024);
  
  s1 = palloc(p, 1);
  s2 = palloc(p, 1);
  assert_assign(p, "1 1");

  s3 = palloc(p, 1);
  assert_assign(p, "1 1 1 0");
  
  s4 = palloc(p, 1);
  s5 = palloc(p, 1);
  s6 = palloc(p, 1);
  assert_assign(p, "1 1 1 1 1 1");
  
  pfree(p, s1);
  pfree(p, s2);
  pfree(p, s3);
  pfree(p, s4);
  pfree(p, s5);
  pfree(p, s6);
  
  assert_assign(p, "0 0 0 0 0 0");
  
  pool_destroy(p);
}

void test_grow_with_big_size(void)
{
  pool_t *p;
  char   *s1, *s2, *s3;
  
  p = pool_create(5, 1024);
  
  s1 = palloc(p, 2);
  s2 = palloc(p, 1);
  assert_assign(p, "2 0 1 0 0");
  s3 = palloc(p, 3);
  assert_assign(p, "2 0 1 0 0 3 0 0 0 0");
  
  pfree(p, s1);
  pfree(p, s2);
  pfree(p, s3);
  assert_assign(p, "0 0 0 0 0 0 0 0 0 0");
  
  pool_destroy(p);
}

void test_frees_from_next_pool(void)
{
  pool_t *p;
  char   *s1, *s2, *s3;
  
  p = pool_create(2, 512);
  
  s1 = palloc(p, 2);
  s2 = palloc(p, 2);
  assert_assign(p, "2 0 2 0");

  s3 = palloc(p, 2);
  assert_assign(p, "2 0 2 0 2 0");
  
  pfree(p, s1);
  assert_assign(p, "0 0 2 0 2 0");
  
  pfree(p, s2);
  assert_assign(p, "0 0 0 0 2 0");
  
  pfree(p, s3);
  assert_assign(p, "0 0 0 0 0 0");
  
  pool_destroy(p);
}

int main(int argc, char const *argv[])
{
  test_start();
  
  test_alloc_same_size();
  test_alloc_diff_size();
  test_grow_pool();
  test_grow_with_big_size();
  test_frees_from_next_pool();
  
  test_end();
}