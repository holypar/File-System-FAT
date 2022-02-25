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

//to do.
//do both ratios 

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


//rootdir struct
/*
	is a single block YES
	32 byte entry per file  CHECKED
	128 entries in total YES
	*/
// struct __attribute__((packed)) rootdir {
// 	struct rootDirEntry rootEntries[MAX_ROOT_FILES];  //4096 bytes = 1 block
// };
// we are reading 1 block of 4096 bytes 
// but we need it to be in 128 different files of each 32 bytes

//initialize necessary structs
struct superblock super_block;
struct rootDirEntry root_dir[MAX_ROOT_FILES];
uint16_t *fatTable;


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
	//fatTable = malloc(super_block.amountDataBlocks*BLOCK_SIZE*sizeof(uint16_t));
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
	//printf("Signature %s\n", super_block.signature);
	printf("total_blk_count=%d\n", super_block.totalVirtualDiskBlocks);
	printf("fat_blk_count=%d\n", super_block.fatBlocks);
	printf("rdir_blk=%d\n", super_block.rootBlockIndex);
	printf("data_blk=%d\n", super_block.dataBlockStartIndex);
	printf("data_blk_count=%d\n", super_block.amountDataBlocks);
	printf("fat_free_ratio=%d/%d\n",freeFatEntry, super_block.amountDataBlocks);
	printf("rdir_free_ratio=%d/%d\n",freeRootEntry, MAX_ROOT_FILES);
	
	return 0;
}

/* Commenting Sections for Phase 1 Testing */

int fs_create(const char *filename)
{
        /* TODO: Phase 2 */
}

int fs_delete(const char *filename)
{
        /* TODO: Phase 2 */
}

int fs_ls(void)
{
        /* TODO: Phase 2 */
}

int fs_open(const char *filename)
{
        /* TODO: Phase 3 */
}

int fs_close(int fd)
{
        /* TODO: Phase 3 */
}

int fs_stat(int fd)
{
        /* TODO: Phase 3 */
}

int fs_lseek(int fd, size_t offset)
{
        /* TODO: Phase 3 */
}

int fs_write(int fd, void *buf, size_t count)
{
        /* TODO: Phase 4 */
}

int fs_read(int fd, void *buf, size_t count)
{
        /* TODO: Phase 4 */
}
