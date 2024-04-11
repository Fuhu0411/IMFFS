#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

int Tests_Passed = 0;
int Tests_Failed = 0;

void verify_int(int expected, int result, char test[]) {
  if (expected == result) {
    printf("Passed: expected %d, got %d for: %s\n", expected, result, test);
    Tests_Passed++;
  } else {
    printf("FAILED: expected %d, got %d for: %s\n", expected, result, test);
    Tests_Failed++;
  }
}

void verify_str(char expected[], char result[], char test[]) {
  if ((NULL == expected && NULL == result) ||
      (NULL != expected && NULL != result && strcmp(expected, result) == 0)) {
    printf("Passed: expected '%s', got '%s' for: %s\n", expected, result, test);
    Tests_Passed++;
  } else {
    printf("FAILED: expected '%s', got '%s' for: %s\n", expected, result, test);
    Tests_Failed++;
  }
}

// type "void *" is a pointer to anything; we'll learn more soon
void verify_null(void *result, char test[]) {
  if (NULL == result) {
    printf("Passed: expected NULL, got NULL for: %s\n", test);
    Tests_Passed++;
  } else {
    printf("FAILED: expected NULL, got %p for: %s\n", result, test);
    Tests_Failed++;
  }
}

void verify_not_null(void *result, char test[]) {
  if (NULL != result) {
    printf("Passed: expected not NULL, got %p for: %s\n", result, test);
    Tests_Passed++;
  } else {
    printf("FAILED: expected not NULL, got NULL for: %s\n", test);
    Tests_Failed++;
  }
}
