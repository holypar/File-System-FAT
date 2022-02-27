#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "disk.h"
#include "fs.h"

#define BLOCK_SIZE 4096
#define FAT_EOC 0xFFFF
#define MAX_ROOT_FILES 128
#define MAX_FILE_NAME_SIZE 16

//TO DO:
	// 1.) Error Handling
	// 2.) Start Phase 3
	// 3.) Clean up code
// Superblock struct
struct __attribute__((packed)) superblock { 
	uint8_t signature[8]; 
	uint16_t totalVirtualDiskBlocks;
	uint16_t rootBlockIndex;
	uint16_t dataBlockStartIndex;
	uint16_t amountDataBlocks; 
	uint8_t fatBlocks;
	uint8_t padding[4079]; 
};

struct __attribute__((packed)) rootDirEntry {
	uint8_t fileName[MAX_FILE_NAME_SIZE];  //16 bytes
	uint32_t fileSize;	   // 4 bytes
	uint16_t indexOfFirstDataBlock; // 2 bytes
	uint8_t unused[10];     // 10 bytes
	//total 32 bytes
};


struct __attribute__((packed)) fdEntry {
	uint8_t fileName[MAX_FILE_NAME_SIZE];
	uint64_t offset;
};

struct __attribute__((packed)) fdTable {
	struct fdEntry fdEntries[FS_OPEN_MAX_COUNT];
		
};

//initialize necessary structs
struct superblock super_block;
struct rootDirEntry root_dir[MAX_ROOT_FILES];
uint16_t *fatTable;
struct fdTable fdTable;


int fs_mount(const char *diskname)
{
	// 1.) Open Disk + Error Handling
	int open_disk, read_disk, disk_size; // fat_block;
	open_disk = block_disk_open(diskname);
	if (open_disk == -1){
		//printf("Disk cannot open"); // Error checking for when testing
		return -1;
	}

	// 2.) Load Meta-info
	// reads in opened disk and reads block 0 -> store in superblock struct
	read_disk = block_read(0, &super_block);
	if (read_disk == -1){
		return -1;
	}	

	// do we compare vs ECS150-FS?
	if (memcmp("ECS150FS",super_block.signature, sizeof(super_block.signature)) != 0){ 
		return -1; 
	}

	// Obtain disk size + Error Handling
	disk_size = block_disk_count();
	if (super_block.totalVirtualDiskBlocks != disk_size){
		return -1;
	}
 
	// 2048 fat block
	//the amount fat entries is equal to #ofdatablocks
	//2048 entries max in each FAT block each entry is 2 bytes. so 4096bytes each block
	//If one creates a file system with 8192 data blocks, the size of the FAT will be 8192 x 2 = 16384 bytes long, thus spanning 16384 / 4096 = 4 blocks. 
	fatTable = malloc(super_block.fatBlocks*BLOCK_SIZE*sizeof(uint16_t)); // uint8 x 2 = uint16
	for(int i = 1; i <= super_block.fatBlocks; i++){
		// Reading in Fat contents + Error Handling
		if (block_read(i, fatTable + (i-1)*BLOCK_SIZE) == -1){
			return -1;
		}
	}

	//the first entry of the fat table has to be FAT_EOC
	if (fatTable[0] != FAT_EOC){
		return -1;
	}

// read into the root directory + Error Handling if fails
	if (block_read(super_block.rootBlockIndex, &root_dir) == -1) {
		return -1;
	}

	return 0;
}

int fs_umount(void)
{
	int close_disk, write_block, write_root;

	// writing out of disk 
	write_block = block_write(0, &super_block);
	if (write_block == -1){
		return -1;
	}

	// assuming you have to write out to root dir also???
	write_root = block_write(super_block.rootBlockIndex, &root_dir);
	if (write_root == -1){
		return -1;
	}

	// fat writing 
	for(int i = 1; i < super_block.fatBlocks; i++){
		if (block_write(i, fatTable + (i-1)*BLOCK_SIZE) == -1){
			return -1;
		};
	}
 
	close_disk = block_disk_close();
	if(close_disk == -1){
		return -1;
	}

	// deallocate memory
	free(fatTable);

	return 0;
}

int fs_info(void)
{

	/* TODO: Phase 1 Part 2*/
	// if file was not opened return -1;

	// Root Ratio Entry
	int freeRootEntry = 0;
	for (int i = 0; i < MAX_ROOT_FILES; i++){
		if (root_dir[i].fileSize == '\0') // Can't be "\0" 
			freeRootEntry++; 
	}

	// Fat Ratio Entry
	int freeFatEntry = 0;
	for (int i = 0; i < super_block.amountDataBlocks; i++){
		if (fatTable[i] == 0){
			freeFatEntry++;
		}
	}
	// Returning SuperBlock Information about ECS150-FS
	printf("FS Info:\n");
	printf("total_blk_count=%d\n", super_block.totalVirtualDiskBlocks);
	printf("fat_blk_count=%d\n", super_block.fatBlocks);
	printf("rdir_blk=%d\n", super_block.rootBlockIndex);
	printf("data_blk=%d\n", super_block.dataBlockStartIndex);
	printf("data_blk_count=%d\n", super_block.amountDataBlocks);
	printf("fat_free_ratio=%d/%d\n",freeFatEntry, super_block.amountDataBlocks);
	printf("rdir_free_ratio=%d/%d\n",freeRootEntry, MAX_ROOT_FILES);
	return 0;
}

int fs_create(const char *filename)
{

	// Error Handling
	if(filename == NULL){
		return -1;
	}

	for (int i = 0; i < MAX_ROOT_FILES; i++){
		if(root_dir[i].fileName[0] == '\0'){ 
			//char* thefile = filename;
			strcpy(root_dir[i].fileName,filename);
			root_dir[i].indexOfFirstDataBlock = FAT_EOC; // First index set to FAT_EOC
			root_dir[i].fileSize = 0; // Setting size to 0
			break;
		}
		
	}
        /* TODO: Phase 2 */
		// from fs.h
			// "String @filename must be NULL-terminated and its total
 			//* length cannot exceed %FS_FILENAME_LEN characters"
	return 0;
}

int fs_delete(const char *filename)
{
	/*
		1) find file we are looking for
		2) save that index access root_dir[index].indexoffirstdatablock
		3) clear there in the fat table?
	
	*/
        /* TODO: Phase 2 */
	// Error Handling
	if(filename == NULL){
		return -1;
	}

		// check if Fat Block is == FAT_EOC
		/* Removing a file is the opposite procedure: the file’s 
		entry must be emptied and all the data blocks containing the file’s 
		contents must be freed in the FAT.*/
		//if( ){

		//}

		uint16_t fat_position;
		// Empty for loop
		// emptying root_dir array
		//ask about memcmp vs ==
		for (int i = 0; i < MAX_ROOT_FILES; i++){
			// maybe have to use memcmp()
			if(!memcmp(root_dir[i].fileName, filename, sizeof(filename))){
			
				// empty file entry
				// file's contents freed from FAT???
				// free(root_dir[].
				//fat_position = i;
				fat_position = root_dir[i].indexOfFirstDataBlock;
				 // this is the spot we look for in our fatTable	
				root_dir[i].fileName[0] = '\0'; // setting filename to NULL? 
				root_dir[i].fileSize = 0;  //uint32_t
				root_dir[i].indexOfFirstDataBlock = FAT_EOC; // resets FAT to 0???
				return 0;
				//break; 
			}
		}
     

		// Free Fat Table contents
		fatTable[fat_position];
		uint16_t next_position;

		while(fatTable[fat_position] != FAT_EOC){
			next_position = fatTable[fat_position]; // save where to go next
			fatTable[next_position] = 0; //set current data entry to 0
			fat_position = next_position; // now move to the next pointer
		}
		fatTable[fat_position] = 0; // Once at the last spot, make it 0

	// Only need to access Fat Blocks .... not data blocks??
	// First item, either FAT EOC or a file
		// Check if FAT EOC or not
		// Save index and find EOC
		// Change to 0 

		// FAT_EOC = 0xFFFF 
		//int fat_block, fat_dataBlock = 0;
		//while(fat_block != FAT_EOC){
			//fatTable[fat_dataBlock] = 0;
			//fat_block = fatTable[fat_dataBlock];

		//}
	return 0;
}

int fs_ls(void)
{
	// Printing LS - Phase 2
	printf("FS Ls:\n");
	for (int i=0; i < MAX_FILE_NAME_SIZE; i++){
		if( root_dir[i].fileName[0] != '\0'){
			printf("file: %s, size: %d, data_blk: %d\n", root_dir[i].fileName, root_dir[i].fileSize, root_dir[i].indexOfFirstDataBlock);
		}
	}
	return 0; 
}



/*
	First check whether the file we want to open even exists in the root directory.
	Then we check for the first available empty fd entry by checking if the filename is empty.
	set offset to 0
	change filename to given input
	return the index of the for loop as the file descriptor.

					*/


int fs_open(const char *filename)
{
	int returnFD = 0;
	for (int i = 0; i < MAX_ROOT_FILES; i++){
		if(!memcmp(root_dir[i].fileName, filename, sizeof(filename))){
			for(i = 0; i < FS_OPEN_MAX_COUNT; i++){
				if(fdTable.fdEntries[i].fileName[0] == "\0"){
					memcpy(fdTable.fdEntries[i].fileName, filename, sizeof(filename));
					fdTable.fdEntries[i].offset = 0;
					returnFD = i;
					break;
				}
			}
		}
	}
	return returnFD;
}


int fs_close(int fd)
{
        fdTable.fdEntries[fd].fileName[0] = "\0";
		fdTable.fdEntries[fd].offset = 0;
}

int fs_stat(int fd)
{
	int returnFileSize = NULL;
 	char* fileName =  fdTable.fdEntries[fd].fileName;
	for (int i = 0; i < MAX_ROOT_FILES; i++){
		if(!memcmp(root_dir[i].fileName, fileName, sizeof(fileName))){
			int returnFileSize = root_dir[i].fileSize;

		}
	}
	return returnFileSize;
}

int fs_lseek(int fd, size_t offset)
{
    fdTable.fdEntries[fd].offset = offset;
}

int fs_write(int fd, void *buf, size_t count)
{
        /* TODO: Phase 4 */
}

int fs_read(int fd, void *buf, size_t count)
{
        /* TODO: Phase 4 */
}
