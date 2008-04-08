#include <stdio.h>

#define test_init() \
  static int assertions; \
  static int failures;

#define test_start() \
  printf("== %s\n", argv[0]);

#define test_end() \
  printf("== %d assertions, %d success, %d failures\n", assertions, assertions - failures, failures); \
  if (failures > 0) { return -1; } else { return 0; }

#define assert_equal(expected, actual) \
  assertions++; \
  if (expected != actual) { \
    printf(#expected " expected but was " #actual ", at line %d\n", __LINE__); \
    failures++; \
  }

#define assert_str_equal(expected, actual) \
  assertions++; \
  if (strcmp(expected, actual) != 0) { \
    printf("%s expected but was %s, at line %d\n", expected, actual, __LINE__); \
    failures++; \
  }
  