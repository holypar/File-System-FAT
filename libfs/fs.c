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
	char filename[16]; //u_int8 filename[16] ?
	uint32_t sizeFile;
	uint16_t indexDataBlock;
	uint8_t padding[10];
};


/*
	big array of 16 bit entries : linked list of data blocks composing a file:
	2048 entries per fat block
	spans multiple blocks if more than 2048 data blocks to manage. 
*/
struct __attribute__((packed)) fat {
	pass;
	// not sure if this a 16bit ptr or uint16_t fat[2048]
};


int fs_mount(const char *diskname)
{
        /* TODO: Phase 1 */
		// Steps: 
			// 1.) Open Virtual Disk using API Block
			// 2.) Load the Meta-Info to handle file operations
			// 3.) Need to add Error Checking 
}

int fs_umount(void)
{
        /* TODO: Phase 1 */
		// Steps: 
			// 1.) Virtual Disk is properly closed
			// 2.) Internal Data Structures of FS layer are cleaned
}

int fs_info(void)
{
        /* TODO: Phase 1 Part 2*/
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
