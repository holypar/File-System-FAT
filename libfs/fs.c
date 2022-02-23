#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "disk.h"
#include "fs.h"

#define BLOCK_SIZE 4096
#define FAT_EOC 0xFFFF


/* TODO: Phase 1 */
struct __attribute__((packed)) superblock { 
	uint64_t signature; 
	uint16_t totalVirtualDiskBlocks;
	uint16_t rootBlockIndex;
	uint16_t dataBlockStartIndex;
	uint16_t amountDataBlocks; 
	uint8_t fatBlocks;
	uint8_t padding[4079]; 
};

struct __attribute__((packed)) rootdir {
	u_int8_t filename[16];
	uint32_t sizeFile;
	uint16_t indexDataBlock;
	uint8_t padding[10];
};

// struct __attribute__((packed)) filedes {
// 	uint16_t file_offset;
// };

struct superblock super_block;
struct rootdir root_dir;
//struct filedes file_des;
uint16_t *fatTable;



int fs_mount(const char *diskname)
{
        /* TODO: Phase 1 */
		// Steps: 
			// 1.) Open Virtual Disk using API Block
			// 2.) Load/Read the Meta-Info to handle file operations
			// 3.) Need to add Error Checking 

	// 1.) Open Disk + Error Handling
	int open_disk, read_disk, disk_size, fat_block;
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
	
	//FAT Block Next??? Obtain Disk size??
	// "For example, the signature of the file system should correspond to the 
	// one defined by the specifications, the total amount of block should correspond to what block_disk_count() returns, etc"

	// Compare if ECS150-FS matches expected file system
	// Returns 0 if matches, otherwise not a 0 if mis-match
	if (strcmp("ECS150-FS", super_block.signature) != 0){
		return -1;
	}

	// Obtain disk size + Error Handling
	disk_size = block_disk_count();
	if (super_block.totalVirtualDiskBlocks != disk_size){
		return -1;
	}
 
 
	fatTable = malloc(BLOCK_SIZE*sizeof(uint16_t));
	// Fat:
		// Allocate memory into our pointer array??
		// Create a loop to insert info into Fat Table???
		// Add fat error handling?


// read into the root directory + Error Handling if fails
	if (block_read(super_block.rootBlockIndex, &root_dir) == -1) {
		return -1;
	}


	for (int i = 0; i < super_block.fatBlocks; i++) {
		block_read(i, &fatTable); //&fatTable[BLOCK_SIZE]);
	}

	return 0;
}

int fs_umount(void)
{
        /* TODO: Phase 1 */
		// Steps:
			// 1.) Internal Data Structures of FS layer are cleaned 
			// 2.) Virtual Disk is properly closed

	// "This means that whenever fs_umount() is called, all meta-information and 
	// file data must have been written out to disk."
	
	int close_disk, write_block;

	write_block = block_write(0, &super_block);
	if (write_block == -1){
		return -1;
	}

	// Fat here??? 
	// free(???)/clean Fat Table???

	close_disk = block_disk_close();
	if(close_disk == -1){
		//printf("Disk could not be closed");
		return -1;
	}
}

int fs_info(void)
{
	/* TODO: Phase 1 Part 2*/
	// if file was not opened return -1;
	
	// Returning SuperBlock Information about ECS150-FS
	printf("FS Information\n");
	printf("Signature%s\n", super_block.signature);
	printf("Total block count%d\n", super_block.totalVirtualDiskBlocks);
	printf("Total FAT block count%d\n", super_block.fatBlocks);
	printf("Total Data block count%d", super_block.amountDataBlocks);
	printf("Root Block Index%d\n", super_block.rootBlockIndex);
	printf("Data Blocks starting Index%d\n", super_block.dataBlockStartIndex);
	
	return 0;
}

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
