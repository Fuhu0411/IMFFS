#ifndef _A5_MULTIMAP
#define _A5_MULTIMAP

// NEW: instead of storing string in the multimap, both the keys and values
//      are void pointers to data allocated elsewhere in the program

//int num: number of blocks in this chunk
//void* data: the address of the first block in this whole chunk
typedef struct VALUE { int num; void *data; } Value;
typedef struct MULTIMAP Multimap; // you need to define this yourself



// We can use this typedef to make the function headers more readable:
typedef int (*Compare)(void *a, void *b);

int get_max(Multimap* mm);

// int compare_keys_as_strings_case_insensitive(void *a, void *b);

// int compare_values_num_part(void *a, void *b);
// Create a new multimap with at most "max_keys" keys. Keys will be ordered
//  and compared using the "compare_keys" function. Values will be ordered
//  and compared using the "compare_values" function.
// NEW: the first function pointer determines key comparison/ordering
//      and the second function pointer determines value ordering.
//      The compare_keys function is passed the key pointer.
//      The compare_values function is passed the **entire Value struct**.
// Return NULL on error.
Multimap *mm_create(int max_keys, Compare compare_keys, Compare compare_values);

// or, without the typedef:
// Multimap *mm_create(int max_keys, int (*compare_keys)(void *key1, void *key2), int (*compare_values)(void *value1, void *value2));

// Insert a new value into the multimap for the given key.
// If the key already exists in the multimap, the value is added to that key.
// Return the number of values associated with this key after insertion.
int mm_insert_value(Multimap *mm, void *key, int value_num, void *value_data);

// Count the number of keys in the multimap.
int mm_count_keys(Multimap *mm);

// Count the number of values associated with the given key.
// Zero means the key is not found in the multimap.
int mm_count_values(Multimap *mm, void *key);

// Copy as many values as possible for the given key into the array.
// Values will be ordered (and copied) numerically.
// Note that while values[] contains copies of the structs, the str pointers
//  still point to memory that belongs to the Multimap. Do not free or modify!
// Return the number of values copied, at most max_values.
int mm_get_values(Multimap *mm, void *key, Value values[], int max_values);

// Remove the key and all its corresponding values from the multimap.
// Return the number of values that were removed with the key.
// Zero means the key is not found in the multimap.
int mm_remove_key(Multimap *mm, void *key);

// Print the contents of the multimap, neatly.
// Keys and values must be in the correct order.
// This will be helpful for debugging and manual testing.
void mm_print(Multimap *mm);

// Destroy a multimap, freeing all allocated memory.
// Return the total number of keys AND values that were freed.
int mm_destroy(Multimap *mm);

// Using these functions together will allow you iterate over the keys in the
// multimap, in the order that they are stored (that is, sorted).

// They can be used as follows:

// void *key;
// if (mm_get_first_key(mm, &key) > 0) {
//   do {
//     // process the key
//   } while (mm_get_next_key(mm, &key) > 0);
// }

// Consider what happens if mm_remove_key() is called (possibly more than
// once) as part of processing a key.
// NEW: it is safe to free() the key after removing it from the multimap

// Copy the first key pointer in the multimap into the pointer **key.
// Returns -1 on error, 0 if there were no more keys, or 1 on success.
// If 0 or -1 are returned, then the contents of key are unchanged.
int mm_get_first_key(Multimap *mm, void **key);

// Same as above, except it copies the next key following a call to either
// mm_get_first_key or mm_get_next_key.
int mm_get_next_key(Multimap *mm, void **key);

#endif
