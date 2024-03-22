#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "a4_tests.h"
#include "a5_imffs.h"




void testing_creating_imffs(){
    printf("TESTING CREATING IMFFS:\n");

    IMFFSPtr ptr_to_imffs;
    int i = imffs_create(10, &ptr_to_imffs);
    VERIFY_INT(IMFFS_OK, i);
    
    //Checking the creation of the entire imffs
    VERIFY_NOT_NULL(ptr_to_imffs);

    //Checking the creation of the mm
    VERIFY_NOT_NULL(checking_multimap_creation(ptr_to_imffs));

    //Checking the creation of the device
    VERIFY_NOT_NULL(checking_device_creation(ptr_to_imffs));

    //Checking the creation of the free_array
    VERIFY_NOT_NULL(checking_free_array_creation(ptr_to_imffs));
    //Checking that the free_array is correctly created);
    VERIFY_STR("1111111111", checking_free_array_creation(ptr_to_imffs));

    //Checking the max_bytes;
    VERIFY_INT(10*256, checking_max_bytes(ptr_to_imffs));

    //Need destroy function:
    VERIFY_INT(IMFFS_OK, imffs_destroy(ptr_to_imffs));
}

void testing_creating_imffs_error_case(){
    #ifdef NDEBUG
        printf("\nTESTING IMFFS CREATION: ERROR CASES\n");

        IMFFSPtr ptr_to_imffs;
        //this should crash the program
        int i = imffs_create(1000000000, &ptr_to_imffs);
        VERIFY_INT(IMFFS_FATAL, i);
        
        //Checking the creation of the entire imffs
        VERIFY_NULL(ptr_to_imffs);

        //Checking the creation of the mm
        VERIFY_NULL(checking_multimap_creation(ptr_to_imffs));

        //Checking the creation of the device
        VERIFY_NULL(checking_device_creation(ptr_to_imffs));

        //Checking the creation of the free_array
        VERIFY_NULL(checking_free_array_creation(ptr_to_imffs));


        //Checking the max_bytes;
        VERIFY_INT(IMFFS_INVALID, checking_max_bytes(ptr_to_imffs));

        //Need destroy function:
        VERIFY_INT(IMFFS_INVALID, imffs_destroy(ptr_to_imffs));
        VERIFY_NULL(ptr_to_imffs);
        VERIFY_NULL(checking_device_creation(ptr_to_imffs));
        VERIFY_NULL(checking_multimap_creation(ptr_to_imffs));
        VERIFY_NULL(checking_free_array_creation(ptr_to_imffs));
    #endif 
}


/**
 * When 0 size device
 * When 1 size device, read in 256 bytes
 * When 1 size device, read in more than 256 bytes
 * When 2 size device, read in 1 full block, then read in more than 256 bytes (the remaining block won't be able to hold)
 * Check the free_array as well
*/

void testing_save_256_1_block(){
    printf("\nSaving 256 in 1 block:\n\n");

    int block_count =1;

    IMFFSPtr fs;
    VERIFY_INT(IMFFS_OK, imffs_create(block_count, &fs));

    //Checking the creation of the entire imffs
    VERIFY_NOT_NULL(fs);

    //Checking the creation of the mm
    VERIFY_NOT_NULL(checking_multimap_creation(fs));

    //Checking the creation of the device
    VERIFY_NOT_NULL(checking_device_creation(fs));

    //Checking the creation of the free_array
    VERIFY_NOT_NULL(checking_free_array_creation(fs));
    //Checking that the free_array is correctly created);
    VERIFY_STR("1", checking_free_array_creation(fs));
    
    //Checking the max_bytes;
    VERIFY_INT(block_count*256, checking_max_bytes(fs));


    //Saving files TO THE FRONT, and nothing obstructing
//FILE A
    printf("\nFile A\n");
    VERIFY_INT(IMFFS_ERROR, imffs_save(fs, "test_file_for_save_256_bytes.txt", "fileA.txt"));
    //Verify the number of keys that the mm has
    VERIFY_INT(0, mm_count_keys(checking_multimap_creation(fs)));
    //Verify the total number of chunks (value nodes)
    VERIFY_INT(0, get_all_value_nodes(fs, "fileA.txt"));
    //Verify the total number of blocks from all the chunks
    VERIFY_INT(0, get_total_blocks_used(fs, "fileA.txt"));
    //check the free_array if things are marked correctly
    VERIFY_STR("1", checking_free_array_creation(fs));

    //VERIFY_INT(IMFFS_INVALID, imffs_load(fs, "fileA.txt", "fileA.txt"));
    
    //Loading
    printf("\n");   
}

void testing_save_0_1_block(){
    printf("\nSaving 0 in 1 block:\n\n");

    int block_count =1;

    IMFFSPtr fs;
    VERIFY_INT(IMFFS_OK, imffs_create(block_count, &fs));

    //Checking the creation of the entire imffs
    VERIFY_NOT_NULL(fs);

    //Checking the creation of the mm
    VERIFY_NOT_NULL(checking_multimap_creation(fs));

    //Checking the creation of the device
    VERIFY_NOT_NULL(checking_device_creation(fs));

    //Checking the creation of the free_array
    VERIFY_NOT_NULL(checking_free_array_creation(fs));
    //Checking that the free_array is correctly created);
    VERIFY_STR("1", checking_free_array_creation(fs));
    
    //Checking the max_bytes;
    VERIFY_INT(block_count*256, checking_max_bytes(fs));


    //Saving files TO THE FRONT, and nothing obstructing
//FILE A
    printf("\nFile A\n");
    VERIFY_INT(IMFFS_OK, imffs_save(fs, "test_file_for_save_0_bytes.txt", "fileA.txt"));
    //Verify the number of keys that the mm has
    VERIFY_INT(1, mm_count_keys(checking_multimap_creation(fs)));
    //Verify the total number of bytes read in
    VERIFY_INT(0, get_total_bytes_read_in(fs, "fileA.txt"));
    //Verify the total number of chunks (value nodes)
    VERIFY_INT(1, get_all_value_nodes(fs, "fileA.txt"));
    //Verify the total number of blocks from all the chunks
    VERIFY_INT(1, get_total_blocks_used(fs, "fileA.txt"));
    //check the free_array if things are marked correctly
    VERIFY_STR("0", checking_free_array_creation(fs));

    //DELETEING files
    //Look for the key in the multimap
    VERIFY_INT(IMFFS_OK, looking_for_key(fs, "fileA.txt"));
    //Delete
    VERIFY_INT(IMFFS_OK, imffs_delete(fs, "fileA.txt"));
    VERIFY_INT(IMFFS_ERROR, imffs_delete(fs, "fileA.txt"));
    //Checking the tree
    VERIFY_STR("1", checking_free_array_creation(fs));
    //Checking the mulitmap
    VERIFY_INT(0, mm_count_keys(checking_multimap_creation(fs)));
    //Look for the key in the multimap
    VERIFY_INT(IMFFS_ERROR, looking_for_key(fs, "fileA.txt"));
    printf("\n");
    
    //Loading
    printf("\n");   
}

void testing_complete(){
    printf("\nSaving 5 blocks:\n\n");

    int block_count =5;

    IMFFSPtr fs;
    VERIFY_INT(IMFFS_OK, imffs_create(block_count, &fs));

    //Checking the creation of the entire imffs
    VERIFY_NOT_NULL(fs);

    //Checking the creation of the mm
    VERIFY_NOT_NULL(checking_multimap_creation(fs));

    //Checking the creation of the device
    VERIFY_NOT_NULL(checking_device_creation(fs));

    //Checking the creation of the free_array
    VERIFY_NOT_NULL(checking_free_array_creation(fs));
    //Checking that the free_array is correctly created);
    VERIFY_STR("11111", checking_free_array_creation(fs));
    
    //Checking the max_bytes;
    VERIFY_INT(block_count*256, checking_max_bytes(fs));

    printf("\nAdding files size 256\n");
    printf("\nFile A\n");
    VERIFY_INT(IMFFS_OK, imffs_save(fs, "test_file_for_save_256_bytes.txt", "fileA.txt"));
    //Verify the number of keys that the mm has
    VERIFY_INT(1, mm_count_keys(checking_multimap_creation(fs)));
    //Verify the total number of bytes read in
    VERIFY_INT(257, get_total_bytes_read_in(fs, "fileA.txt"));
    //Verify the total number of chunks (value nodes)
    VERIFY_INT(1, get_all_value_nodes(fs, "fileA.txt"));
    //Verify the total number of blocks from all the chunks
    VERIFY_INT(2, get_total_blocks_used(fs, "fileA.txt"));
    //check the free_array if things are marked correctly
    VERIFY_STR("00111", checking_free_array_creation(fs));
    VERIFY_INT(IMFFS_OK, looking_for_key(fs, "fileA.txt"));

    printf("\nFile B\n");
    VERIFY_INT(IMFFS_OK, imffs_save(fs, "test_file_for_save_256_bytes.txt", "fileB.txt"));
    //Verify the number of keys that the mm has
    VERIFY_INT(2, mm_count_keys(checking_multimap_creation(fs)));
    //Verify the total number of bytes read in
    VERIFY_INT(257, get_total_bytes_read_in(fs, "fileB.txt"));
    //Verify the total number of chunks (value nodes)
    VERIFY_INT(1, get_all_value_nodes(fs, "fileB.txt"));
    //Verify the total number of blocks from all the chunks
    VERIFY_INT(2, get_total_blocks_used(fs, "fileB.txt"));
    //check the free_array if things are marked correctly
    VERIFY_STR("00001", checking_free_array_creation(fs));
    VERIFY_INT(IMFFS_OK, looking_for_key(fs, "fileB.txt"));

    printf("\nBroken file\n");
    VERIFY_INT(IMFFS_ERROR, imffs_save(fs, "test_file_for_save_1024_bytes.txt", "fileM.txt"));
    //Verify the number of keys that the mm has
    VERIFY_INT(2, mm_count_keys(checking_multimap_creation(fs)));
    //Verify the total number of bytes read in
    VERIFY_INT(IMFFS_ERROR, get_total_bytes_read_in(fs, "fileM.txt"));
    //Verify the total number of chunks (value nodes)
    VERIFY_INT(0, get_all_value_nodes(fs, "fileM.txt"));
    //Verify the total number of blocks from all the chunks
    VERIFY_INT(0, get_total_blocks_used(fs, "fileM.txt"));
    //check the free_array if things are marked correctly
    VERIFY_STR("00001", checking_free_array_creation(fs));
    VERIFY_INT(IMFFS_ERROR, looking_for_key(fs, "fileM.txt"));


    printf("\nFile C\n");
    VERIFY_INT(IMFFS_OK, imffs_save(fs, "test_file_for_save_0_bytes.txt", "fileC.txt"));
    //Verify the number of keys that the mm has
    VERIFY_INT(3, mm_count_keys(checking_multimap_creation(fs)));
    //Verify the total number of bytes read in
    VERIFY_INT(0, get_total_bytes_read_in(fs, "fileC.txt"));
    //Verify the total number of chunks (value nodes)
    VERIFY_INT(1, get_all_value_nodes(fs, "fileC.txt"));
    //Verify the total number of blocks from all the chunks
    VERIFY_INT(1, get_total_blocks_used(fs, "fileC.txt"));
    //check the free_array if things are marked correctly
    VERIFY_STR("00000", checking_free_array_creation(fs));
    VERIFY_INT(IMFFS_OK, looking_for_key(fs, "fileC.txt"));

    printf("\nFile D\n");
    VERIFY_INT(IMFFS_ERROR, imffs_save(fs, "test_file_for_save_0_bytes.txt", "fileO.txt"));
    

    printf("\nFile E\n");
    VERIFY_INT(IMFFS_ERROR, imffs_save(fs, "test_file_for_save_0_bytes.txt", "fileI.txt"));

    printf("\nPrinting\n");
    imffs_dir(fs);
    imffs_fulldir(fs);

    printf("\nremoving some\n");
    VERIFY_INT(IMFFS_OK, imffs_delete(fs, "fileC.txt"));
    VERIFY_INT(IMFFS_ERROR, imffs_delete(fs, "aa.txt"));
    VERIFY_INT(IMFFS_OK, imffs_delete(fs, "fileA.txt"));
    VERIFY_STR("11001", checking_free_array_creation(fs));

    printf("\nPrinting\n");
    imffs_dir(fs);
    imffs_fulldir(fs);

    printf("\nEnter something of size < 768\n");
    printf("\nFile D\n");
    VERIFY_INT(IMFFS_OK, imffs_save(fs, "file_less_than_768.txt", "fileD.txt"));
    //Verify the number of keys that the mm has
    VERIFY_INT(2, mm_count_keys(checking_multimap_creation(fs)));
    //Verify the total number of bytes read in
    VERIFY_INT(671, get_total_bytes_read_in(fs, "fileD.txt"));
    //Verify the total number of chunks (value nodes)
    VERIFY_INT(2, get_all_value_nodes(fs, "fileD.txt"));
    //Verify the total number of blocks from all the chunks
    VERIFY_INT(3, get_total_blocks_used(fs, "fileD.txt"));
    //check the free_array if things are marked correctly
    VERIFY_STR("00000", checking_free_array_creation(fs));
    VERIFY_INT(IMFFS_OK, looking_for_key(fs, "fileD.txt"));
    
    printf("\nPrinting\n");
    imffs_dir(fs);
    imffs_fulldir(fs);

    printf("\nLoad\n");
    VERIFY_INT(IMFFS_INVALID, imffs_load(fs, "aaa.txt", "angel.txt"));
    VERIFY_INT(IMFFS_OK, imffs_load(fs, "fileD.txt", "angel.txt"));


    printf("\nTESTING OVERWRITE\n");
    printf("\nFile GABRIEL\n");
    VERIFY_INT(IMFFS_OK, imffs_save(fs, "another_less_than_768.txt", "fileD.txt"));
    //Verify the number of keys that the mm has
    VERIFY_INT(2, mm_count_keys(checking_multimap_creation(fs)));
    //Verify the total number of bytes read in
    VERIFY_INT(732, get_total_bytes_read_in(fs, "fileD.txt"));
    //Verify the total number of chunks (value nodes)
    VERIFY_INT(2, get_all_value_nodes(fs, "fileD.txt"));
    //Verify the total number of blocks from all the chunks
    VERIFY_INT(3, get_total_blocks_used(fs, "fileD.txt"));
    //check the free_array if things are marked correctly
    VERIFY_STR("00000", checking_free_array_creation(fs));
    VERIFY_INT(IMFFS_OK, looking_for_key(fs, "fileD.txt"));

    VERIFY_INT(IMFFS_OK, imffs_rename(fs, "fileD.txt", "GABRIEL.txt"));

    VERIFY_INT(IMFFS_OK, imffs_load(fs, "GABRIEL.txt", NULL));


    VERIFY_INT(IMFFS_OK, imffs_destroy(fs));

    
}

void testing_save_3_blocks(){
    printf("\nSaving 3 blocks:\n\n");

    int block_count =3;

    IMFFSPtr fs;
    VERIFY_INT(IMFFS_OK, imffs_create(block_count, &fs));

    //Checking the creation of the entire imffs
    VERIFY_NOT_NULL(fs);

    //Checking the creation of the mm
    VERIFY_NOT_NULL(checking_multimap_creation(fs));

    //Checking the creation of the device
    VERIFY_NOT_NULL(checking_device_creation(fs));

    //Checking the creation of the free_array
    VERIFY_NOT_NULL(checking_free_array_creation(fs));
    //Checking that the free_array is correctly created);
    VERIFY_STR("111", checking_free_array_creation(fs));
    
    //Checking the max_bytes;
    VERIFY_INT(block_count*256, checking_max_bytes(fs));


    //Saving files TO THE FRONT, and nothing obstructing
//FILE A
    printf("\nFile A\n");
    VERIFY_INT(IMFFS_OK, imffs_save(fs, "test_file_for_save_0_bytes.txt", "fileA.txt"));
    //Verify the number of keys that the mm has
    VERIFY_INT(1, mm_count_keys(checking_multimap_creation(fs)));
    //Verify the total number of bytes read in
    VERIFY_INT(0, get_total_bytes_read_in(fs, "fileA.txt"));
    //Verify the total number of chunks (value nodes)
    VERIFY_INT(1, get_all_value_nodes(fs, "fileA.txt"));
    //Verify the total number of blocks from all the chunks
    VERIFY_INT(1, get_total_blocks_used(fs, "fileA.txt"));
    //check the free_array if things are marked correctly
    VERIFY_STR("011", checking_free_array_creation(fs));
    VERIFY_INT(IMFFS_OK, looking_for_key(fs, "fileA.txt"));


//FILE B

    printf("\nFile B\n");
    VERIFY_INT(IMFFS_OK, imffs_save(fs, "test_file_for_save_256_bytes.txt", "fileB.txt"));
    //Verify the number of keys that the mm has
    VERIFY_INT(2, mm_count_keys(checking_multimap_creation(fs)));
    //Verify the total number of bytes read in
    VERIFY_INT(257, get_total_bytes_read_in(fs, "fileB.txt"));
    //Verify the total number of chunks (value nodes)
    VERIFY_INT(1, get_all_value_nodes(fs, "fileB.txt"));
    //Verify the total number of blocks from all the chunks
    VERIFY_INT(2, get_total_blocks_used(fs, "fileB.txt"));
    //check the free_array if things are marked correctly
    VERIFY_STR("000", checking_free_array_creation(fs));

    printf("\n");
    imffs_fulldir(fs);
    imffs_dir(fs);


    printf("Checking A\n");
    //Verify the number of keys that the mm has
    VERIFY_INT(2, mm_count_keys(checking_multimap_creation(fs)));
    //Verify the total number of bytes read in
    VERIFY_INT(0, get_total_bytes_read_in(fs, "fileA.txt"));
    //Verify the total number of chunks (value nodes)
    VERIFY_INT(1, get_all_value_nodes(fs, "fileA.txt"));
    //Verify the total number of blocks from all the chunks
    VERIFY_INT(1, get_total_blocks_used(fs, "fileA.txt"));

    printf("\nDelete file B\n");
    //Look for the key in the multimap
    VERIFY_INT(IMFFS_OK, looking_for_key(fs, "fileB.txt"));
    //Delete
    VERIFY_INT(IMFFS_OK, imffs_delete(fs, "fileB.txt"));
    VERIFY_INT(IMFFS_ERROR, imffs_delete(fs, "fileB.txt"));
    //Checking the tree
    VERIFY_STR("011", checking_free_array_creation(fs));
    //Checking the mulitmap
    VERIFY_INT(1, mm_count_keys(checking_multimap_creation(fs)));
    //Look for the key in the multimap
    VERIFY_INT(IMFFS_ERROR, looking_for_key(fs, "fileB.txt"));
    printf("\n");

    //DELETEING files
    //Look for the key in the multimap
    printf("\nDelete file A\n");
    VERIFY_INT(IMFFS_OK, looking_for_key(fs, "fileA.txt"));
    //Delete
    VERIFY_INT(IMFFS_OK, imffs_delete(fs, "fileA.txt"));
    VERIFY_INT(IMFFS_ERROR, imffs_delete(fs, "fileA.txt"));
    //Checking the tree
    VERIFY_STR("111", checking_free_array_creation(fs));
    //Checking the mulitmap
    VERIFY_INT(0, mm_count_keys(checking_multimap_creation(fs)));
    //Look for the key in the multimap
    VERIFY_INT(IMFFS_ERROR, looking_for_key(fs, "fileA.txt"));



    printf("Try to print\n");
    VERIFY_INT(IMFFS_OK, imffs_dir(fs));
    VERIFY_INT(IMFFS_OK, imffs_fulldir(fs));

    VERIFY_INT(IMFFS_OK, imffs_destroy(fs));

    VERIFY_INT(IMFFS_INVALID, imffs_dir(fs));
    VERIFY_INT(IMFFS_INVALID, imffs_fulldir(fs));



}

void testing_save_adding_to_front(){
    printf("\nWHEN THE FILE SIZE CAN GO UP TO 256:\n\n");

    int block_count =4;

    IMFFSPtr fs;
    VERIFY_INT(IMFFS_OK, imffs_create(block_count, &fs));

    //Checking the creation of the entire imffs
    VERIFY_NOT_NULL(fs);

    //Checking the creation of the mm
    VERIFY_NOT_NULL(checking_multimap_creation(fs));

    //Checking the creation of the device
    VERIFY_NOT_NULL(checking_device_creation(fs));

    //Checking the creation of the free_array
    VERIFY_NOT_NULL(checking_free_array_creation(fs));
    //Checking that the free_array is correctly created);
    VERIFY_STR("1111", checking_free_array_creation(fs));
    
    //Checking the max_bytes;
    VERIFY_INT(block_count*256, checking_max_bytes(fs));


    //Saving files TO THE FRONT, and nothing obstructing
//FILE A
    printf("\nFile A\n");
    VERIFY_INT(IMFFS_OK, imffs_save(fs, "test_file_for_save_256_bytes.txt", "fileA.txt"));
    //Verify the number of keys that the mm has
    VERIFY_INT(1, mm_count_keys(checking_multimap_creation(fs)));
    //Verify the total number of bytes read in
    VERIFY_INT(257, get_total_bytes_read_in(fs, "fileA.txt"));
    //Verify the total number of chunks (value nodes)
    VERIFY_INT(1, get_all_value_nodes(fs, "fileA.txt"));
    //Verify the total number of blocks from all the chunks
    VERIFY_INT(2, get_total_blocks_used(fs, "fileA.txt"));
    //check the free_array if things are marked correctly
    VERIFY_STR("0011", checking_free_array_creation(fs));
    //Delete fileA
    //DELETEING files
    //Look for the key in the multimap
    VERIFY_INT(IMFFS_OK, looking_for_key(fs, "fileA.txt"));
    //Delete
    VERIFY_INT(IMFFS_OK, imffs_delete(fs, "fileA.txt"));
    VERIFY_INT(IMFFS_ERROR, imffs_delete(fs, "fileA.txt"));
    //Checking the tree
    VERIFY_STR("1111", checking_free_array_creation(fs));
    //Checking the mulitmap
    VERIFY_INT(0, mm_count_keys(checking_multimap_creation(fs)));
    //Look for the key in the multimap
    VERIFY_INT(IMFFS_ERROR, looking_for_key(fs, "fileA.txt"));
    printf("\n");


    VERIFY_INT(IMFFS_OK, imffs_destroy(fs));
}

//Edge case
void testing_device_size_0(){
    printf("\nWHEN THE DEVICE HAS SIZE OF 0:\n\n");
    int block_count =0;

    IMFFSPtr fs;
    VERIFY_INT(IMFFS_FATAL, imffs_create(block_count, &fs));

    VERIFY_INT(IMFFS_FATAL, imffs_save(fs, "test_file_for_save_0_bytes.txt", "bob.txt"));

    VERIFY_INT(IMFFS_INVALID, imffs_destroy(fs));
}



/**
 * 
 * Checking delete()
 * Check the number of keys that the mm has
 * Check the boxes
 * Try looking for that key
*/
void testing_delete(){
    printf("\nTESTING DELETE FUNCTION: \n\n");

    int block_count =2;

    IMFFSPtr fs;

    VERIFY_INT(IMFFS_OK, imffs_create(block_count, &fs));

    //Checking the creation of the entire imffs
    VERIFY_NOT_NULL(fs);

    //Checking the creation of the mm
    VERIFY_NOT_NULL(checking_multimap_creation(fs));

    //Checking the creation of the device
    VERIFY_NOT_NULL(checking_device_creation(fs));

    //Checking the creation of the free_array
    VERIFY_NOT_NULL(checking_free_array_creation(fs));
    //Checking that the free_array is correctly created);
    VERIFY_STR("11", checking_free_array_creation(fs));
    
    //Checking the max_bytes;
    VERIFY_INT(2*256, checking_max_bytes(fs));

    //Saving 2 boxes
    VERIFY_INT(IMFFS_ERROR, imffs_save(fs, "test_file_for_save_512_bytes.txt", "2Boxes.txt"));

    //Checking if the delete works
    //Checking if the key did not get inserted
    VERIFY_INT(0, mm_count_keys(checking_multimap_creation(fs)));

    //Checking if the free_array is correct
    VERIFY_STR("11", checking_free_array_creation(fs));

}

void testing_saving_out_of_bounds(){
    printf("\nWHEN THE FILE SIZE CAN GO out of bounds:\n\n");

    int block_count =3;

    IMFFSPtr fs;
    VERIFY_INT(IMFFS_OK, imffs_create(block_count, &fs));

    //Checking the creation of the entire imffs
    VERIFY_NOT_NULL(fs);

    //Checking the creation of the mm
    VERIFY_NOT_NULL(checking_multimap_creation(fs));

    //Checking the creation of the device
    VERIFY_NOT_NULL(checking_device_creation(fs));

    //Checking the creation of the free_array
    VERIFY_NOT_NULL(checking_free_array_creation(fs));
    //Checking that the free_array is correctly created);
    VERIFY_STR("111", checking_free_array_creation(fs));
    
    //Checking the max_bytes;
    VERIFY_INT(block_count*256, checking_max_bytes(fs));

    printf("\nAdding file of size 256\n");
    printf("\nFile D\n");
    VERIFY_INT(IMFFS_OK, imffs_save(fs, "test_file_for_save_256_bytes.txt", "fileD.txt"));
    //Verify the number of keys that the mm has
    VERIFY_INT(1, mm_count_keys(checking_multimap_creation(fs)));
    //Verify the total number of bytes read in
    VERIFY_INT(257, get_total_bytes_read_in(fs, "fileD.txt"));
    //Verify the total number of chunks (value nodes)
    VERIFY_INT(1, get_all_value_nodes(fs, "fileD.txt"));
    //Verify the total number of blocks from all the chunks
    VERIFY_INT(2, get_total_blocks_used(fs, "fileD.txt"));
    //check the free_array if things are marked correctly
    VERIFY_STR("001", checking_free_array_creation(fs));

    printf("\nAdding file of size 1024\n");

    // //FILE F
    printf("\nFile F\n");
    VERIFY_INT(IMFFS_ERROR, imffs_save(fs, "test_file_for_save_1024_bytes.txt", "fileF.txt"));
    //Verify the number of keys that the mm has
    VERIFY_INT(1, mm_count_keys(checking_multimap_creation(fs)));
    //Verify the total number of bytes read in
    VERIFY_INT(IMFFS_ERROR, get_total_bytes_read_in(fs, "fileF.txt"));
    //Verify the total number of chunks (value nodes)
    VERIFY_INT(0, get_all_value_nodes(fs, "fileF.txt"));
    //Verify the total number of blocks from all the chunks
    VERIFY_INT(0, get_total_blocks_used(fs, "fileF.txt"));
    //check the free_array if things are marked correctly
    VERIFY_STR("001", checking_free_array_creation(fs));
    printf("\n");

}

void testing_changing_name(){
    /**
     * comparing 2 names, that's it
     * Making sure all of the data is intact
    */
}

void testing_loading_basic(){
    printf("\nTESTING LOADING BASIC:\n\n");

    int block_count =6;

    IMFFSPtr fs;
    VERIFY_INT(IMFFS_OK, imffs_create(block_count, &fs));

    //Checking the creation of the entire imffs
    VERIFY_NOT_NULL(fs);

    //Checking the creation of the mm
    VERIFY_NOT_NULL(checking_multimap_creation(fs));

    //Checking the creation of the device
    VERIFY_NOT_NULL(checking_device_creation(fs));

    //Checking the creation of the free_array
    VERIFY_NOT_NULL(checking_free_array_creation(fs));
    //Checking that the free_array is correctly created);
    VERIFY_STR("111111", checking_free_array_creation(fs));
    
    //Checking the max_bytes;
    VERIFY_INT(block_count*256, checking_max_bytes(fs));

    //Saving files TO THE FRONT, and nothing obstructing
//FILE A
    printf("\nFile A\n");
    VERIFY_INT(IMFFS_OK, imffs_save(fs, "file_less_than_768.txt", "fileA.txt"));
    //Verify the number of keys that the mm has
    VERIFY_INT(1, mm_count_keys(checking_multimap_creation(fs)));
    //Verify the total number of bytes read in
    VERIFY_INT(671, get_total_bytes_read_in(fs, "fileA.txt"));
    //Verify the total number of chunks (value nodes)
    VERIFY_INT(1, get_all_value_nodes(fs, "fileA.txt"));
    //Verify the total number of blocks from all the chunks
    VERIFY_INT(3, get_total_blocks_used(fs, "fileA.txt"));
    //check the free_array if things are marked correctly
    VERIFY_STR("000111", checking_free_array_creation(fs));
    VERIFY_INT(IMFFS_OK, looking_for_key(fs, "fileA.txt"));

//FILE B
    printf("\nFile B\n");
    VERIFY_INT(IMFFS_OK, imffs_save(fs, "another_less_than_768.txt", "fileB.txt"));
    //Verify the number of keys that the mm has
    VERIFY_INT(2, mm_count_keys(checking_multimap_creation(fs)));
    //Verify the total number of bytes read in
    VERIFY_INT(732, get_total_bytes_read_in(fs, "fileB.txt"));
    //Verify the total number of chunks (value nodes)
    VERIFY_INT(1, get_all_value_nodes(fs, "fileB.txt"));
    //Verify the total number of blocks from all the chunks
    VERIFY_INT(3, get_total_blocks_used(fs, "fileB.txt"));
    //check the free_array if things are marked correctly
    VERIFY_STR("000000", checking_free_array_creation(fs));
    VERIFY_INT(IMFFS_OK, looking_for_key(fs, "fileB.txt"));
    

    VERIFY_INT(IMFFS_OK, imffs_load(fs, "fileA.txt", "angel.txt"));
    VERIFY_INT(IMFFS_OK, imffs_delete(fs, "fileA.txt"));

    VERIFY_INT(IMFFS_OK, imffs_load(fs, "fileB.txt", "GABRIEL.txt"));
    VERIFY_INT(IMFFS_INVALID, imffs_load(fs, "fileA.txt", "angel.txt"));

    VERIFY_INT(IMFFS_OK, imffs_destroy(fs));

}

int main(){
    testing_creating_imffs();
    testing_creating_imffs_error_case();
    testing_device_size_0();
    testing_delete();
    testing_save_adding_to_front();
    testing_saving_out_of_bounds();


    testing_save_256_1_block();
    testing_save_0_1_block();
    testing_save_3_blocks();


    testing_complete();

    testing_loading_basic();

    if (0 == Tests_Failed) {
        printf("\nAll %d tests passed.\n", Tests_Passed);
    } else {
        printf("\nFAILED %d of %d tests.\n", Tests_Failed, Tests_Failed+Tests_Passed);
    }
    
    printf("\n*** Tests complete.\n");  
    
    return 0;
}