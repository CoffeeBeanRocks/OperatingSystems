#ifndef FS_H
#define FS_H

#define FS_DEFAULT 0

#define SB_BIT 0
#define BM_BIT 1

#define FILENAME_LEN 16
#define DIR_SIZE     16

#define INODE_BLOCKS    12
#define MDEV_BLOCK_SIZE 512
#define MDEV_NUM_BLOCKS 512

#define O_RDONLY      0
#define O_WRONLY      1
#define O_RDWR        2
#define FSTATE_CLOSED 0
#define FSTATE_OPEN   1
#define NUM_FD       16

typedef struct inode {
  int id;
  int size;
  int blocks[INODE_BLOCKS];
} inode_t;

typedef struct dirent {
  int inode_block;
  char name[FILENAME_LEN];
} dirent_t;

typedef struct directory {
  int numentries;
  dirent_t entry[DIR_SIZE];
} directory_t;

typedef struct bdev {
  int nblocks;
  int blocksz;
} bdev_t;

typedef struct fsystem {
  bdev_t device;
  int freemasksz;
  char* freemask;
  directory_t root_dir;
} fsystem_t;

typedef struct filetable {
  int state;
  int fileptr;
  int de;
  inode_t in;
  int flag;
} filetable_t;



bdev_t bs_stats(void);
syscall bs_mk_ramdisk(int, int);
syscall bs_free_ramdisk(void);
syscall bs_read(int, int, void*, int);
syscall bs_write(int, int, void*, int);

void fs_setmaskbit(int);
void fs_clearmaskbit(int);
int fs_getmaskbit(int);

syscall fs_mkfs(void);
syscall fs_mount(void);
syscall fs_umount(void);

void fs_print_fsd(void);
void fs_print_mask(void);
void fs_print_oft(void);
void fs_print_root(void);
void fs_print_fd(int fd);

syscall fs_create(char*);
int     fs_open(char*, int);
syscall fs_close(int);

int     fs_read(int, char*, int);
int     fs_write(int, char*, int);

#endif
