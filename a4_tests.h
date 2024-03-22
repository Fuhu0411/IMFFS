// Assignment 4 solution: common test definitions and functions

#define VERIFY_INT(e,t) verify_int(e, t, #t)
#define VERIFY_STR(e,t) verify_str(e, t, #t)
// use these two for other types of pointers, *not* strings:
#define VERIFY_NULL(t) verify_null(t, #t)
#define VERIFY_NOT_NULL(t) verify_not_null(t, #t)

// Two options for these variables: make them extern here, or keep them
// private (static) in a4_tests.c and add getters for them.
// We've seen examples of #2 so let's use strategy #1.
extern int Tests_Passed;
extern int Tests_Failed;

void verify_int(int expected, int result, char test[]);
void verify_str(char expected[], char result[], char test[]);
void verify_null(void *result, char test[]);
void verify_not_null(void *result, char test[]);
