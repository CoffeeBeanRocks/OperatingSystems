#include <xinu.h>
#include <fs.h>

extern fsystem_t* fsd;
extern filetable_t oft[NUM_FD];

void fs_print_fsd(void) {
  kprintf("\n    fsd\n");
  kprintf("nblocks:  %d\n", fsd->device.nblocks);
  kprintf("blocksz:  %d\n", fsd->device.blocksz);
  kprintf("masksz:   %d\n", fsd->freemasksz);
  kprintf("numfiles: %d\n", fsd->root_dir.numentries);
}

void fs_print_mask(void) {
  int i, j;

  kprintf("\n    free mask\n");
  for (i=0; i<fsd->freemasksz; i++) {
    for (j=0; j<8; j++)
      if (i==0 && j<2)
	kprintf("%c", (fs_getmaskbit((i*8)+j) ? 's' : '0'));
      else
	kprintf("%d", fs_getmaskbit((i*8)+j));
    kprintf("%c", ((i+1) % 8 ? ' ' : '\n'));
  }
  if ((i+1) % 8)
    kprintf("\n");
}

void fs_print_oft(void) {
  int i;

  kprintf("\n    oft\n");
  kprintf("Num  state  fileptr  in.id  flag  name\n");
  for (i=0; i<NUM_FD; i++) {
    kprintf("%3d  %5s  %7d  %5d  %4d  %s 0x%x\n", i,
	    (oft[i].state == FSTATE_OPEN ? "OPEN" : "CLOSE"),
	    oft[i].fileptr,
	    oft[i].in.id, oft[i].flag,
	    fsd->root_dir.entry[oft[i].de].name,
	    &oft);
  }
}

void fs_print_root(void) {
  int i;
  kprintf("\n    root directory [%d entries]\n", fsd->root_dir.numentries);
  kprintf("ID  block  name          blocks\n");
  for (i=0; i<fsd->root_dir.numentries; i++) {
    kprintf("%2d %5d %s \n", i,
	   fsd->root_dir.entry[i].inode_block,
	   fsd->root_dir.entry[i].name);
  }
}

void fs_print_fd(int fd) {
  int sz=0,i=0;

  kprintf("\n    file descriptor [%d]\n", fd);
  kprintf("Name:    %s\n", fsd->root_dir.entry[oft[fd].de].name);
  kprintf("State:   %d\n", oft[fd].state);
  kprintf("Flag:    %d\n", oft[fd].flag);
  kprintf("Fileptr: %d\n", oft[fd].fileptr);
  kprintf("Size:    %d\n", oft[fd].in.size);
  kprintf("\nblocks: ");
  while (sz < oft[fd].in.size && i <= INODE_BLOCKS) {
    int bsize = oft[fd].in.size - i;
    bsize = (bsize > fsd->device.blocksz ? fsd->device.blocksz : bsize);
    sz += bsize;
    kprintf(" %d", oft[fd].in.blocks[i]);
    i++;
  }
  kprintf("\n");
}
