#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "a5_multimap.h"

typedef struct VALUE_NODE {
  Value value;
  struct VALUE_NODE *next;
} ValueNode;


typedef struct KEY_AND_VALUES {
  void* key; //This data is in IMFFS, and can only be freed from there
  int num_values; //Number of chunks that the key has
  ValueNode *head;
} KeyAndValues;


struct MULTIMAP {
  int num_keys;
  int max_keys;

  //These are the 2 functions we use to tell our mm how to compare the keys and values of different types
  Compare check_the_keys; 
  Compare check_the_values;

  KeyAndValues *keys;

  // NEW: traversal position, for the get_keys functions
  int trav_pos;
};


// Helper functions
static int find_key_pos(void *key, int num_keys, Multimap* mm);
static int insert_key_alphabetically(Multimap* mm, int keys_length, void *key);
static int insert_value_by_data(KeyAndValues *key, ValueNode *node, Multimap* mm);

// This gets rid of the warning about an unused function when assertions are off.
// It wouldn't be a problem if your function isn't "static" (which it doesn't have to be).
#ifndef NDEBUG
static int validate_multimap(Multimap *mm)
{
  //If the mm has been created successfully
  assert(NULL != mm);
  //If the keys have been created successfully
  assert(NULL != mm->keys);
  //If the number of max_keys is valid
  assert(mm->max_keys >= 0);
  //If the number of keys in the mm is valid 
  assert(mm->num_keys >= 0 && mm->num_keys <= mm->max_keys);
  // NEW
  //To keep track of traversing
  assert(mm->trav_pos >= -1 && mm->trav_pos <= mm->max_keys);
  
  // Those were the easy/efficient ones, here is the tricky part to check the entire structure
  ValueNode *curr, *prev;
  int count;
  //Going through all the keys currently in the mm
  for (int i = 0; i < mm->num_keys; i++) {

    //Don't think I need this anymore, because the keys data is out of mm's jurisdiction
    //assert(strlen(mm->keys[i].key) < MAX_KEY_LENGTH);
    assert(mm->keys[i].num_values > 0); // can't have a key with no values
    assert(NULL != mm->keys[i].head); //And since a key must have at least 1 value, the head of the linkedlist must not be null
    
    
    count = 0;
    curr = mm->keys[i].head;
    prev = NULL;
    while (NULL != curr) {
      count++;
      //We no longer care about the str in the value, because we now have a void*
      // assert(strlen(curr->value.str) < MAX_VALUE_LENGTH);
      if (prev != NULL) {
        // ordering
        //we have to compare by data
        // assert(prev->value.num <= curr->value.num);
        assert(mm->check_the_values(&prev->value, &curr ->value) <= 0);
      }
      prev = curr;
      curr = curr->next;
    }
    //making sure that the multimap doesn't miscount
    assert(count == mm->keys[i].num_values);
  }
  
  return 1; // always return TRUE
}
#endif

int get_max(Multimap* mm){
  return mm->max_keys;
}

Multimap *mm_create(int max_keys, Compare compare_keys, Compare compare_values)
{
  assert(max_keys >= 0);

  //The function pointers cannot be NULL
  assert(compare_keys != NULL && compare_values != NULL);
  Multimap *mm = NULL;


  if(compare_keys != NULL && compare_values != NULL){


    
    if (max_keys >= 0) {
      mm = malloc(sizeof(Multimap));
      if (NULL != mm) {
        mm->keys = malloc(max_keys * sizeof(KeyAndValues));
        if (NULL == mm->keys) {
          free(mm);
          mm = NULL;
        } else {
          mm->max_keys = max_keys;
          mm->num_keys = 0;
          // NEW
          mm->trav_pos = -1;
          //Giving the multimap the ability to compare things
          mm->check_the_keys = compare_keys;
          mm->check_the_values = compare_values;
        }
      }
    }
    

    assert(validate_multimap(mm));
  }

  return mm;
}



int mm_insert_value(Multimap *mm, void *key, int value_num, void* value_data)
{
  assert(validate_multimap(mm));
  assert(NULL != key);
  assert(value_data != NULL);
  
  int result = -1;
  int pos;
  ValueNode *node;

  if (NULL != mm && NULL != key && NULL != value_data) {
    pos = find_key_pos(key, mm->num_keys, mm);
    if (pos < 0 && mm->num_keys < mm->max_keys) {

      pos = insert_key_alphabetically(mm, mm->num_keys, key);
      assert(pos >= 0 && pos < mm->max_keys);
      mm->num_keys++;
      
      // NEW
      if (pos < mm->trav_pos && mm->trav_pos < mm->max_keys) {
        mm->trav_pos++;
      }
    }

    if (pos >= 0) {
      assert(pos < mm->num_keys);
      // key was either already there, or successfully added
      
      node = malloc(sizeof(ValueNode));
      if (NULL != node) {
        node->value.num = value_num;

        node->value.data = value_data;

        result = insert_value_by_data(&mm->keys[pos], node, mm);
      }

      assert (result > 0);
    }
  }
  
  assert(validate_multimap(mm));
  return result;
}



int mm_count_keys(Multimap *mm)
{
  //Forcing the mm to always be correct
  assert(validate_multimap(mm));
  

  int count = -1;
  
  //if the mm exists, the number of keys is already known
  if (NULL != mm) {
    count = mm->num_keys;
  }
  
  //fOrcing that nomatter what the count will be >=1, and that the count cannot exceed max_keys
  assert(count >= -1 && count <= mm->max_keys);
  return count;
}


int mm_count_values(Multimap *mm, void *key)
{
  //Making sure the mm follows the logic
  assert(validate_multimap(mm));

  //Making sure that the key where we want to count values at is not NULL
  assert(NULL != key);
  
  int count = -1;
  int pos;
  
  //if the mm exists, and the key is valid
  if (NULL != mm && NULL != key) {
    //Start from 0 for the counting
    count = 0;
    //Find the key in the mm, to see if the mm has that key
    pos = find_key_pos(key, mm->num_keys, mm);

    //if yes, 
    if (pos >= 0) {
      //Forcing that the pos returned must be a valid index 
      assert(pos < mm->num_keys);

      //and the key must always knows how many chunks it has
      count = mm->keys[pos].num_values;
    }
  }
  
  //Forcing that the count is >=-1
  assert(count >= -1);
  return count;
}



int mm_get_values(Multimap *mm, void *key, Value values[], int max_values)
{
  //Forcing that the mm is valid
  assert(validate_multimap(mm));

  //Forcing that we are not getting the values at a null key
  assert(NULL != key);

  //Forcing that the array to store all the values is also valid enough to store them
  assert(NULL != values);
  //Forcing that the amount of values to be taken is valid 
  assert(max_values >= 0);
  
  int count = -1;
  int pos;
  ValueNode *node;
  
  //If the mm is valid, the key to get the values is valid, the array to hold the values is valid, and the number to get is valid,
  if (NULL != mm && NULL != key && NULL != values && max_values >= 0) {
    count = 0;

    //We find that key in the mm
    pos = find_key_pos(key, mm->num_keys, mm);

    //If found,
    if (pos >= 0) {
      //Forcing that the pos returned must be a valid index 
      assert(pos < mm->num_keys);

      //storing the head of the linkedlist in a temp ValueNode*
      node = mm->keys[pos].head;

      //While there is still more values to get, and that the amount taken is not sufficed,
      while (NULL != node && count < max_values) {
        //in the slot of the array, we store the value
        values[count] = node->value;
        
        //Go to the next slot in the array
        count++;

        //Go to the next value
        node = node->next;
      }
    }
  }
  
  //Making sure that after that operation, the mm still maintains its logic
  assert(validate_multimap(mm));

  //Forcing that the count >= -1
  assert(count >= -1);
  return count;

}



int mm_remove_key(Multimap *mm, void *key)
{
  //Forcing that mm still maintains its logic 
  assert(validate_multimap(mm));

  //Forcing that the key that we want to remove is valid 
  assert(NULL != key);
  

  int count = -1;
  int pos;
  ValueNode *curr, *next;
  
  //If the mm is valid, and the key is valid
  if (NULL != mm && NULL != key) {
    count = 0;

    //Look for the key in the mm
    pos = find_key_pos(key, mm->num_keys, mm);

    //If found,
    if (pos >= 0) {
      //Forcing the pos to be a valid index
      assert(pos < mm->num_keys);
      
      count = 0;
      
      // free the list
      //Starting with the head,
      curr = mm->keys[pos].head;
      //While there are still value nodes,
      while (NULL != curr) {
        //Store the next node's address
        next = curr->next;
        //Free the prev
        free(curr);
        //Change to the next
        curr = next;
        //Increase the count 
        count++;
      }

      // move keys up by one starting from the immediate next key
      for (int i = pos + 1; i < mm->num_keys; i++) {
        mm->keys[i - 1] = mm->keys[i];
      }
      mm->num_keys--;//Then decrease the amount of keys

      // NEW
      //SEND HELP
      //I don't understand this 
      if (pos+1 <= mm->trav_pos && mm->trav_pos > 0) {
        mm->trav_pos--;
      }
    }
  }
  
  //Forcing that after the operation, the mm still has its logic
  assert(validate_multimap(mm));
  assert(count >= -1);
  return count;
}

void mm_print(Multimap *mm)
{
  //Forcing that the mm is still valid
  assert(validate_multimap(mm));

  ValueNode *node;

  //If there is a mm,
  if (NULL != mm) {
    printf("Printing\n");
    printf("The number of keys is: %d\n", mm->num_keys);
    //For every key,
    for (int i = 0; i < mm->num_keys; i++) {
      //SEND HELP
      //Since the key is now a void* to a struct, we just have to cast it then access the fields
      //FileInformation* key_info = (FileInformation*) mm->keys[i].key;
      printf("[%3d] '%s' (%d)\n", i, (char*)mm->keys[i].key, mm->keys[i].num_values);

      //Go to the head of the linkedlist to get the values
      node = mm->keys[i].head;
      //While there are still value nodes
      while (NULL != node) {
        //SEND HELP
        //Here is a problem as well. How do I print this value.data? It has the address of the first block
        printf(" %9d\n", node->value.num);
        node = node->next;
      }
    }
  }  

  assert(validate_multimap(mm));
}

int mm_destroy(Multimap *mm)
{
  int count = -1;

  //Forcing the mm to follow its logic
  assert(validate_multimap(mm));
  
  ValueNode *node, *next;

  //If the mm exists, 
  if (NULL != mm) {
    //Get all the number of keys,
    count = mm->num_keys;

    //For each key,
    for (int i = 0; i < mm->num_keys; i++) {
      
      //Get the head of the linkedlist of nodes of each key
      node = mm->keys[i].head;

      //If there are still nodes
      while (NULL != node) {
        //Store address of the next
        next = node->next;
        //Free the current
        free(node);

      
        count++;
      
        //Go to the next node
        node = next;
      }
    }
    //Then free the keys after the nodes are removed
    free(mm->keys);
    
    // set everything to zero, to help catch a dangling pointer error
    mm->num_keys = 0;
    mm->max_keys = 0;
    mm->keys = NULL;
    
    //After the value nodes, the keys are removed, remove the mm as well
    free(mm);
  }  
  
  return count;
}

/*** NEW ***/

int mm_get_first_key(Multimap *mm, void **key)
{
  //forcing that the mm is still logical 
  assert(validate_multimap(mm));
  //Forcing that the place to store the key is not null
  assert(NULL != key);

  //No need to check for lengths anymore  
  //assert(length > 0);
  
  int result = 0;
  
  //If either the mm is null or the place to store the first key is null, it is an error
  if (NULL == mm || NULL == key) {
    // If we get here, assertions are off, so there are no postconditions to skip
    return -1;
  }
  
  //If we have at least 1 element
  if (mm->num_keys > 0) {
    //SEND HELP
    // strncpy(key, mm->keys[0].key, length);
    // key[length-1] = '\0';
    //Point at a different key
    *key = mm->keys[0].key;


    result = 1;
    mm->trav_pos = 1;
  } else {
    mm->trav_pos = -1;
  }
  
  assert(result >= -1 && result <= 1);
  assert(validate_multimap(mm));

  return result;
}

int mm_get_next_key(Multimap *mm, void **key)
{
  assert(validate_multimap(mm));
  assert(NULL != key);

  int result = 0;
  
  if (NULL == mm || NULL == key) {
    return -1;
  }

  if (mm->trav_pos < mm->num_keys && mm->trav_pos >= 0) {
    // printf("%d\n", mm->trav_pos);
    //SEND HELP
    // strncpy(key, mm->keys[mm->trav_pos].key, length);
    // key[length-1] = '\0';
    //Point to a different key
    *key = mm->keys[mm->trav_pos].key;
    result = 1;
    mm->trav_pos++;
  } else {
    if (mm->trav_pos < 0) {
      // Attempted to call get_next when the previous call would have failed
      result = -1;
    }
    mm->trav_pos = -1;
  }
  
  assert(result >= -1 && result <= 1);
  assert(validate_multimap(mm));

  return result;
}

static int find_key_pos(void *key, int num_keys, Multimap* mm)
{
  assert(validate_multimap(mm));
  assert(NULL != key);
  assert(num_keys >= 0);
  
  int pos = -1;

  if(mm != NULL){
    int start = 0, end = num_keys - 1;
    int mid, comp;
    
    while (start <= end && pos < 0) {
      mid = (end - start) / 2 + start;
      
      //Gotta use the function pointer here
      comp = mm->check_the_keys(key, mm->keys[mid].key);

      if (comp < 0) {
        end = mid - 1;
      } else if (comp > 0) {
        start = mid + 1;
      } else {
        pos = mid;
      }
    }
  }

  
  return pos;
}

static int insert_key_alphabetically(Multimap* mm, int keys_length, void *key) {
  assert(validate_multimap(mm));
  assert(keys_length >= 0);
  assert(NULL != key);

  int pos = 0;

  if(mm!= NULL){
    while (pos < keys_length && mm->check_the_keys(key, mm->keys[pos].key) >= 0) {
      pos++;
    }
    
    if (pos < keys_length) {
      // shift everything over: a for loop is ok but this is more efficient
      memmove(&mm->keys[pos+1], &mm->keys[pos], (keys_length - pos) * sizeof(KeyAndValues));
    }
    
    //This would make the key at the pos to point at the key I want to insert
    mm->keys[pos].key = key;

    mm->keys[pos].num_values = 0;
    mm->keys[pos].head = NULL;
  }
  
  
  return pos;
}

static int insert_value_by_data(KeyAndValues *key, ValueNode *node, Multimap* mm) {
  assert(NULL != key);
  assert(NULL != node);

  ValueNode *curr = key->head;
  ValueNode *prev = NULL;
  
  // while (NULL != curr && mm->check_the_values(&(curr->value), &(node->value)) <0) {
  while(NULL != curr && mm->check_the_values(&node->value, &curr->value) >0){
    prev = curr;
    curr = curr->next;
  }
  
  if (NULL == prev) {
    node->next = key->head;
    key->head = node;
  } 
  else {
    node->next = curr;
    prev->next = node;
  }
  
  key->num_values++;

  return key->num_values;
}


