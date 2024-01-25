#include <xinu.h>
#include <fs.h>

extern fsystem_t* fsd;

/*
 * fs_create - Return SYSERR if not enough space is available
 *     in the directory or the filename already exists.
 *     Otherwise:
 *        1. Find an available block on the block store
 *        2. Create an inode_t for the new file
 *        3. Update the root directory
 *        4. Write the inode and free bitmask back to the block device
 */
syscall fs_create(char* filename) {
  
  //Duplicate Filename
  for(int i=0; i<DIR_SIZE; i++) {
	if(strcmp(fsd->root_dir.entry[i].name, filename)==0)
		return SYSERR;
  }

  for(int i=0; i<DIR_SIZE; i++) {
	if(fs_getmaskbit(i) == 0) {

		fs_setmaskbit(i);

		inode_t inode;
		inode.size = 0;
		inode.id = i;

		bs_write(i, 0, (void*)&inode, sizeof(inode));
		fsd->root_dir.entry[fsd->root_dir.numentries].inode_block = i;
		strcpy(fsd->root_dir.entry[fsd->root_dir.numentries].name, filename);
		fsd->root_dir.numentries++;

		return OK;
	}
  }
  return SYSERR;
}
