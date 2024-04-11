#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "a4_tests.h"
#include "a5_multimap.h"

/*** Comparison functions for keys and values ***/

static int void_strcasecmp(void *a, void *b) {
  assert(NULL != a && NULL != b);
  return strcasecmp(a, b);
}

static int compare_values_num_part(void *a, void *b) {
  assert(NULL != a && NULL != b);
  Value *va = a, *vb = b;
  return va->num - vb->num;
}

static int compare_ints(void *a, void *b) {
  assert(NULL != a && NULL != b);
  int *ia = a, *ib = b;
  return *ia - *ib;
}

static int compare_double_values(void *a, void *b) {
  assert(NULL != a && NULL != b);
  Value *va = a, *vb = b;

  double *da = va->data, *db = vb->data;
  assert(NULL != da && NULL != db);

  if (*da - *db < 0) {
    return -1;
  }
  if (*da - *db > 0) {
    return 1;
  }
  return 0;
}

// This one will *not* work for keys,
// since binary search needs predictable ordering
static int compare_always_less(void *a, void *b) {
  return -1;
}

/*** Example tests based on assignment 3 ***/

void test_example() {
  Multimap *mm;
  Value arr[3]; // using one extra space
  
  printf("\n*** Example tests:\n\n");
  
  VERIFY_NOT_NULL(mm = mm_create(3, void_strcasecmp, compare_values_num_part));
  VERIFY_INT(1, mm_insert_value(mm, "hello", 123, "abc"));
  VERIFY_INT(1, mm_insert_value(mm, "world", 456, "def"));
  VERIFY_INT(2, mm_insert_value(mm, "hello", 999, "zzz"));
  // maximum value length:
  VERIFY_INT(3, mm_insert_value(mm, "hello", 1048576, "abcdefghijabcdefghij"));
  // there is no connection between this value and the one with the key "hello"
  VERIFY_INT(1, mm_insert_value(mm, "!", 123, "abc"));
  VERIFY_INT(2, mm_insert_value(mm, "!", 5, ""));
  
  VERIFY_INT(3, mm_count_keys(mm));
  VERIFY_INT(3, mm_count_values(mm, "hello"));
  VERIFY_INT(1, mm_count_values(mm, "world"));
  VERIFY_INT(2, mm_count_values(mm, "!"));
  VERIFY_INT(0, mm_count_values(mm, "?"));
  
  VERIFY_INT(2, mm_get_values(mm, "!", arr, 2));
  VERIFY_INT(5, arr[0].num);
  VERIFY_STR("", arr[0].data);
  VERIFY_INT(123, arr[1].num);
  VERIFY_STR("abc", arr[1].data);
  
  // no space for more keys
  VERIFY_INT(-1, mm_insert_value(mm, "xyz", 789, "ghi"));

  VERIFY_INT(2, mm_remove_key(mm, "!"));
  VERIFY_INT(0, mm_count_values(mm, "!"));
  VERIFY_INT(1, mm_insert_value(mm, "xyz", 789, "ghi"));

  // make sure we aren't copying three values when we ask for two
  arr[2].num = 0;
  arr[2].data = NULL;
  VERIFY_INT(2, mm_get_values(mm, "hello", arr, 2));
  VERIFY_INT(123, arr[0].num);
  VERIFY_STR("abc", arr[0].data);
  VERIFY_INT(999, arr[1].num);
  VERIFY_STR("zzz", arr[1].data);
  VERIFY_INT(0, arr[2].num);
  VERIFY_STR(NULL, arr[2].data);
  
  // this isn't a test:
  // NOTE: you can make mm_print default to (and only work with) string keys and values, since it's for debugging. Then you can uncomment it throughout the tests
  // The better strategy would have been to add key printer and value printer function pointers to the multimap. But we have enough of those already.
  // mm_print(mm);
  
  VERIFY_INT(8, mm_destroy(mm));
  
  // these could fail assertions but should execute and pass with assertions off
#ifdef NDEBUG
  VERIFY_NULL(mm_create(-1, void_strcasecmp, compare_values_num_part));
  VERIFY_INT(-1, mm_insert_value(NULL, "hello", 123, "abc"));
#endif
}

/*** Complete tests ***/

void test_typical() {
  Multimap *mm;
  Value arr[5];
  char key[] = "hello";
  char value[] = "goodbye";
  
  printf("\n*** Typical tests:\n\n");
  
  VERIFY_NOT_NULL(mm = mm_create(7, void_strcasecmp, compare_values_num_part));
  // Now we *can* change keys but we shouldn't, because it breaks ordering.
  // We can also change values, which is dangerous, because it *may* break ordering.
  VERIFY_INT(1, mm_insert_value(mm, key, 99, value));
  value[0] = 'f'; // will change the value inserted to "hello" too!
  VERIFY_INT(1, mm_insert_value(mm, "yello", 0, value));
  VERIFY_INT(1, mm_insert_value(mm, "aaa", 555, ""));
  VERIFY_INT(1, mm_insert_value(mm, "III", -555, ""));
  VERIFY_INT(2, mm_insert_value(mm, "hello", -1, "iii"));
  VERIFY_INT(2, mm_insert_value(mm, "yello", 1234567, "goodbye"));
  VERIFY_INT(3, mm_insert_value(mm, "Hello", 123456789, "GOODBYE"));
  VERIFY_INT(4, mm_insert_value(mm, "hELLO", -1, "Goodbye"));
  VERIFY_INT(5, mm_insert_value(mm, "HELLO", -1, "zzzzalphaend"));
  VERIFY_INT(1, mm_insert_value(mm, "Hellow", 0, ""));
  VERIFY_INT(1, mm_insert_value(mm, "H", 0, ""));
  VERIFY_INT(2, mm_insert_value(mm, "h", 1, ""));
  VERIFY_INT(1, mm_insert_value(mm, "???", 0, ""));
  VERIFY_INT(3, mm_insert_value(mm, "h", -1, ""));
  VERIFY_INT(4, mm_insert_value(mm, "h", 0, ""));

  VERIFY_INT(7, mm_count_keys(mm));
  
  printf("Inspect key ordering:\n");
  // mm_print(mm);
  
  VERIFY_INT(1, mm_count_values(mm, "???"));
  VERIFY_INT(0, mm_count_values(mm, "??"));
  VERIFY_INT(0, mm_count_values(mm, "????"));
  VERIFY_INT(1, mm_count_values(mm, "aaa"));
  VERIFY_INT(4, mm_count_values(mm, "h"));
  VERIFY_INT(5, mm_count_values(mm, "hello"));
  VERIFY_INT(5, mm_count_values(mm, "Hello"));
  VERIFY_INT(5, mm_count_values(mm, "HELLO"));
  VERIFY_INT(5, mm_count_values(mm, "hELLO"));
  VERIFY_INT(1, mm_count_values(mm, "hellow"));
  VERIFY_INT(0, mm_count_values(mm, "hell"));
  VERIFY_INT(1, mm_count_values(mm, "iii"));
  VERIFY_INT(2, mm_count_values(mm, "yello"));
 
  VERIFY_INT(5, mm_get_values(mm, key, arr, 5));
  VERIFY_INT(-1, arr[0].num);
  VERIFY_STR("zzzzalphaend", arr[0].data);
  VERIFY_INT(-1, arr[1].num);
  VERIFY_STR("Goodbye", arr[1].data);
  VERIFY_INT(-1, arr[2].num);
  VERIFY_STR("iii", arr[2].data);
  VERIFY_INT(99, arr[3].num);
  VERIFY_STR("foodbye", arr[3].data);
  VERIFY_INT(123456789, arr[4].num);
  VERIFY_STR("GOODBYE", arr[4].data);

  VERIFY_INT(2, mm_get_values(mm, "yello", arr, 5));
  VERIFY_INT(0, arr[0].num);
  VERIFY_STR("foodbye", arr[0].data);
  VERIFY_INT(1234567, arr[1].num);
  VERIFY_STR("goodbye", arr[1].data);

  VERIFY_INT(2, mm_remove_key(mm, "yello"));
  VERIFY_INT(0, mm_count_values(mm, "yello"));
  VERIFY_INT(0, mm_get_values(mm, "yello", arr, 5));
  VERIFY_INT(5, mm_count_values(mm, key));
  VERIFY_INT(6, mm_count_keys(mm));

  VERIFY_INT(5, mm_remove_key(mm, key));
  VERIFY_INT(0, mm_count_values(mm, key));
  
  VERIFY_INT(1, mm_insert_value(mm, "hello", -123, ""));
  VERIFY_INT(1, mm_get_values(mm, "hello", arr, 5));
  VERIFY_INT(-123, arr[0].num);
  VERIFY_STR("", arr[0].data);

  VERIFY_INT(3, mm_get_values(mm, "h", arr, 3));
  VERIFY_INT(-1, arr[0].num);
  VERIFY_STR("", arr[0].data);
  VERIFY_INT(0, arr[1].num);
  VERIFY_STR("", arr[1].data);
  VERIFY_INT(0, arr[2].num);
  VERIFY_STR("", arr[2].data);
  
  VERIFY_INT(15, mm_destroy(mm));
}

void test_different_types_and_orders() {
  Multimap *mm;
  Value arr[4];
  void *key;

  // using arrays because we need to store them in memory,
  // and we don't want to have to malloc() each separately
  int ints[] = { 13, 7, 9, 11, 99 };
  double doubles[] = { 17.5, 13.5, 7.5, 9.5, 11.5 };

  printf("\n*** Integer keys and double values:\n\n");
 
  VERIFY_NOT_NULL(mm = mm_create(4, compare_ints, compare_double_values));
  VERIFY_INT(1, mm_insert_value(mm, &ints[0], 1, &doubles[0]));
  VERIFY_INT(2, mm_insert_value(mm, &ints[0], 2, &doubles[1]));
  VERIFY_INT(1, mm_insert_value(mm, &ints[1], 3, &doubles[2]));
  VERIFY_INT(1, mm_insert_value(mm, &ints[2], 4, &doubles[3]));
  // num order doesn't matter, we're comparing the double part:
  VERIFY_INT(1, mm_insert_value(mm, &ints[3], 6, &doubles[4]));
  VERIFY_INT(2, mm_insert_value(mm, &ints[3], 5, &doubles[0]));
  VERIFY_INT(3, mm_insert_value(mm, &ints[3], 4, &doubles[1]));
  
  mm_print(mm);
  // keys:
  
  VERIFY_INT(1, mm_get_first_key(mm, &key));
  VERIFY_INT(7, *(int *)key);
  VERIFY_INT(1, mm_get_next_key(mm, &key));
  VERIFY_INT(9, *(int *)key);
  VERIFY_INT(1, mm_get_next_key(mm, &key));
  VERIFY_INT(11, *(int *)key);
  VERIFY_INT(1, mm_get_next_key(mm, &key));
  VERIFY_INT(13, *(int *)key);
  VERIFY_INT(0, mm_get_next_key(mm, &key)); // should not change key
  VERIFY_INT(13, *(int *)key);
  
  // values:
  
  VERIFY_INT(1, mm_get_values(mm, &ints[1], arr, 4)); // first
  VERIFY_INT(3, arr[0].num); // not going to check this every time
  // I don't want to have to write VERIFY_DOUBLE.
  // Note that precise double comparison is dangerous. Taking a risk here!
  VERIFY_INT(1, 7.5 == *(double *)arr[0].data);

  VERIFY_INT(2, mm_get_values(mm, &ints[0], arr, 4)); // last
  VERIFY_INT(1, 13.5 == *(double *)arr[0].data);
  printf("the double is: %f\n", *(double*)arr[0].data);
  VERIFY_INT(1, 17.5 == *(double *)arr[1].data);

  VERIFY_INT(3, mm_get_values(mm, &ints[3], arr, 4)); // second-last
  VERIFY_INT(1, 11.5 == *(double *)arr[0].data);
  VERIFY_INT(1, 13.5 == *(double *)arr[1].data);
  VERIFY_INT(1, 17.5 == *(double *)arr[2].data);

  doubles[0] = 21.5; // dangerous! this could violate the ordering! (it doesn't)

  VERIFY_INT(2, mm_get_values(mm, &ints[0], arr, 4)); // last
  VERIFY_INT(1, 13.5 == *(double *)arr[0].data);
  VERIFY_INT(1, 21.5 == *(double *)arr[1].data);
  VERIFY_INT(3, mm_get_values(mm, &ints[3], arr, 4)); // second-last
  VERIFY_INT(1, 11.5 == *(double *)arr[0].data);
  VERIFY_INT(1, 13.5 == *(double *)arr[1].data);
  VERIFY_INT(1, 21.5 == *(double *)arr[2].data);

  // removal:
  
  VERIFY_INT(1, mm_get_values(mm, &ints[2], arr, 4)); // second
  VERIFY_INT(1, 9.5 == *(double *)arr[0].data);
  VERIFY_INT(1, mm_remove_key(mm, &ints[2]));
  VERIFY_INT(0, mm_get_values(mm, &ints[2], arr, 4));
 
  // insert again:

  VERIFY_INT(1, mm_insert_value(mm, &ints[4], 1, &doubles[0]));
 
  // after removal and insertion:
 
  VERIFY_INT(1, mm_get_first_key(mm, &key));
  VERIFY_INT(7, *(int *)key);
  VERIFY_INT(1, mm_get_next_key(mm, &key));
  VERIFY_INT(11, *(int *)key);
  VERIFY_INT(1, mm_get_next_key(mm, &key));
  VERIFY_INT(13, *(int *)key);
  VERIFY_INT(1, mm_get_next_key(mm, &key));
  VERIFY_INT(99, *(int *)key);
  VERIFY_INT(0, mm_get_next_key(mm, &key));

  VERIFY_INT(11, mm_destroy(mm));
  
  // re-using the same int/double data helps verify that mm_destroy()
  // didn't overwrite anything

  printf("\n*** Integer keys and mixed values with values in insertion order:\n\n");

  VERIFY_NOT_NULL(mm = mm_create(3, compare_ints, compare_always_less));
  VERIFY_INT(1, mm_insert_value(mm, &ints[1], 1, &doubles[4]));
  VERIFY_INT(1, mm_insert_value(mm, &ints[0], 1, &doubles[1])); // again num is ignored for ordering
  VERIFY_INT(2, mm_insert_value(mm, &ints[0], 3, "hello"));
  VERIFY_INT(3, mm_insert_value(mm, &ints[0], 4, &ints[2]));
  VERIFY_INT(4, mm_insert_value(mm, &ints[0], 2, "world"));

  VERIFY_INT(1, mm_get_first_key(mm, &key));
  VERIFY_INT(7, *(int *)key);
  VERIFY_INT(1, mm_get_next_key(mm, &key));
  VERIFY_INT(13, *(int *)key);
  VERIFY_INT(0, mm_get_next_key(mm, &key));

  VERIFY_INT(1, mm_get_values(mm, &ints[1], arr, 4));
  VERIFY_INT(1, 11.5 == *(double *)arr[0].data);

  VERIFY_INT(4, mm_get_values(mm, &ints[0], arr, 4));
  mm_print(mm);
  VERIFY_INT(2, arr[0].num);
  VERIFY_STR("world", arr[0].data);
  VERIFY_INT(4, arr[1].num);
  VERIFY_INT(9, *(int *)arr[1].data);
  VERIFY_INT(3, arr[2].num);
  VERIFY_STR("hello", arr[2].data);
  VERIFY_INT(1, arr[3].num);
  VERIFY_INT(1, 13.5 == *(double *)arr[3].data);
 
  VERIFY_INT(7, mm_destroy(mm));
}

void test_empty()
{
  Multimap *mm;
  Value arr[1];
  
  printf("\n*** Empty multimap:\n\n");
  
  VERIFY_NOT_NULL(mm = mm_create(1, void_strcasecmp, compare_values_num_part));
  VERIFY_INT(0, mm_count_keys(mm));
  VERIFY_INT(0, mm_count_values(mm, "abc"));
  VERIFY_INT(0, mm_remove_key(mm, "A"));
  VERIFY_INT(0, mm_get_values(mm, "", arr, 1));
  printf("Nothing printed here:\n");
  // mm_print(mm);
  VERIFY_INT(0, mm_destroy(mm));
}

void test_edge()
{
  Multimap *mm;
  Value arr[3];

  printf("\n*** Edge cases:\n\n");
  
  VERIFY_NOT_NULL(mm = mm_create(0, void_strcasecmp, compare_values_num_part));
  VERIFY_INT(-1, mm_insert_value(mm, "abc", 123, "def"));
  VERIFY_INT(0, mm_count_keys(mm));
  VERIFY_INT(0, mm_count_values(mm, "abc"));
  VERIFY_INT(0, mm_remove_key(mm, "A"));
  VERIFY_INT(0, mm_get_values(mm, "", arr, 1));
  printf("Nothing printed here:\n");
  // mm_print(mm);
  VERIFY_INT(0, mm_destroy(mm));

  VERIFY_NOT_NULL(mm = mm_create(5, void_strcasecmp, compare_values_num_part));
  VERIFY_INT(1, mm_insert_value(mm, "cccccccccc", 3, "cccccccccccccccccccc"));
  VERIFY_INT(1, mm_insert_value(mm, "bbbbbbbbbbb", 2, "bbbbbbbbbbbbbbbbbbbbb"));
  VERIFY_INT(1, mm_insert_value(mm, "a", 999, "aaa"));
  VERIFY_INT(2, mm_insert_value(mm, "a", 1, "aaaaaaaaaaaaaaaaaaaaa"));
  VERIFY_INT(1, mm_insert_value(mm, "", 4, "ddd"));
  VERIFY_INT(1, mm_insert_value(mm, "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee", 5, "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"));
  VERIFY_INT(2, mm_insert_value(mm, "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee", -999, "ddd"));
  VERIFY_INT(-1, mm_insert_value(mm, ".", 0, "")); // full

  printf("Inspect key ordering:\n");
  // mm_print(mm);
  
  VERIFY_INT(0, mm_count_values(mm, "aa"));
  VERIFY_INT(1, mm_count_values(mm, "bbbbbbbbbbb"));
  VERIFY_INT(1, mm_count_values(mm, "cccccccccc"));
  VERIFY_INT(1, mm_count_values(mm, ""));
  VERIFY_INT(2, mm_count_values(mm, "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"));

  VERIFY_INT(2, mm_get_values(mm, "a", arr, 3));
  VERIFY_INT(1, arr[0].num);
  VERIFY_STR("aaaaaaaaaaaaaaaaaaaaa", arr[0].data);
  VERIFY_INT(999, arr[1].num);
  VERIFY_STR("aaa", arr[1].data);
  VERIFY_INT(0, mm_get_values(mm, "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee", arr, 3));
  VERIFY_INT(2, mm_get_values(mm, "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee", arr, 3));
  VERIFY_INT(-999, arr[0].num);
  VERIFY_STR("ddd", arr[0].data);
  VERIFY_INT(5, arr[1].num);
  VERIFY_STR("eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee", arr[1].data);

  VERIFY_INT(0, mm_get_values(mm, "aaaaaaaaaaa", arr, 0));

  VERIFY_INT(2, mm_remove_key(mm, "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"));
  VERIFY_INT(1, mm_insert_value(mm, ".", 0, "")); // not full
  
  VERIFY_INT(5, mm_count_keys(mm));
  VERIFY_INT(1, mm_count_values(mm, "."));

  VERIFY_INT(1, mm_remove_key(mm, ""));
  
  VERIFY_INT(9, mm_destroy(mm));
}

void test_multiple()
{
  Multimap *mm1, *mm2, *mm3;
  Value arr[3];

  printf("\n*** More than one independent multimap:\n\n");
  
  VERIFY_NOT_NULL(mm1 = mm_create(1, void_strcasecmp, compare_values_num_part));
  VERIFY_NOT_NULL(mm2 = mm_create(2, void_strcasecmp, compare_values_num_part));
  VERIFY_NOT_NULL(mm3 = mm_create(3, void_strcasecmp, compare_values_num_part));
  
  VERIFY_INT(1, mm_insert_value(mm1, "abc", -1, "def"));
  VERIFY_INT(2, mm_insert_value(mm1, "abc", 1, "ghi"));
  VERIFY_INT(1, mm_insert_value(mm3, "abc", 10, "mno"));
  VERIFY_INT(1, mm_insert_value(mm3, "pqr", 100, "stu"));
  
  VERIFY_INT(1, mm_count_keys(mm1));
  VERIFY_INT(0, mm_count_keys(mm2));
  VERIFY_INT(2, mm_count_keys(mm3));

  VERIFY_INT(2, mm_count_values(mm1, "abc"));
  VERIFY_INT(0, mm_count_values(mm1, "jkl"));
  VERIFY_INT(0, mm_count_values(mm2, "abc"));
  VERIFY_INT(1, mm_count_values(mm3, "abc"));
  VERIFY_INT(1, mm_count_values(mm3, "pqr"));
  
  VERIFY_INT(2, mm_get_values(mm1, "abc", arr, 3));
  VERIFY_STR("def", arr[0].data);
  VERIFY_STR("ghi", arr[1].data);
  VERIFY_INT(0, mm_get_values(mm2, "abc", arr, 3));
  VERIFY_INT(1, mm_get_values(mm3, "abc", arr, 3));
  VERIFY_STR("mno", arr[0].data);
  
  VERIFY_INT(1, mm_remove_key(mm3, "abc"));
  VERIFY_INT(2, mm_count_values(mm1, "abc"));
  
  VERIFY_INT(3, mm_destroy(mm1));

  VERIFY_INT(0, mm_count_values(mm2, "abc"));
  VERIFY_INT(0, mm_count_values(mm3, "abc"));
  VERIFY_INT(1, mm_count_values(mm3, "pqr"));

  VERIFY_INT(0, mm_destroy(mm2));
  VERIFY_INT(2, mm_destroy(mm3));
}

void test_invalid()
{
  Multimap *mm;
  Value arr[2];

  printf("\n*** Invalid cases:\n\n");

  VERIFY_NULL(mm = mm_create(10, NULL, compare_values_num_part));
  VERIFY_NULL(mm = mm_create(10, void_strcasecmp, NULL));
  
  // we need a valid multimap for some of these
  VERIFY_NOT_NULL(mm = mm_create(1, void_strcasecmp, compare_values_num_part));
  VERIFY_INT(1, mm_insert_value(mm, "hello", 123, "abc"));
  
  VERIFY_INT(-1, mm_insert_value(mm, NULL, 123, "abc"));
  VERIFY_INT(-1, mm_insert_value(mm, "hello", 123, NULL));
 
  VERIFY_INT(-1, mm_count_keys(NULL));
  VERIFY_INT(-1, mm_count_values(NULL, "hello"));
  VERIFY_INT(-1, mm_count_values(mm, NULL));
  
  VERIFY_INT(-1, mm_get_values(NULL, "hello", arr, 2));
  VERIFY_INT(-1, mm_get_values(mm, NULL, arr, 2));
  VERIFY_INT(-1, mm_get_values(mm, "hello", NULL, 2));
  VERIFY_INT(-1, mm_get_values(mm, "hello", arr, -1));

  VERIFY_INT(-1, mm_remove_key(NULL, "hello"));
  VERIFY_INT(-1, mm_remove_key(mm, NULL));

  VERIFY_INT(-1, mm_destroy(NULL));

  // make sure none of this put us in a bad state
  VERIFY_INT(1, mm_count_keys(mm));
  VERIFY_INT(1, mm_count_values(mm, "hello"));
  VERIFY_INT(2, mm_destroy(mm));
}

void test_get_simple() {
  Multimap *mm;
  void *key;
  
  printf("\n*** Simple typical tests for get:\n\n");

  // These tests are easy because we only care about keys, not values.
  VERIFY_NOT_NULL(mm = mm_create(10, void_strcasecmp, compare_values_num_part));
  VERIFY_INT(1, mm_insert_value(mm, "bbb", 0, ""));
  VERIFY_INT(1, mm_insert_value(mm, "aaa", 0, ""));
  VERIFY_INT(1, mm_insert_value(mm, "ccc", 0, ""));
  VERIFY_INT(2, mm_insert_value(mm, "bbb", 1, ""));

  VERIFY_INT(1, mm_get_first_key(mm, &key));
  VERIFY_STR("aaa", key);
  VERIFY_INT(1, mm_get_first_key(mm, &key));
  VERIFY_STR("aaa", key);
  VERIFY_INT(1, mm_get_next_key(mm, &key));
  VERIFY_STR("bbb", key);
  VERIFY_INT(1, mm_get_next_key(mm, &key));
  VERIFY_STR("ccc", key);
  VERIFY_INT(0, mm_get_next_key(mm, &key));
  VERIFY_INT(-1, mm_get_next_key(mm, &key));
  VERIFY_INT(-1, mm_get_next_key(mm, &key)); // keep trying...

  VERIFY_INT(1, mm_insert_value(mm, "a", 0, ""));
  VERIFY_INT(1, mm_remove_key(mm, "ccc"));
  VERIFY_INT(1, mm_remove_key(mm, "aaa"));

  VERIFY_INT(1, mm_get_first_key(mm, &key));
  VERIFY_STR("a", key);
  VERIFY_INT(1, mm_get_next_key(mm, &key));
  VERIFY_STR("bbb", key);
  VERIFY_INT(0, mm_get_next_key(mm, &key));
  VERIFY_INT(-1, mm_get_next_key(mm, &key));
 
  VERIFY_INT(5, mm_destroy(mm));
}

void test_get_edge() {
  Multimap *mm;
  void *key;
  
  printf("\n*** Edge case tests for get:\n\n");

  VERIFY_NOT_NULL(mm = mm_create(0, void_strcasecmp, compare_values_num_part));
  VERIFY_INT(0, mm_get_first_key(mm, &key));
  VERIFY_INT(0, mm_destroy(mm));

  VERIFY_NOT_NULL(mm = mm_create(4, void_strcasecmp, compare_values_num_part));
  VERIFY_INT(0, mm_get_first_key(mm, &key));

  VERIFY_INT(1, mm_insert_value(mm, "", 0, ""));
  VERIFY_INT(1, mm_insert_value(mm, "----------", 0, ""));
  VERIFY_INT(1, mm_insert_value(mm, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 0, ""));
  VERIFY_INT(1, mm_insert_value(mm, "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz", 0, ""));
  // mm_print(mm);
  VERIFY_INT(1, mm_get_first_key(mm, &key));
  VERIFY_STR("", key);
  VERIFY_INT(1, mm_get_next_key(mm, &key));
  VERIFY_STR("----------", key);
  VERIFY_INT(1, mm_get_next_key(mm, &key));
  VERIFY_STR("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", key);
  VERIFY_INT(1, mm_get_next_key(mm, &key));
  VERIFY_STR("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz", key);
  VERIFY_INT(0, mm_get_next_key(mm, &key));
  VERIFY_INT(-1, mm_get_next_key(mm, &key));
  
  VERIFY_INT(8, mm_destroy(mm));
}

void test_get_move() {
  Multimap *mm;
  void *key;
  
  printf("\n*** Insert and removal tests during get:\n\n");

  VERIFY_NOT_NULL(mm = mm_create(10, void_strcasecmp, compare_values_num_part));
  VERIFY_INT(1, mm_insert_value(mm, "abc", 0, ""));
  VERIFY_INT(1, mm_insert_value(mm, "def", 0, ""));
  VERIFY_INT(1, mm_insert_value(mm, "ghi", 0, ""));
  VERIFY_INT(1, mm_insert_value(mm, "jkl", 0, ""));
  VERIFY_INT(1, mm_insert_value(mm, "mno", 0, ""));
  VERIFY_INT(1, mm_insert_value(mm, "pqr", 0, ""));
  VERIFY_INT(1, mm_insert_value(mm, "stu", 0, ""));
  VERIFY_INT(1, mm_insert_value(mm, "vwx", 0, ""));
  
  // this is tricky to get test properly!
  VERIFY_INT(1, mm_get_first_key(mm, &key));
  VERIFY_STR("abc", key);
  VERIFY_INT(1, mm_remove_key(mm, "abc"));
  VERIFY_INT(1, mm_get_next_key(mm, &key));
  VERIFY_STR("def", key);
  VERIFY_INT(1, mm_get_next_key(mm, &key));
  VERIFY_STR("ghi", key);
  VERIFY_INT(1, mm_remove_key(mm, "def"));
  VERIFY_INT(1, mm_get_next_key(mm, &key));
  VERIFY_STR("jkl", key);
  VERIFY_INT(1, mm_remove_key(mm, "pqr"));
  VERIFY_INT(1, mm_get_next_key(mm, &key));
  VERIFY_STR("mno", key);
  VERIFY_INT(1, mm_remove_key(mm, "stu"));
  VERIFY_INT(1, mm_remove_key(mm, "vwx"));
  VERIFY_INT(0, mm_get_next_key(mm, &key));
  
  VERIFY_INT(6, mm_destroy(mm));
  
  VERIFY_NOT_NULL(mm = mm_create(10, void_strcasecmp, compare_values_num_part));
  VERIFY_INT(1, mm_insert_value(mm, "def", 0, ""));
  VERIFY_INT(1, mm_get_first_key(mm, &key));
  VERIFY_STR("def", key);
  VERIFY_INT(1, mm_insert_value(mm, "abc", 0, ""));
  VERIFY_INT(1, mm_insert_value(mm, "ghi", 0, ""));
  VERIFY_INT(1, mm_get_next_key(mm, &key));
  VERIFY_STR("ghi", key);
  VERIFY_INT(1, mm_insert_value(mm, "jkl", 0, ""));
  VERIFY_INT(1, mm_insert_value(mm, "mno", 0, ""));
  VERIFY_INT(1, mm_get_next_key(mm, &key));
  VERIFY_STR("jkl", key);
  VERIFY_INT(1, mm_get_next_key(mm, &key));
  VERIFY_STR("mno", key);
  VERIFY_INT(0, mm_get_next_key(mm, &key));

  VERIFY_INT(10, mm_destroy(mm));
}

void test_get_multiple() {
  Multimap *mm1, *mm2;
  void *key;

  printf("\n*** Multiple multimaps for get:\n\n");
  VERIFY_NOT_NULL(mm1 = mm_create(10, void_strcasecmp, compare_values_num_part));
  VERIFY_INT(1, mm_insert_value(mm1, "abc", 0, ""));
  VERIFY_INT(1, mm_insert_value(mm1, "def", 0, ""));

  VERIFY_NOT_NULL(mm2 = mm_create(5, void_strcasecmp, compare_values_num_part));
  VERIFY_INT(1, mm_insert_value(mm2, "bbb", 0, ""));
  VERIFY_INT(1, mm_insert_value(mm2, "ccc", 0, ""));
  VERIFY_INT(1, mm_insert_value(mm2, "aaa", 0, ""));
  
  VERIFY_INT(1, mm_get_first_key(mm1, &key));
  VERIFY_STR("abc", key);
  VERIFY_INT(1, mm_get_first_key(mm2, &key));
  VERIFY_STR("aaa", key);
  VERIFY_INT(1, mm_get_next_key(mm2, &key));
  VERIFY_STR("bbb", key);
  VERIFY_INT(1, mm_get_next_key(mm1, &key));
  VERIFY_STR("def", key);
  VERIFY_INT(0, mm_get_next_key(mm1, &key));
  VERIFY_INT(1, mm_get_next_key(mm2, &key));
  VERIFY_STR("ccc", key);
  VERIFY_INT(0, mm_get_next_key(mm2, &key));

  VERIFY_INT(4, mm_destroy(mm1));
  VERIFY_INT(6, mm_destroy(mm2));
}

void test_get_invalid() {
  Multimap *mm;
  void *key;

  printf("\n*** Invalid case tests for get:\n\n");

  VERIFY_NOT_NULL(mm = mm_create(10, void_strcasecmp, compare_values_num_part));
  VERIFY_INT(1, mm_insert_value(mm, "", 0, ""));
  VERIFY_INT(1, mm_insert_value(mm, " ", 0, ""));

  VERIFY_INT(-1, mm_get_first_key(NULL, &key));
  VERIFY_INT(-1, mm_get_first_key(mm, NULL));
  VERIFY_INT(-1, mm_get_next_key(mm, &key));
  VERIFY_INT(1, mm_get_first_key(mm, &key));
  VERIFY_INT(-1, mm_get_next_key(NULL, &key));
  VERIFY_INT(-1, mm_get_next_key(mm, NULL));
  VERIFY_INT(1, mm_get_next_key(mm, &key));
  VERIFY_INT(0, mm_get_next_key(mm, &key));
  VERIFY_INT(-1, mm_get_next_key(mm, &key));

  VERIFY_INT(4, mm_destroy(mm));
}

int main() {
  printf("*** Starting tests...\n");
  
  test_example();
  
  test_typical();
  test_different_types_and_orders();
  test_empty();
  test_edge();
  test_multiple();
#ifdef NDEBUG
  test_invalid();
#endif
  
  // These should have been integrated into the tests above
  test_get_simple();
  test_get_edge();
  test_get_move();
  test_get_multiple();
#ifdef NDEBUG
  test_get_invalid();
#endif
  
  if (0 == Tests_Failed) {
    printf("\nAll %d tests passed.\n", Tests_Passed);
  } else {
    printf("\nFAILED %d of %d tests.\n", Tests_Failed, Tests_Failed+Tests_Passed);
  }
  
  printf("\n*** Tests complete.\n");  
  return 0;
}