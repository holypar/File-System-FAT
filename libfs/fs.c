#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "disk.h"
#include "fs.h"

/* TODO: Phase 1 */
// 1 byte = 8 bits
struct superblock { 
	uint64_t signature[8]; //gotta check this 
	u_int16_t totalVirtualDiskBlocks;
	u_int16_t rootBlockIndex;
	u_int16_t dataBlockStartIndex;
	u_int16_t amountDataBlocks; 
	u_int8_t fatBlocks;
	u_int8_t padding[4079];  // check this too! 
};

struct rootdir {
	char filename[16]; 
	uint32_t sizeFile;
	uint16_t indexDataBlock;
	uint8_t padding[10];
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
