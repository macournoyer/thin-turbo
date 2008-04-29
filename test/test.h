#include <stdio.h>

#define test_init() \
  static int assertions; \
  static int failures;

#define test_start() \
  printf("== %s\n", argv[0]);

#define test_end() \
  printf("== %d assertions, %d success, %d failures\n", assertions, assertions - failures, failures); \
  if (failures > 0) { return -1; } else { return 0; }

#define _assert(cmp, fmt, expected, actual) \
  assertions++; \
  if (cmp) { \
    printf(fmt " expected but was " fmt ", in %s at line %d\n", expected, actual, __FUNCTION__, __LINE__); \
    failures++; \
  }

#define assert_equal(expected, actual)     _assert(expected != actual, "%d", expected, actual)
#define assert_int_equal(expected, actual) _assert(expected != actual, "%d", expected, actual)
#define assert_str_equal(expected, actual) _assert(strcmp(expected, actual) != 0, "%s", expected, actual)
