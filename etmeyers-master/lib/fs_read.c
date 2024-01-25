#include <xinu.h>
#include <fs.h>

extern fsystem_t* fsd;
extern filetable_t oft[NUM_FD];

/*
 * Copy the data from the block device into the 'buff' argument
 *    Use the fileptr field of the oft to track the location of the next read/write
 *    operation.
 *    - Determine the current block and offset from the fileptr
 *    - Copy bytes into the 'buff' until you have exhausted the block or copied 'len' bytes
 *    - Repeat until 'len' bytes are copied
 *    - Update the fileptr field
 *    - Return the number of bytes read
 */
int fs_read(int fd, char* buff, int len) {
  return 0;
}
