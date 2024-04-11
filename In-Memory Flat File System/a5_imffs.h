#ifndef _A5_IMMFS
#define _A5_IMMFS

#include <stdint.h>
#include <stdio.h>

#include "a5_multimap.h"

// Using a typedef'd pointer to the IMFFS struct, for a change
typedef struct IMFFS *IMFFSPtr;



// All functions should print a message in the event of any error, and also return an error code as follows:
//  IMFFS_OK is zero and means that the function was successful;
//  IMFFS_ERROR means that something went wrong and the operation couldn't complete but it was recoverable (e.g. running out of space on the device while saving a file, or renaming a file that doesn't exist or to a filename that already exists).
//  IMFFS_FATAL means that a fatal error occured and the program cannot continue (e.g. running out of memory during malloc);
//  IMFFS_INVALID means that the function was called incorrectly (e.g. a NULL parameter where a string was required);
//  IMFFS_NOT_IMPLEMENTED means that the function is not implemented.
// Not all functions need to use all of these return codes. The exact choice of what value is most appropriate for any given situation is up to you.

typedef enum {
  IMFFS_OK = 0,
  IMFFS_ERROR = 1,
  IMFFS_FATAL = 2,
  IMFFS_INVALID = 3,
  IMFFS_NOT_IMPLEMENTED = 4
} IMFFSResult;



// this function will create the filesystem with the given number of blocks;
// it will modify the fs parameter to point to the new file system or set it
// to NULL if something went wrong (fs is a pointer to a pointer)
/*
1. create the huge array where I can store 256 x block_count bytes using 
2. fs in this case is a pointer to a pointer, so we are 
        pointing to a location that has the address to the location of the device.
        So we dereference and then say  that the pointer that directly points at the device will be malloc'd

* I am going to create the free array with the same size as the #of blocks that we have for the device
* I am going to create an array that has all of the indices that device has

Tests:

 * Test if not null
 * Test the size of the free array
 * Test that everything starts out with a 1 for the free array
 * Test if mm is not null
 * Test if mm keys_array size is the number of blocks
 * Test if the free array works properly
      * Simulating adding.
*/
IMFFSResult imffs_create(uint32_t block_count, IMFFSPtr *fs);

Multimap *checking_multimap_creation(IMFFSPtr fs);
uint8_t *checking_device_creation(IMFFSPtr fs);
char *checking_free_array_creation(IMFFSPtr fs);
int checking_max_bytes(IMFFSPtr fs);
int get_all_value_nodes(IMFFSPtr fs, char *imffsfile);
int get_total_blocks_used(IMFFSPtr fs, char *imffsfile);
IMFFSResult looking_for_key(IMFFSPtr fs, char *imffsfile);

// save diskfile imffsfile copy from your system to IMFFS
/**
 * fread(): size_t fread(void * buffer, size_t size, size_t count, FILE * stream);
                  * void* buffer: This is the device, the blocks (256 bytes/block)
                  * size_t size: sizeof(data_type); Maybe sizeof(char) (1 byte)
                  * size_t count: #elements we want to read in.
                  * FILE* stream: from where we are reading data
                  * return size_t: #elements read successfully
                              * If the return value is < that the (#elements we want to read in)
                                      * an error occured, or we have reached the end of the file. 
 * 
 * Approach:
      * Most important thing is the number of blocks I use 
      * I guess I can just read in 256 bytes every time
      * As long as the return value is 256, I will keep reading, until the return value is < 256
      * I have to make sure that I search for the next available block before adding more.
              * I am going to use the mathematical formula
                    * 256 * the index of the free block
                        * For example:
                            * If the free block was the third block, its index would be 2
                                  * 256 x 2 = 512. That would the very first index in the device 
                                  * where we start adding.        
      * I have to keep track of how many blocks I have used
      * I gotta make sure I make the right end of the last byte in the file read in.
      * For marking the block to be occupied, I am just going to look for the free block ("1"), 
      * take 256 * its index in the free array, then mark that block as "0".
      * How do I know where the last block to be occupied is?
            * Each value node will know how many blocks it has
                    * Meaning: 
                      * I know the starting_index: 256 * index_of_first_free_block
                      * I know the ending_index of that chunk: [(256 x #blocks chunk has) - 1] + starting_index   
      * I am going to create another array that has all of the indices in the device so that I know what the starting index is 
        * Then I am going to have the pointer of each value node to point at the index
        * which represents the first 1 byte slot of a block of that chunk.
 * 
 * Tests:
      * Manually check: the data readin
      * Check if the number of blocks occupied is correct
 * 
 * char* diskfile: the file we have
 * char* imffsfile: the file in the device
 * IMFFSptr fs: the device pointer
 * 
*/
IMFFSResult imffs_save(IMFFSPtr fs, char *diskfile, char *imffsfile);

int get_total_bytes_read_in(IMFFSPtr fs, char *imffsfile);

// load imffsfile diskfile copy from IMFFS to your system
/**
 * size_t fwrite(const void *ptr, size_t size, size_t count, FILE *stream);
                * void* ptr: from where we get the data
                * size_t size: size of each element in bytes
                * size_t count: the number of elements to be written
                *  FILE *stream: where to write to
                * 
  * Approach:
      * SO I know of each chunk:
          * I know the starting_index: 256 * index_of_first_free_block
          * I know the ending_index of that chunk: [(256 x #blocks chunk has) - 1] + starting_index
      * I can just write the information to the file 1 chunk at the time.   
  
  
  * Tests:
      * Manually check the file's contents
      * 
  * 
  * 
  * char* imffsfile: the file in the device
  * char* diskfile: the file we have
  * IMFFSPtr fs: the device pointer
  * 
*/
IMFFSResult imffs_load(IMFFSPtr fs, char *imffsfile, char *diskfile);

// delete imffsfile remove the IMFFS file from the system, allowing the blocks to be used for other files
/**
 * Approach:
   * Find the file that I want to delete in the mm
   * I have to free the value nodes first, along with marking the blocks to be free
   * then free the location where you malloc'd for the file
   * then free the pointer in mm(?)
   * How would I convert from the starting_index, and the #blocks to which blocks to free for each chunk?
      * starting_index: where the pointer is pointing at
      * ending_index:   [(256 x #blocks chunk has) - 1] + starting_index
          *From the starting_index: i can tell: the first block index is: starting_index / 256
                                                  The last block index: (ending_index + 1/ 256) -1 
                                                      or, I can just go with the number of blocks that the chunkn has

 * 
 * 
 * Test:
      * Checking if those blocks are available 
 * 
 * 
*/
IMFFSResult imffs_delete(IMFFSPtr fs, char *imffsfile);

// rename imffsold imffsnew rename the IMFFS file from imffsold to imffsnew, keeping all of the data intact
/**
 * Approach:
    * Go to the file in the mm, then update the name
 * 
 * Test:
    *VERIFY string: expected new name, curr name 
 * 
*/
IMFFSResult imffs_rename(IMFFSPtr fs, char *imffsold, char *imffsnew);

// dir will list all of the files and the number of bytes they occupy
/**
 * 
 * Approach:
    * Print only the files in the mm
*/
IMFFSResult imffs_dir(IMFFSPtr fs);

// fulldir is like "dir" except it shows a the files and details about all of the chunks they are stored in (where, and how big)
/**
 * Approach:
    * print the files and the values from the mm
*/
IMFFSResult imffs_fulldir(IMFFSPtr fs);

// defrag will defragment the filesystem: if you haven't implemented it, have it print "feature not implemented" and return IMFFS_NOT_IMPLEMENTED
IMFFSResult imffs_defrag(IMFFSPtr fs);

// quit will quit the program: clean up the data structures
/**
 * Approach: 
    * Have a temp ptr to the first array slot in the device, so that we can free the entire device
    * Free each value node first, (we don't need to free the array that checks which block is free)
    * Free the files
    * Free the mm
    * Free the device
    * 
  * Tests:
    * Everything is null 
*/
IMFFSResult imffs_destroy(IMFFSPtr fs);

#endif