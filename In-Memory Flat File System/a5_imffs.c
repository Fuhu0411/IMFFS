#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "a5_imffs.h"


typedef enum{
  TRUE =1,
  FALSE =0
}Boolean;

typedef struct FILE_INFORMATION{
  char file_name[1024];
  int file_byte_size;
}FileInformation;

typedef struct IMFFS{
    /*
        Index: multimap*
        array to keep track of the free blocks: char*
        the device itself: Uint8* 
    */
   Multimap *mm;
   char *free_array;//1 means available, 0 means occupied
   
   uint8_t *device;

   int max_bytes; //This needs to be integer
   int max_bytes_read;
}Imffs;

//This is to compare the keys alphabetically, case-insensitively
static int void_strcasecmp(void *a, void *b) {
  assert(NULL != a && NULL != b);
  return strcasecmp(a, b);
}

//always adding to the end
static int compare_always_larger(void *a, void *b) {
  return 1;
}

#ifndef NDEBUG
static int validate_IMFFS(IMFFSPtr fs)
{
  //If the mm has been created successfully
  assert(NULL != fs);
  
  //If the device has been created successfully
  assert(NULL != fs->device);
  
  //If the number of max_bytes is valid
  assert(fs->max_bytes >= 0);
  
  //If the free_array is valid
  assert(fs->free_array != NULL);
  
  //If the mm is valid
  assert(NULL!= fs->mm);

  
  
  return 1; // always return TRUE
}
#endif

//For defining the free_array with all "1";
static void defining_free_array(IMFFSPtr fs, uint32_t block_count){
    assert(validate_IMFFS(fs));

    if(fs!= NULL && fs->device != NULL && fs->mm != NULL && fs->free_array != NULL){
        //Since strings in c in mutable 
        for(int i =0; i<block_count; i++){
            fs->free_array[i] = '1';
        }
    }
}

IMFFSResult imffs_create(uint32_t block_count, IMFFSPtr *fs){
    //block_count cannot be <=0, must be at least 1
    assert(block_count>0);

    *fs = NULL;


    if(block_count>0){
        //Have to create the place to store the imffs
        (*fs) = malloc(sizeof(Imffs));
        if(*fs != NULL){
            //for testing purposes
            (*fs)->mm = NULL;
            //for testing purposes
            (*fs)->device = NULL;
            //for testing purposes
            (*fs)->free_array = NULL;
            //for testing purposes
            (*fs)->max_bytes =0;

            //Create mm
            (*fs)->mm = mm_create(block_count, void_strcasecmp, compare_always_larger);
            //If cannot create mm
            if(NULL == (*fs)->mm){
                //Free the *fs
                free(*fs);
                (*fs) = NULL;
                //Return fatal
                return IMFFS_FATAL;
            }
            //If created mm
            else{
                //Create device
                (*fs)->device = malloc(256 * block_count);            
                //If cannot create device,
                if((*fs)->device == NULL){
                    //Free mm
                    free((*fs)->mm);
                    //Free *fs
                    free(*fs);

                    //for testing purposes
                    (*fs)->mm = NULL;
                    
                    //for testing purposes
                    *fs = NULL;

                    //Return fatal
                    return IMFFS_FATAL;
                }
                //If created device
                else{
                    //Create free_array
                    (*fs)->free_array = malloc(block_count+1);
                    //If cannot create free_array
                    if((*fs)->free_array == NULL){
                        //Free device
                        free((*fs)->device);
                        
                        //for testing purposes
                        (*fs)->device = NULL;

                        //Free mm
                        free((*fs)->mm);

                        //for testing purposes
                        (*fs)->mm = NULL;


                        //Free *fs
                        free(*fs);
                        *fs = NULL;
                        


                        //Return fatal
                        return IMFFS_FATAL;
                    }
                    //If created free_array
                    else{
                        defining_free_array(*fs, block_count);
                        //Making sure the end is \0
                        (*fs)->free_array[block_count] = '\0';

                        (*fs)->max_bytes = block_count * 256;
                        (*fs)->max_bytes_read = 0;
                        return IMFFS_OK;
                    }
                }
            }
        }
    }


    return IMFFS_FATAL;

}


//For testing purposes
Multimap *checking_multimap_creation(IMFFSPtr fs){
    assert(validate_IMFFS(fs));

    if(fs!= NULL && fs->device != NULL && fs->mm != NULL && fs->free_array != NULL){
        return fs->mm;
    }
    else{
        return NULL;
    }
}
uint8_t *checking_device_creation(IMFFSPtr fs){
    assert(validate_IMFFS(fs));

    if(fs!= NULL && fs->device != NULL && fs->mm != NULL && fs->free_array != NULL){
        return fs->device;
    }
    else{
        return NULL;
    }
}
char *checking_free_array_creation(IMFFSPtr fs){
    assert(validate_IMFFS(fs));

    if(fs!= NULL && fs->device != NULL && fs->mm != NULL && fs->free_array != NULL){
        return fs->free_array;
    }
    else{
        return NULL;
    }
}
int checking_max_bytes(IMFFSPtr fs){
    assert(validate_IMFFS(fs));

    if(fs!= NULL && fs->device != NULL && fs->mm != NULL && fs->free_array != NULL){
        return fs->max_bytes;
    }
    else{
        return IMFFS_INVALID;
    }
}

//to look for free box
static int find_the_free_box(IMFFSPtr fs){
    int count =0;
    while(fs->free_array[count] != '\0' && fs->free_array[count] != '1'){
        count++;
    }

    //Case when there are no empty boxes left
    if(fs->free_array[count] == '\0'){
        return -1;
    }
    else{
        return count;
    }
}

//To check the next box
static int go_to_next_box(IMFFSPtr fs, int index){
    //If not out of boxes and that the next box is available,
    if(fs->free_array[index+1] != '\0' && fs->free_array[index+1] == '1'){
        return index+1;
    }
    else{
        return -1;
    }
}

int get_all_value_nodes(IMFFSPtr fs, char *imffsfile){
    return mm_count_values(fs->mm, imffsfile);
}

int get_total_blocks_used(IMFFSPtr fs, char *imffsfile){
    int total_num_of_value_nodes = get_all_value_nodes(fs, imffsfile);

    Value values[total_num_of_value_nodes];
    mm_get_values(fs->mm, imffsfile, values, total_num_of_value_nodes);

    int count =0;
    int total = 0;
    while(count < total_num_of_value_nodes){
        total += values[count].num;    
        count++;  
    }

    return total;
}

static int saving_process(FileInformation *file, char *imffsfile, IMFFSPtr fs, FILE* in){
    int result =0;

    int num_blocks_each_chunk = 0;
    int total_bytes_read_in = 0;

    //Let's store the file info
    file->file_byte_size = 0; //for now
    strcpy(file->file_name, imffsfile); //copy file

    //I think this means I am pointing at the beginning of the device
    uint8_t* ptr_to_save = fs->device;
    uint8_t* points_beginning_of_chunk = fs->device;
    
    //Find if there are any boxes available
    int index_of_free_box = 0;
    int temp_bytes_read = 0;
    
    int count_iterations = 0;

    //Boolean to check if we should update the beginning pointer
    Boolean update = TRUE;


    //Find the free box to store
    index_of_free_box = find_the_free_box(fs);
    //Case when there is no box left to save
    if(index_of_free_box == -1){
        printf("CANNOT SAVE FILE BECAUSE NOT ENOUGH SPACE\n");
        result = IMFFS_ERROR;
    }
    else{
        do{
            if(update){
                //Resetting the points_beginning_of_chunk to make sure the calculation is correct
                points_beginning_of_chunk = fs->device;
                points_beginning_of_chunk = points_beginning_of_chunk+ 256 *index_of_free_box;
                update = FALSE;
            }
            //This one will move continuously as long as there are more blocks in a chunk
            ptr_to_save = ptr_to_save + 256 * index_of_free_box;
            
            temp_bytes_read = fread(ptr_to_save, 1, 256, in);
            
            //As long as the bytes is >0, or first iteration, 
            total_bytes_read_in += temp_bytes_read;
            fs->free_array[index_of_free_box] = '0'; //marking block as read
            num_blocks_each_chunk++;

            //look at block next to this block
            index_of_free_box = go_to_next_box(fs, index_of_free_box);

            //if the next one is already occupied, look for a different block
            if(index_of_free_box == -1){
                //Because the next one is blocked, must turn this into a chunk
                mm_insert_value(fs->mm, file, num_blocks_each_chunk, points_beginning_of_chunk);

                //Need to find another block to keep saving
                index_of_free_box = find_the_free_box(fs);

                num_blocks_each_chunk =0;
                update = TRUE;
            }

            //Preparing for the next reading
            ptr_to_save = fs->device;
            

            count_iterations++;

        }while(temp_bytes_read == 256 && index_of_free_box != -1);
    
        //Either temp_bytes_read < 256
        //index_of_free_box == -1
        //fs->max_bytes_read >= fs->max_bytes
        //index_of_free_box== -2

        if(index_of_free_box == -1 && temp_bytes_read == 256){
            printf("NO MORE SPACE\n");
            imffs_delete(fs, imffsfile);
            result = IMFFS_ERROR;
        }
        else if(index_of_free_box == -1 && temp_bytes_read <256 && temp_bytes_read >=0){
            result = IMFFS_OK;
            file->file_byte_size = total_bytes_read_in;


            //Just updated
            fs->max_bytes_read += file->file_byte_size;
            if(total_bytes_read_in == 0){
                fs->max_bytes_read += 256;
            }

        }
        else if(index_of_free_box != -1 && temp_bytes_read <256 && temp_bytes_read >=0){
            result = IMFFS_OK;
            mm_insert_value(fs->mm, file, num_blocks_each_chunk, points_beginning_of_chunk);
            file->file_byte_size = total_bytes_read_in;
            
            //Just updated
            fs->max_bytes_read += file->file_byte_size;
            if(total_bytes_read_in == 0){
                fs->max_bytes_read += 256;
            }
        }

    }


    return result;
}


//For testing purposes
int get_total_bytes_read_in(IMFFSPtr fs, char *imffsfile){
    assert(validate_IMFFS(fs));
    assert(imffsfile != NULL);

    if(fs!= NULL && fs->device != NULL && fs->mm != NULL && fs->free_array != NULL && imffsfile != NULL){
        // printf("Trying to print\n");
        // mm_print(fs->mm);
        void *ptr;
      


        //Have to find the key
        if (mm_get_first_key(fs->mm, &ptr) > 0) {
            //This loop is empty, because I am just looking for the key with the right name
            while (strcmp(((FileInformation*)ptr)->file_name, imffsfile) != 0 && mm_get_next_key(fs->mm, &ptr) > 0){}
        }

        if(strcmp(((FileInformation*)ptr)->file_name, imffsfile) == 0){
            return ((FileInformation*)ptr)->file_byte_size;
        }
        else{
            return IMFFS_ERROR;
        }
    }
    else if(fs == NULL){
        return IMFFS_FATAL;

    }

    return IMFFS_ERROR;
}

IMFFSResult imffs_save(IMFFSPtr fs, char *diskfile, char *imffsfile){
    assert(validate_IMFFS(fs));
    assert(diskfile != NULL);
    assert(imffsfile != NULL);

    int result = IMFFS_OK;
    if(fs!= NULL && fs->device != NULL && fs->mm != NULL && fs->free_array != NULL && diskfile!= NULL && imffsfile != NULL){
        //Need to make sure that the file hasn't already existed
        if(IMFFS_OK == looking_for_key(fs, imffsfile)){
            printf("THE FILE ALREADY EXISTS. OVERWRITING\n");
            //delete the previous file
            imffs_delete(fs, imffsfile);
        }
        //Need to open the file for reading
        FILE* in;
        in = fopen(diskfile, "r");


        if(in != NULL){
            //Create a place to store the filename
            //Saving diskfile as imffsfile in the device
            FileInformation *file = malloc(sizeof(FileInformation));
            //If cannot create the file
            if(file == NULL){
                return IMFFS_ERROR;
            }
            else{
                result = saving_process(file, imffsfile, fs, in);
            }
        }
        else{
            perror("Cannot open the file!");
            result = IMFFS_ERROR;
        }
    }
    else if(fs == NULL){
        result = IMFFS_FATAL;
    }
    else{
        result = IMFFS_ERROR;
    }


    return result;
}

static int getting_the_total_bytes_to_write(IMFFSPtr fs, char *imffsfile){
    int total_bytes =-1;
    void* ptr;


    if(mm_get_first_key(fs->mm, &ptr)>0){
        while(strcmp(((FileInformation*)ptr)->file_name, imffsfile) !=0  && mm_get_next_key(fs->mm, &ptr) > 0){}
    }

    if(strcmp(((FileInformation*)ptr)->file_name, imffsfile) == 0){
        total_bytes = ((FileInformation*)ptr)->file_byte_size;
    }

    return total_bytes;
}

IMFFSResult looking_for_key(IMFFSPtr fs, char *imffsfile){
    assert(validate_IMFFS(fs));
    assert(imffsfile != NULL);

    if(fs!= NULL && fs->device != NULL && fs->mm != NULL && fs->free_array != NULL && imffsfile != NULL){
        void* ptr;
        int count = 0;
        if(mm_get_first_key(fs->mm, &ptr)>0){
            while(strcmp(((FileInformation*)ptr)->file_name, imffsfile) != 0 && mm_get_next_key(fs->mm, &ptr)>0){
                count++;
            }
        }

        if(count < mm_count_keys(fs->mm) && strcmp(((FileInformation*)ptr)->file_name, imffsfile) == 0){
            return IMFFS_OK;
        }
        
        //Cannot find
        else{
            return IMFFS_ERROR;
        }
  
    }
    
    return IMFFS_INVALID;
}

IMFFSResult imffs_load(IMFFSPtr fs, char *imffsfile, char *diskfile){
    assert(validate_IMFFS(fs));
    assert(imffsfile != NULL);
    assert(diskfile != NULL);

    if(fs!= NULL && fs->device != NULL && fs->mm != NULL && fs->free_array != NULL && imffsfile != NULL){
        //Making sure the key exists
        if(IMFFS_OK == looking_for_key(fs, imffsfile)){
            FILE* out;

            if(diskfile!= NULL){
                out = fopen(diskfile, "w");

            }
            else{
                out = fopen(imffsfile, "w");
            }
            

            if(out == NULL){
                perror("AYO! Unable to open the output file to write");
                return IMFFS_ERROR;
            }else{
                //Get the total bytes to write
                int total_bytes_to_write = getting_the_total_bytes_to_write(fs, imffsfile);

                //Get the values
                int num_values = mm_count_values(fs->mm, imffsfile);
                Value values[num_values];
                mm_get_values(fs->mm, imffsfile, values, num_values);

                //Successfully retrieved the total bytes
                if(total_bytes_to_write != -1){
                    int count =0;

                    while(count < num_values){
                        int through_block =0;

                        //Apparently, fwrite will start writing at the pointer you inserted in the first slot.
                        //Therefore, I  must update that regularly
                        void* temp = values[count].data;
                        while(through_block < values[count].num){
                            int amount_to_write = 256;
                            if(total_bytes_to_write < 256){
                                amount_to_write = total_bytes_to_write;
                            }
                            fwrite(temp, 1, amount_to_write, out);

                            //Maybe I have to update my values[count].data
                            temp += amount_to_write;

                            total_bytes_to_write -= amount_to_write;

                            through_block++;
                        }
                        count++;
                    }


                    assert(total_bytes_to_write == 0);
                    if(total_bytes_to_write != 0){
                        return IMFFS_ERROR;
                    }
                    else{
                        return IMFFS_OK;

                    }

                }


                return IMFFS_ERROR;
            }

        }
    }

    return IMFFS_INVALID;
}


static void updating_free_array(IMFFSPtr fs, char* imffsfile){
    //Go straight there and change the values for the free_array
    int num_values = mm_count_values(fs->mm, imffsfile);
    Value values[num_values];
    mm_get_values(fs->mm, imffsfile, values, num_values);

    int count =0;
    while(count < num_values){
        uint8_t* beginning = fs->device;

        //Find the index of the first box, and then update
        int index_to_update = ((uint8_t*)values[count].data - beginning)/256;

        //Cases when a chunk has multiple blocks
        int last_box_to_update = index_to_update + values[count].num;
        int count_box =index_to_update;

        //Forcing that there is no way we going out bounds
        //Checking that the last box we are checking is NOT out of bounds
        assert(fs->free_array[last_box_to_update-1] != '\0');
        
            
        if(fs->free_array[last_box_to_update-1] != '\0'){
            //Checking that the last box we are checking is NOT out of bounds
            while(count_box < last_box_to_update){
                fs->free_array[count_box] = '1';
                count_box++;
            }
        }

        
        count++;
    }

}



IMFFSResult imffs_delete(IMFFSPtr fs, char *imffsfile){
    assert(validate_IMFFS(fs));
    assert(imffsfile != NULL);

    if(fs!= NULL && fs->device != NULL && fs->mm != NULL && fs->free_array != NULL && imffsfile != NULL){

        //Look for the key
        //and free() first
        void* ptr;
        if(mm_get_first_key(fs->mm, &ptr)>0){
            while(strcmp(((FileInformation*)ptr)->file_name, imffsfile) != 0 && mm_get_next_key(fs->mm, &ptr)>0){}
        }

        if(mm_count_keys > 0 && strcmp(((FileInformation*)ptr)->file_name, imffsfile) == 0){
            fs->max_bytes_read -= ((FileInformation*)ptr)->file_byte_size;
            if(((FileInformation*)ptr)->file_byte_size ==0){
                 fs->max_bytes_read -= 256;
            }

            //Have to update the free_array first because the mm knows where to update
            updating_free_array(fs, imffsfile);
            //Remove the key from the mm
            mm_remove_key(fs->mm, imffsfile);
            //free the location that stores the data of the key
            free(ptr);
        }
        //If I cannot find the key
        else{
            //Invalid because we are removing key that does not exist
            return IMFFS_ERROR;
        }


        return IMFFS_OK;
    }
    
    
    return IMFFS_INVALID;

}

IMFFSResult imffs_rename(IMFFSPtr fs, char *imffsold, char *imffsnew){
    assert(validate_IMFFS(fs));
    assert(imffsold != NULL);
    assert(imffsnew != NULL);

    if(fs!= NULL && fs->device != NULL && fs->mm != NULL && fs->free_array != NULL && imffsold != NULL && imffsnew != NULL){
        void* ptr;
        //Look for the key
        if(mm_get_first_key(fs->mm, &ptr)>0){
            while(strcmp(((FileInformation*)ptr)->file_name, imffsold)!=0 && mm_get_next_key(fs->mm, &ptr) >0){};
        }

        if(strcmp(((FileInformation*)ptr)->file_name, imffsold)==0){
            strcpy(((FileInformation*)ptr)->file_name, imffsnew);
            return IMFFS_OK;
        }

        return IMFFS_ERROR;
    }

    return IMFFS_INVALID;

}

IMFFSResult imffs_dir(IMFFSPtr fs){
    assert(validate_IMFFS(fs));

    if(fs!= NULL && fs->device != NULL && fs->mm != NULL && fs->free_array != NULL){
        void* ptr;
        //list all the files, and the bytes
        if(mm_get_first_key(fs->mm, &ptr)>0){
            do{
                printf("File name: %s\n", ((FileInformation*)ptr)->file_name);
                printf("File size: %d\n", ((FileInformation*)ptr)->file_byte_size);
                printf("\n");
            }
            while(mm_get_next_key(fs->mm, &ptr)>0);

            printf("The total bytes device has: %d\n", fs->max_bytes);
            printf("Total bytes used: %d\n", fs->max_bytes_read);
            printf("BYTES LEFT: %d\n\n", (fs->max_bytes - fs->max_bytes_read));
        }
        else if(mm_count_keys(fs->mm) == 0){
            printf("*Insert cricket noise*\n");
            printf("The total bytes device has: %d\n", fs->max_bytes);
            printf("Total bytes used: %d\n", fs->max_bytes_read);
            printf("BYTES LEFT: %d\n\n", fs->max_bytes);
        }
        else{
            return IMFFS_ERROR;
        }

        return IMFFS_OK;
    }

    return IMFFS_INVALID;

}

IMFFSResult imffs_fulldir(IMFFSPtr fs){
    assert(validate_IMFFS(fs));

    if(fs!= NULL && fs->device != NULL && fs->mm != NULL && fs->free_array != NULL){
        void* ptr;
        //List all the files, the bytes, and the chunks, blocks
        if(mm_get_first_key(fs->mm, &ptr)>0){
            do{
                int num_values = mm_count_values(fs->mm, ((FileInformation*)ptr)->file_name);
                Value values[num_values];

                mm_get_values(fs->mm, ((FileInformation*)ptr)->file_name, values, num_values);

                printf("File name: %9s", ((FileInformation*)ptr)->file_name);
                printf("\tFile size: %9d\n", ((FileInformation*)ptr)->file_byte_size);

                int count=0;
                while(count < num_values){
                    printf("\tChunk number: %d\t", count);
                    
                    printf("Num of blocks: %5d\n\n", values[count].num);
                    
                    count++;
                }

            }
            while(mm_get_next_key(fs->mm, &ptr)>0);
            printf("The total bytes device has: %d\n", fs->max_bytes);
            printf("Total bytes used: %d\n", fs->max_bytes_read);
            printf("BYTES LEFT: %d\n\n", (fs->max_bytes - fs->max_bytes_read));
        }
        else if(mm_count_keys(fs->mm) == 0){
            printf("*Insert cricket noise*\n");
            printf("The total bytes device has: %d\n", fs->max_bytes);
            printf("Total bytes used: %d\n", fs->max_bytes_read);
            printf("BYTES LEFT: %d\n\n", fs->max_bytes);
        }
        else{                                   
            return IMFFS_ERROR;
        }


        return IMFFS_OK;
    }

    return IMFFS_INVALID;
}

IMFFSResult imffs_defrag(IMFFSPtr fs){
    return IMFFS_NOT_IMPLEMENTED;
}


static void destroy_mm(IMFFSPtr fs){
    assert(validate_IMFFS(fs));
    
   if(fs!= NULL && fs->device != NULL && fs->mm != NULL && fs->free_array != NULL){
        void* ptr;
        //Finding the keys and free
        if(mm_get_first_key(fs->mm, &ptr) >0){
            do{
                //Free the location where each key is pointing at.
                free(ptr);
            }
            while(mm_get_next_key(fs->mm, &ptr)>0);
        }
    }

    //Free mm
    mm_destroy(fs->mm);
    fs->mm= NULL;
}

IMFFSResult imffs_destroy(IMFFSPtr fs){
    assert(validate_IMFFS(fs));

    if(fs != NULL){
        //Free mm
        destroy_mm(fs);

        //Free free_array
        free(fs->free_array);
        fs->free_array = NULL;

        //Free device
        free(fs->device);
        fs->device = NULL;

        //Free IMFFS
        free(fs);
        fs = NULL;
        return IMFFS_OK;
    }

    return IMFFS_INVALID;
}
