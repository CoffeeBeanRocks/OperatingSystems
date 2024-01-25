#include <xinu.h>
#include <fs.h>

fsystem_t* fsd = NULL;
filetable_t oft[NUM_FD];

void fs_setmaskbit(int x) {
  if (fsd == NULL) return;
  fsd->freemask[x / 8] |= 0x1 << (x % 8);
}

void fs_clearmaskbit(int x) {
  if (fsd == NULL) return;
  fsd->freemask[x / 8] &= ~(0x1 << (x % 8));
}

int fs_getmaskbit(int x) {
  if (fsd == NULL) return -1;
  return (fsd->freemask[x / 8] >> (x % 8)) & 0x1;
}

syscall fs_mkfs(void) {
  intmask mask;
  fsystem_t fsd;
  bdev_t device = bs_stats();
  uint32 masksize, i;
  mask = disable();
  
  masksize = device.nblocks / 8;
  masksize += (device.nblocks % 8 ? 0 : 1);
  fsd.device = device;
  fsd.freemasksz = masksize;
  fsd.freemask = getmem(masksize);
  fsd.root_dir.numentries = 0;

  for (i=0; i<masksize; i++)
    fsd.freemask[i] = 0;

  for (i=0; i<DIR_SIZE; i++) {
    fsd.root_dir.entry[i].inode_block = EMPTY;
    memset(fsd.root_dir.entry[i].name, 0, FILENAME_LEN);
  }
  
  fsd.freemask[SB_BIT / 8] |= 0x1 << (SB_BIT % 8);  
  fsd.freemask[BM_BIT / 8] |= 0x1 << (BM_BIT % 8);
  bs_write(SB_BIT, 0, &fsd, sizeof(fsystem_t));
  bs_write(BM_BIT, 0, fsd.freemask, fsd.freemasksz);
  freemem(fsd.freemask, fsd.freemasksz);
  
  restore(mask);
  return OK;
}

syscall fs_mount(void) {
  intmask mask;
  int i;

  mask = disable();
  if ((fsd = (fsystem_t*)getmem(sizeof(fsystem_t))) == (fsystem_t*)SYSERR) {
    restore(mask);
    return SYSERR;
  }
  bs_read(SB_BIT, 0, fsd, sizeof(fsystem_t));
  if ((fsd->freemask = getmem(fsd->freemasksz)) == (char*)SYSERR) {
    restore(mask);
    return SYSERR;
  }
  bs_read(BM_BIT, 0, fsd->freemask, fsd->freemasksz);

  for (i=0; i<NUM_FD; i++) {
    oft[i].state = FSTATE_CLOSED;
    oft[i].fileptr = 0;
    oft[i].de = 0;
    oft[i].flag = 0;
  }
  

  restore(mask);
  return OK;
}

syscall fs_umount(void) {
  intmask mask = disable();

  bs_write(BM_BIT, 0, fsd->freemask, fsd->freemasksz);
  bs_write(SB_BIT, 0, fsd, sizeof(fsystem_t));

  freemem(fsd->freemask, fsd->freemasksz);
  freemem((char*)fsd, sizeof(fsystem_t));
  
  restore(mask);
  return OK;
}

