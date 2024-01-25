#include <xinu.h>
#include <fs.h>

extern fsystem_t* fsd;
extern filetable_t oft[NUM_FD];

/*
 * fs_open - Return SYSERR if the file is already open or does not exist
 *     otherwise, add a record to the oft in the first current FSTATE_CLOSED
 *     slot corresponding to the opened file and copy the inode from the block
 *     device to the oft record and return the index into the oft table for the
 *     newly opened file.
 */
int fs_open(char* filename, int flags) {

  for(int i=0; i<NUM_FD; i++) {
	if((strcmp(fsd->root_dir.entry[oft[i].in.id].name, filename)) && oft[i].state == FSTATE_OPEN)
		return SYSERR;
  }

  for(int i=0; i<NUM_FD; i++) {
	if(oft[i].state == FSTATE_CLOSED) {
		oft[i].state = FSTATE_OPEN;
		oft[i].flag = flags;
		for(int j=0; j<DIR_SIZE; j++) {
			if(strcmp(fsd->root_dir.entry[j].name, filename) == 0)
				bs_read(fsd->root_dir.entry[j].inode_block, 0, (void*)&oft[i].in, sizeof(oft[i].in));
		}
		return i;
	}
  }  

  return SYSERR;
}
