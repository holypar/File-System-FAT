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

// TO Do
	// 1.) Comments
	// 2.) Lookover for error Handling/edge cases
	// 3.) Clean up any code


// Superblock Struct
struct __attribute__((packed)) superblock { 
	uint8_t signature[8]; 
	uint16_t totalVirtualDiskBlocks;
	uint16_t rootBlockIndex;
	uint16_t dataBlockStartIndex;
	uint16_t amountDataBlocks; 
	uint8_t fatBlocks;
	uint8_t padding[4079]; 
};

// Root Directory Struct
struct __attribute__((packed)) rootDirEntry {
	uint8_t fileName[MAX_FILE_NAME_SIZE];  //16 bytes
	uint32_t fileSize;	   // 4 bytes
	uint16_t indexOfFirstDataBlock; // 2 bytes
	uint8_t unused[10];     // 10 bytes
	//total 32 bytes
};

// Struct to make filename NULL for mount
struct __attribute__((packed)) fdEntry {
	uint8_t fileName[MAX_FILE_NAME_SIZE];
	uint64_t offset;
	int status; // 0 is available;  1 is unvaiable
};

// Struct for File Descriptors
struct __attribute__((packed)) fdTable {
	int openFiles;
	struct fdEntry fdEntries[FS_OPEN_MAX_COUNT];	
};

// Initializing Structs
struct superblock super_block;
struct rootDirEntry root_dir[MAX_ROOT_FILES];
uint16_t *fatTable;
struct fdTable fdTable;

// Global Variable for if Virtual Disk is opened (For )
int opened_vd = -1;


int fs_mount(const char *diskname)
{
    // Open Disk + Error Handling
	int open_disk, read_disk, disk_size; // fat_block;

	open_disk = block_disk_open(diskname);
	if (open_disk == -1){
		//printf("Disk cannot open"); // Error checking for when testing
		return -1;
	}

	// Reads in opened disk and reads block 0 -> store in superblock struct
	read_disk = block_read(0, &super_block);
	if (read_disk == -1){
		return -1;
	}	

	// Error Handling to check if signature matches
	if (memcmp("ECS150FS", super_block.signature, sizeof(super_block.signature)) != 0){ 
		return -1;  
	}

	// If Virtual Disk was opened successfully, switch from -1 to 0.
	opened_vd = 0;


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

	// The first entry of the fat table has to be FAT_EOC (Error Handling)
	if (fatTable[0] != FAT_EOC){
		return -1;
	}

	// Read into the root directory + Error Handling 
	if (block_read(super_block.rootBlockIndex, &root_dir) == -1) {
		return -1;
	}

	// Checking if fd entries match 0
	for (int i = 0; i < FS_OPEN_MAX_COUNT; i++){
		fdTable.fdEntries[i].status = 0; 
	}

	return 0;
}

int fs_umount(void)
{
	int close_disk, write_block, write_root;

	// If FS not mounted to write + Error Handling 
	write_block = block_write(0, &super_block);
	if (write_block == -1){
		return -1;
	}

	// Writing to root index + Error Handling
	write_root = block_write(super_block.rootBlockIndex, &root_dir);
	if (write_root == -1){
		return -1;
	}

	// Fat table writing + Error Handling
	for(int i = 1; i < super_block.fatBlocks; i++){
		if (block_write(i, fatTable + (i-1)*BLOCK_SIZE) == -1){
			return -1;
		}
	}
 
	// Close disk + Error Handling
	close_disk = block_disk_close();
	if(close_disk == -1){
		return -1;
	}

	// Deallocate memory
	free(fatTable);

	return 0;
}

int fs_info(void)
{

	if (opened_vd == -1){
		return -1;
	}

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
	// Returning SuperBlock Information about ECS150FS
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

	if(filename == NULL || opened_vd == -1 || strlen(filename) > FS_FILENAME_LEN){
		return -1;
	}

	int file_exists = 0;
	// Error Handling loop to check if file already exists in directory
	for (int i = 0; i < MAX_ROOT_FILES; i++){
		if(root_dir[i].fileName[0] != '\0'){
			file_exists++;
		}
		if(strcmp((char*)root_dir[i].fileName, filename) == 0){
			return -1; // Means file already exists in Root Directory
		}
		// Error Handling if Directory has reached its max files
		if (file_exists == FS_FILE_MAX_COUNT){
			return -1; 
		}
	}

	for (int i = 0; i < MAX_ROOT_FILES; i++){
		if(root_dir[i].fileName[0] == '\0'){ 
			memcpy(root_dir[i].fileName,filename, FS_FILENAME_LEN);
			root_dir[i].indexOfFirstDataBlock = FAT_EOC; // First index set to FAT_EOC
			root_dir[i].fileSize = 0; // Setting size to 0
			break;
		}
		
	}

	return 0;
}

int fs_delete(const char *filename)
{
	// Error Handling
	if(filename == NULL || opened_vd == -1 || strlen(filename) > FS_FILENAME_LEN){
		return -1;
	}

		uint16_t fat_position = 9999;
		// Empty for loop
		// emptying root_dir array
		//ask about memcmp vs ==
		for (int i = 0; i < MAX_ROOT_FILES; i++){
			// maybe have to use memcmp()
			if(!memcmp(root_dir[i].fileName, filename, FS_FILENAME_LEN)){
				fat_position = root_dir[i].indexOfFirstDataBlock;
				 // this is the spot we look for in our fatTable	
				root_dir[i].fileName[0] = '\0'; // setting filename to NULL? 
				root_dir[i].fileSize = 0;  //uint32_t
				root_dir[i].indexOfFirstDataBlock = FAT_EOC; // resets FAT to 0???
				return 0;
			}
		}
     

		// Free Fat Table contents
		uint16_t next_position;

		while(fatTable[fat_position] != FAT_EOC){
			next_position = fatTable[fat_position]; // save where to go next
			fatTable[next_position] = 0; //set current data entry to 0
			fat_position = next_position; // now move to the next pointer
		}
		fatTable[fat_position] = 0; // Once at the last spot, make it 0
	
	return 0;
}

int fs_ls(void)
{
	// Error Handling if Disk is not opened
	if (opened_vd == -1){
		return -1;
	}

	printf("FS Ls:\n");
	for (int i=0; i < MAX_FILE_NAME_SIZE; i++){
		if( root_dir[i].fileName[0] != '\0'){
			printf("file: %s, size: %d, data_blk: %d\n", root_dir[i].fileName, root_dir[i].fileSize, root_dir[i].indexOfFirstDataBlock);
		}
	}
	return 0; 
}


int fs_open(const char *filename)
{
	// Error Handling
	// 32 files can't be opened ; FS_OPEN_MAX_COUNT 
	if (filename == NULL || opened_vd == -1 || strlen(filename) > FS_FILENAME_LEN){
		return -1;
	}

	if (fdTable.openFiles > FS_OPEN_MAX_COUNT){
		return -1;
	}

	int returnFD = 0; //1;
	for (int i = 0; i < MAX_ROOT_FILES; i++){ // max root = 128
		if(strcmp((char*)root_dir[i].fileName, filename) == 0){      // check if filename even exists in the root dir beforehand
			for(int i = 0; i < FS_OPEN_MAX_COUNT; i++){ //Max Count = 32
				if((fdTable.fdEntries[i].status == 0)){		
					memcpy(fdTable.fdEntries[i].fileName, filename, FS_FILENAME_LEN); // FS_FILENAME_LEN = 16         check for first FDentry that's status is 0 
					fdTable.fdEntries[i].offset = 0;
					fdTable.fdEntries[i].status = 1;
					fdTable.openFiles += 1;
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
	// Error Handling if Out of Bounds 0 < x < 32 and if file is not opened
	if (fd > FS_OPEN_MAX_COUNT || fd < 0 || fdTable.fdEntries[fd].fileName[0] == '\0'){
		return -1;
	}

    fdTable.fdEntries[fd].status = 0;
	fdTable.fdEntries[fd].offset = 0;
	fdTable.openFiles -= 1;
	return 0;
}

int fs_stat(int fd)
{

	if (fd > FS_OPEN_MAX_COUNT || fd < 0 || fdTable.fdEntries[fd].fileName[0] == '\0'){
		return -1;
	}

	int returnFileSize = -1;
 	

	for (int i = 0; i < MAX_ROOT_FILES; i++){
		//once a file in the root directory matches the fdentry file name, we return that rootentry filesize
		if(strcmp((char*)root_dir[i].fileName, (char*)fdTable.fdEntries[fd].fileName) == 0){
		 	returnFileSize = root_dir[i].fileSize; //updates filesize
		}
	}
	
	return returnFileSize;
}

int fs_lseek(int fd, size_t offset)
{

	if (fd > FS_OPEN_MAX_COUNT || fd < 0 || fd > 31 || fdTable.fdEntries[fd].fileName[0] == '\0'){
		return -1;
	}

	//if someone is trying to go somewhere which extends off the size of the actual file. 
	if ((int)offset > fs_stat(fd)){      
		return -1;
	}

    fdTable.fdEntries[fd].offset = offset;
	return 0;
}

// Finds where to start reading
uint16_t offset_helper(uint64_t offset, uint16_t firstDataBlockIndex){

	int move = offset / BLOCK_SIZE;
	int nextPosition = 0;
	//uint16_t indexReadFirstDataBlock = 999;

	for (int i = 0; i < move; i++){ // less than or equal to? say when the offset of the file is 3000 then move = 0 
		nextPosition = fatTable[firstDataBlockIndex];
		firstDataBlockIndex = nextPosition;
	}
	return firstDataBlockIndex; 
}

int fs_write(int fd, void *buf, size_t count)
{
	// Error Handling
	if (buf == NULL){
		return -1;
	}
	if (fd > FS_OPEN_MAX_COUNT || fd < 0 || fd > 31 ||fdTable.fdEntries[fd].fileName[0] == '\0'){
		return -1;
	}

	void* bounceBuffer = malloc(BLOCK_SIZE);
	uint64_t offset = fdTable.fdEntries[fd].offset;
	uint64_t initialOffset = fdTable.fdEntries[fd].offset;
	uint16_t indexReadFirstDataBlock = 999;
	

	for(int i = 0; i < MAX_ROOT_FILES; i++){
		if(strcmp((char*)root_dir[i].fileName, (char*)fdTable.fdEntries[fd].fileName) == 0){
			indexReadFirstDataBlock = root_dir[i].indexOfFirstDataBlock; //saves index of first data block
			break;
		}
	}

	uint64_t offset_bounced = offset % BLOCK_SIZE; // offset is only a part of the data block we want to get if user asks for it offset of the data block below this.
	uint16_t datablockindex = offset_helper(offset, indexReadFirstDataBlock); // locates index of first data block to ACTUALLY READ IN ACCOUNTING FOR THE OFFSET for example if the FILE'S offset is 5000 we need to start reading at block 2nd block.

	int bufTracking = 0;
	int howManyBlocksToRead = ((offset_bounced + count) / BLOCK_SIZE) + 1;  //example 3000 block offset want to read 10000.  // 3 // 4 blocks for 3000 fof and 10000 read good
	size_t totalBytesTransferred = 0;

	//large operation
	if (offset_bounced + count > BLOCK_SIZE){
		//first block
		block_read(datablockindex + super_block.dataBlockStartIndex, bounceBuffer); 
		memcpy(bounceBuffer + offset_bounced, buf, BLOCK_SIZE - offset_bounced);
		block_write(datablockindex + super_block.dataBlockStartIndex, bounceBuffer);
		totalBytesTransferred += BLOCK_SIZE - offset_bounced;
		bufTracking += BLOCK_SIZE - offset_bounced;
		offset +=  BLOCK_SIZE - offset_bounced;
		fdTable.fdEntries[fd].offset +=  BLOCK_SIZE - offset_bounced;
		howManyBlocksToRead--;

		// all inner blocks + that edge case
		for (int i = 1; i < howManyBlocksToRead; i++ ){
			datablockindex = offset_helper(offset, indexReadFirstDataBlock);
			if (fatTable[datablockindex] == FAT_EOC){
				int new_index = -1;
				for (int i = 0; i < super_block.fatBlocks; i++){
					if (fatTable[i] == 0){
						new_index = i; 
						fatTable[datablockindex] = new_index;
						fatTable[i] = FAT_EOC;
						break;
					}
				}
				if (new_index == -1){
					return totalBytesTransferred; // No more bytes/space to be written
				}
				datablockindex = offset_helper(offset, indexReadFirstDataBlock);
				block_write(datablockindex + super_block.dataBlockStartIndex, buf + bufTracking); // i can read these full blocks directly into the return buf
				bufTracking += BLOCK_SIZE; //we read in 1 block each time
				fdTable.fdEntries[fd].offset += BLOCK_SIZE;
				offset += BLOCK_SIZE;
				totalBytesTransferred += BLOCK_SIZE;
				if (totalBytesTransferred == count){ // edge case
					free(bounceBuffer);
					return totalBytesTransferred; // this handles the case where the initial file offset is 3000 and i want to read 5192 bytes. so basically the last block i read is exactly 4096 bytes.
				}
				
			}
		}
		datablockindex = offset_helper(offset, indexReadFirstDataBlock);
			if (fatTable[datablockindex] == FAT_EOC){
				int new_index = -1;
				for (int i = 0; i < super_block.fatBlocks; i++){
					if (fatTable[i] == 0){
						new_index = i; 
						fatTable[datablockindex] = new_index;
						fatTable[i] = FAT_EOC;
						break;
					}
				}
				if (new_index == -1){
					return totalBytesTransferred; // No more bytes/space to be written
				}
				datablockindex = offset_helper(offset, indexReadFirstDataBlock);
				block_read(datablockindex + super_block.dataBlockStartIndex, bounceBuffer);
				memcpy(bounceBuffer, buf + bufTracking, ((initialOffset + count) % BLOCK_SIZE));
				block_write(datablockindex + super_block.dataBlockStartIndex, bounceBuffer);	
				bufTracking += BLOCK_SIZE; //we read in 1 block each time
				fdTable.fdEntries[fd].offset += BLOCK_SIZE;
				offset += BLOCK_SIZE;
				totalBytesTransferred += BLOCK_SIZE;
				free(bounceBuffer);
				return totalBytesTransferred;
		}
	}
	//Small Operation
	if(offset_bounced + count <= BLOCK_SIZE){
		block_read(datablockindex + super_block.dataBlockStartIndex, bounceBuffer); // copies the WHOLE block in where the offset is at.
		// we need to write inside of the bouncebuffer from bounceBuffer + offset_bounced count times
		memcpy(bounceBuffer + offset_bounced, buf, count); //we only want to copy over from the offset of that datablock and how much we want to read.
		block_write(datablockindex + super_block.dataBlockStartIndex, bounceBuffer);
		fdTable.fdEntries[fd].offset += count;
		totalBytesTransferred += count;
		free(bounceBuffer);
		return totalBytesTransferred;
	}
	return totalBytesTransferred;
}


int fs_read(int fd, void *buf, size_t count)
{
	// Error Handling
	if (buf == NULL){
		return -1;
	}
	if (fd > FS_OPEN_MAX_COUNT || fd < 0 || fd > 31 || fdTable.fdEntries[fd].fileName[0] == '\0'){
		return -1;
	}
	
	void* bounceBuffer = malloc(BLOCK_SIZE);
	uint64_t offset = fdTable.fdEntries[fd].offset;
	uint64_t initialOffset = fdTable.fdEntries[fd].offset;
	uint16_t indexReadFirstDataBlock = 999;

	for(int i = 0; i < MAX_ROOT_FILES; i++){
		if(strcmp((char*)root_dir[i].fileName, (char*)fdTable.fdEntries[fd].fileName) == 0){
			indexReadFirstDataBlock = root_dir[i].indexOfFirstDataBlock; //saves index of first data block
			break;
		}
	}

	uint64_t offset_bounced = offset % BLOCK_SIZE; // offset is only a part of the data block we want to get if user asks for it offset of the data block below this.
	uint16_t datablockindex = offset_helper(offset, indexReadFirstDataBlock); // locates index of first data block to ACTUALLY READ IN ACCOUNTING FOR THE OFFSET for example if the FILE'S offset is 5000 we need to start reading at block 2nd block.

	int bufTracking = 0;
	int howManyBlocksToRead = ((offset_bounced + count) / BLOCK_SIZE) + 1;  //example 3000 block offset want to read 10000.  // 3 // 4 blocks for 3000 fof and 10000 read good
	size_t totalBytesTransferred = 0;

	if((offset_bounced + count) % BLOCK_SIZE == 0){
		howManyBlocksToRead--;  //2 
	} 

	// Larger/Bigger Operation
		if (offset_bounced + count > BLOCK_SIZE){
			//copy over  from the location of first data block offset to the end of that block
			block_read(datablockindex + super_block.dataBlockStartIndex, bounceBuffer); // read in WHOLE block into the bouncebuffer
			memcpy(buf, bounceBuffer + offset_bounced, BLOCK_SIZE - offset_bounced);  // only import from offset to end of THAT block
			bufTracking += BLOCK_SIZE - offset_bounced; //next time we copy into it, we copy into buf + buftracking
			howManyBlocksToRead--;  // 1  //3
			fdTable.fdEntries[fd].offset += BLOCK_SIZE - offset_bounced;
			offset += BLOCK_SIZE - offset_bounced;
			totalBytesTransferred +=  BLOCK_SIZE - offset_bounced;
	// inner blocks read fully
			for (int i = 1; i < howManyBlocksToRead; i++ ){
				datablockindex = offset_helper(offset, indexReadFirstDataBlock); // this is now the next index
				block_read(datablockindex + super_block.dataBlockStartIndex, buf + bufTracking); // i can read these full blocks directly into the return buf
				bufTracking += BLOCK_SIZE; //we read in 1 block each time
				fdTable.fdEntries[fd].offset += BLOCK_SIZE;
				offset += BLOCK_SIZE;
				totalBytesTransferred += BLOCK_SIZE;
				if (totalBytesTransferred == count){
					free(bounceBuffer);
					return totalBytesTransferred; // this handles the case where the initial file offset is 3000 and i want to read 5192 bytes. so basically the last block i read is exactly 4096 bytes.
				}
		}
			//finally read the leftover data in the LAST datablock if needed
			datablockindex = offset_helper(offset, indexReadFirstDataBlock);
			block_read(datablockindex + super_block.dataBlockStartIndex, bounceBuffer); //copy whole block into bouncebuffer
			memcpy(buf + bufTracking, bounceBuffer, ((initialOffset + count) % BLOCK_SIZE));
			fdTable.fdEntries[fd].offset += ((initialOffset + count) % BLOCK_SIZE);
			offset += (initialOffset + count) % BLOCK_SIZE;
			totalBytesTransferred += ((initialOffset + count) % BLOCK_SIZE);
			free(bounceBuffer);
			return totalBytesTransferred;
	}

	// Small Operation
	if(offset_bounced + count <= BLOCK_SIZE){
		block_read(datablockindex + super_block.dataBlockStartIndex, bounceBuffer); // copies the WHOLE block in where the offset is at.
		memcpy(buf, bounceBuffer + offset_bounced, count); //we only want to copy over from the offset of that datablock and how much we want to read.
		fdTable.fdEntries[fd].offset += count;
		totalBytesTransferred += count;
		free(bounceBuffer);
		return totalBytesTransferred;
	}

	return totalBytesTransferred;
}