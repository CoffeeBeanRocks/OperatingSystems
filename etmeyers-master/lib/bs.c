#include <xinu.h>
#include <fs.h>

local bdev_t ramdisk;
local char* ramfs_blocks = NULL;

bdev_t bs_stats(void) {
  return ramdisk;
}

syscall bs_mk_ramdisk(int blocksize, int numblocks) {
  intmask mask = disable();
  ramdisk.blocksz = (blocksize == FS_DEFAULT ? MDEV_BLOCK_SIZE : blocksize);
  ramdisk.nblocks = (numblocks == FS_DEFAULT ? MDEV_NUM_BLOCKS : numblocks);
  ramfs_blocks = getmem(ramdisk.blocksz * ramdisk.nblocks);
  restore(mask);
  return (ramfs_blocks == (void*)SYSERR ? SYSERR : OK);
}

syscall bs_free_ramdisk(void) {
  intmask mask;
  syscall result;
  if (ramfs_blocks == NULL) {
    return SYSERR;
  }
  mask = disable();
  result = freemem(ramfs_blocks, ramdisk.nblocks * ramdisk.blocksz);
  restore(mask);
  return result;
}

syscall bs_read(int block, int offset, void* buf, int len) {
  intmask mask = disable();
  if (offset < 0 || offset + len > ramdisk.blocksz ||
      block < 0 || block >= ramdisk.nblocks ||
      ramfs_blocks == NULL) {
    restore(mask);
    return SYSERR;
  }
  memcpy(buf, &(ramfs_blocks[block * ramdisk.blocksz]) + offset, len);
  restore(mask);
  return OK;
}

syscall bs_write(int block, int offset, void* buf, int len) {
  intmask mask = disable();
  if (offset < 0 || offset + len > ramdisk.blocksz ||
      block < 0 || block >= ramdisk.nblocks ||
      ramfs_blocks == NULL) {
    restore(mask);
    return SYSERR;
  }
  memcpy(&(ramfs_blocks[block * ramdisk.blocksz]) + offset, buf, len);
  restore(mask);
  return OK;
}
