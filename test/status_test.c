#include "test.h"
#include "status.h"

test_init();

void test_get_status(void)
{
  assert_equal("100 Continue", thin_status(100));
  assert_equal("200 OK", thin_status(200));
  assert_equal("500 Internal Server Error", thin_status(500));
  assert_equal("405 Method Not Allowed", thin_status(405));
}

void test_unknow_status_return_200_ok(void)
{
  assert_equal("200 OK", thin_status(999));
}

int main(int argc, char const *argv[])
{
  test_start();
  
  test_get_status();
  test_unknow_status_return_200_ok();
  
  test_end();
}