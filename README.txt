Dec 11th, 2023
Huu Nguyen Phu Le


Programs made by Makefile:
    1. a5_test_mm: 
        a. To test the multimap. 
        b. "The multimap is based on the Assignment 4 solution but needs to be modified to allow it to be used as an index for IMFFS."
    
    2. a5_test_imffs:
        a. To test imffs (a flat file system)
        b. "A file system is responsible for storing and retrieving data in a computer system by organizing it into named files."
    
    3. a5_imffs:
        a. "A file system is responsible for storing and retrieving data in a computer system by organizing it into named files."
        b. The file system program:
            "...The data for the file system is a large region of memory that will be allocated using a single call to malloc. 
            Even though this is not a "real" storage system, we will call this region the device. 
            The device is then conceptually subdivided into equally-sized pieces known as blocks. 
            Files are stored in one or more contiguous (adjacent) sequences of blocks, which we will call chunks. 
            A single file's data can consist of multiple chunks, and their sequence in the file does not need to 
            match their sequence in the memory that makes up the devie..."

            "...The blocks have a fixed size of 256 bytes, all of which is used for data. 
            A file's data will occupy some whole number of blocks, with the last block not fully consumed. 
            For example, a 1000-byte file will require 4 blocks: three full blocks, and one with only 232 bytes used. 
            The remaining 24 byes of the last block are unused (and is known as "slack")..."

            "...File data is stored in blocks on the device starting from the beginning of its memory; 
            the first file at the start, the second file after, and so on. At first, each file can be stored in a single chunk. 
            Things start getting tricky when we delete a file and want to re-use the space it previously occupied. 
            If we delete the first file we saved, then the next file can use the blocks where the first file had been stored. 
            But if the next file is bigger than the deleted file, then we can only fit part of it in that space, and the rest later. 
            That means the file's content will be divided into two (or more) chunks. Over time, as the files split into more than 
            one chunk also get deleted, files will be stored in more and smaller chunks. This is known as data fragmentation.
            To determine where the file can fit we will keep a separate array, in parallel with the device, with one entry for each block. 
            It will keep track of whether the block is in use (that is, contains some file's data) or free. 
            When we want to save a file to the device, we start searching the array at the beginning and we look for the first free block. 
            The new file's data is saved starting at that block, and using as many contiguous free blocks as it needs or are available 
            to save the data. If there are not enough contiguous free blocks for the entire file, then that group of blocks 
            becomes a chunk, and we search for the next free block to start the next chunk, and so on until the entire file is copied.
            We use the multimap to index the files. A key contains the file name and other metadata: for our purposes, 
            this is just the exact size of the file in bytes. The values are chunks (not blocks!), in the order of the file data. 
            The numeric part is the number of blocks in the chunk, and the void pointer can either be a pointer to the memory at 
            the start of the block or a pointer to an int that identifies a block by index (make sure the int is appropriately allocated).
            Note that this list needs to be stored in order that values are added, not sorted by number..."

            "...save diskfile imffsfile copy from your system to IMFFS;
            load imffsfile diskfile copy from IMFFS to your system;
            delete imffsfile remove the IMFFS file from the system, allowing the blocks to be used for other files;
            rename imffsold imffsnew rename the IMFFS file from imffsold to imffsnew, keeping all of the data intact;
            dir will list all of the files and the number of bytes they occupy;
            fulldir is like "dir" except it shows a the files and details about all of the chunks they are stored in (where, and how big);
            defrag is described below;
            help lists the commands (you don't need to do anything to implement this); and
            quit will quit the program..."





1. I have added the below functions to a5_imffs.h for testing purposes.
    Multimap *checking_multimap_creation(IMFFSPtr fs);
            To check if I have created the multimap correctly

    uint8_t *checking_device_creation(IMFFSPtr fs);
            To check if I have created the device correctly

    uint8_t *checking_free_array_creation(IMFFSPtr fs);
            To check if I have created the free_array correctly

    int get_total_bytes_read_in(IMFFSPtr fs, char *imffsfile);
            To check if the total bytes read in is correct

    int get_all_value_nodes(IMFFSPtr fs, char *imffsfile);
            To check if the values nodes are correct 
    
    int get_total_blocks_used(IMFFSPtr fs, char *imffsfile);
            To check if the total blocks used is correct 

    IMFFSResult looking_for_key(IMFFSPtr fs)
            To check if the key exists

2. I have added the below functions to a5_multimap.h for tesing purposes
    int get_max(Multimap* mm);
            To check that max amount of keys the mm can hold

3. Test files included (you need my text files to run the tests)

4. NOTE: MY DEFRAG WILL CRASH THE PROGRAM, YOU HAVE BEEN WARNED!