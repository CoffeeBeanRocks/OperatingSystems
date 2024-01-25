#include <xinu.h>
#include <string.h>
#include <stdio.h>
#include <fs.h>

#define TEST_OPEN   0x1
#define TEST_CLOSE  0x2
#define TEST_CREATE 0x4
#define TEST_WRITE  0x8
#define TEST_READ   0x10

#define BLOCK_VAL 0xBAD
#define BLOCK_IDX 5

#define FAIL 0
#define PASS 1

extern fsystem_t* fsd;
extern filetable_t oft[NUM_FD];

local int detail;

typedef struct detail_ {
  char test;
  const char* msg;
} detail_t;

void _fs_assert(const char* title, int ndetails, detail_t* details) {
  int i, pass=0;
  for (i=0; i<ndetails; i++)
    pass += details[i].test;
    
  printf("  %s - %s\n", (pass == ndetails ? "[PASS]" : "<FAIL>"), title);
  if (detail)
    for (i=0; i<ndetails; i++)
      if (!details[i].test)
	printf("         %s\n", details[i].msg);
}

void _maskdiff(char* bitmask, int* set, int* clear) {
  int i=0;
  for (; i<MDEV_NUM_BLOCKS; i++) {
    *set   += ( fs_getmaskbit(i) & (!(bitmask[i / 8] >> (i % 8))));
    *clear += ((!fs_getmaskbit(i)) & ( bitmask[i / 8] >> (i % 8)));
  }
}

/*
 * `fs_create` tests
 *       TEST 1 - Bitmask bit set for inode block
 *       TEST 2 - Directory entry added
 *       TEST 3 - Directory entry points to correct block
 *       TEST 4 - inode size set to 0
 *       TEST 5 - inode id set to block index
 *       TEST 6 - Second file has separate inode block
 *       TEST 7 - Second file directory entry added
 *       TEST 8 - Repeated name
 *       TEST 9 - Too man calls, cannot fit in directory
 */
void test_create_1(char* bitmask, int result) {
  int set=0, clear=0;
  _maskdiff(bitmask, &set, &clear);
  
  _fs_assert("Block marked as occupied", 4, (detail_t[]) {
          { .test = (result == OK), .msg = "Create result is SYSERR, expected OK" },
	  { .test = (set != 0 || result == SYSERR), .msg = "No bit set in mask for inode block" },
	  { .test = (set < 2  || result == SYSERR), .msg = "Too many bits set, expected 1 new block" },
	  { .test = (clear == 0 || result == SYSERR), .msg = "Unexpected cleared bits found" }
    });
}

void test_create_2(char* bitmask, int result) {
  _fs_assert("Directory entry added on create", 3, (detail_t[]) {
	   { .test = (fsd->root_dir.numentries != 0), .msg = "Directory has no new entries" },
	   { .test = (fsd->root_dir.numentries < 2), .msg = "Too many directory entries" },
	   { .test = (strcmp("testfile1", fsd->root_dir.entry[0].name) == 0),
	       .msg = "Directory entry name does not match new file" }
	   });
}

void test_create_3(char* bitmask, int result) {
  int i=2;
  for (; i<MDEV_NUM_BLOCKS; i++)
    if (fs_getmaskbit(i)) break;
  _fs_assert("Directory entry indicates correct inode block", 1, (detail_t[]) {
	   { .test = (fsd->root_dir.entry[0].inode_block == i),
	       .msg = "Entry points to incorrect inode" }
	  });
}

void test_create_4(char* bitmask, int result) {
  inode_t inode;
  bs_read(fsd->root_dir.entry[0].inode_block, 0, &inode, sizeof(inode_t));
  _fs_assert("File inode size set for empty file", 1, (detail_t[]) \
	  { { .test = (inode.size == 0), .msg = "inode size not initialized to 0" } });
}

void test_create_5(char* bitmask, int result) {
  inode_t inode;
  bs_read(fsd->root_dir.entry[0].inode_block, 0, &inode, sizeof(inode_t));
  _fs_assert("File inode id set to block index", 1, (detail_t[]) {
	   { .test = (inode.id == fsd->root_dir.entry[0].inode_block),
	       .msg = "inode id not initialized to correct block index" }
	   });
}

void test_create_6(char* bitmask, int result) {
  int set=0, clear=0;
  _maskdiff(bitmask, &set, &clear);
  
  _fs_assert("Second file block marked as occupied", 5, (detail_t[]) {
	   { .test = (result == OK), .msg = "Create result is SYSERR, expected OK" },
	   { .test = (set != 0 || result == SYSERR), .msg = "No bit set in mask for inode block" },
	   { .test = (set < 2  || result == SYSERR), .msg = "Too many bits set, expected 1 new block" },
	   { .test = (clear == 0 || result == SYSERR), .msg = "Unexpected cleared bits found" },
	   { .test = (fsd->root_dir.entry[0].inode_block != fsd->root_dir.entry[1].inode_block),
	       .msg = "First and second files share the same inode block index" }
	   });
}

void test_create_7(char* bitmask, int result) {
  _fs_assert("Second directory entry added on create", 3,  (detail_t[]) {
	   { .test = (fsd->root_dir.numentries > 1), .msg = "Directory has no new entries" },
	   { .test = (fsd->root_dir.numentries < 3), .msg = "Too many directory entries" },
	   { .test = (strcmp("testfile2", fsd->root_dir.entry[1].name) == 0),
	       .msg = "Directory entry name does not match new file" }
	   });
}
 
 void test_create_8(char* bitmask, int result) {
   _fs_assert("SYSERR returned on repeat filenames", 1, (detail_t[]) \
	  { { .test = (result == SYSERR), .msg = "Got OK" } });
}

void test_create_9(char* bitmask, int result) {
  _fs_assert("SYSERR returned on too many create calls", 1, (detail_t[]) \
	  { { .test = (result == SYSERR), .msg = "Got OK" } });
}

/*
 * `fs_open` tests
 *      TEST 1 - File Table entry has correct name
 *      TEST 2 - File Table entry state set to FSTATE_OPEN
 *      TEST 3 - File Table entry fileptr set to 0
 *      TEST 4 - File Table entry flag field matches provided flags
 *      TEST 5 - File Table entry inode read from block (modify block[0] data for test)
 *      TEST 6 - Function returns File Table index
 *      TEST 7 - File already open
 *      TEST 8 - File doesn't exist
 */
void test_open_1(int result) {
  int i;
  for (i=0; i<NUM_FD; i++)
    if (strcmp(fsd->root_dir.entry[oft[i].de].name, "testfile1") == 0) break;
  
  _fs_assert("Open File Table contains entry with correct name", 1, (detail_t[]) \
	  { { .test = (i < NUM_FD), .msg = "No file found in 'oft' matching 'testfile1'" } });
}

void test_open_2(int result) {
  int i, count=0;
  for (i=0; i<NUM_FD; i++)
    count += (oft[i].state == FSTATE_OPEN);
  _fs_assert("Open File Table entry state set to FSTATE_OPEN", 2, (detail_t[]) {
	   { .test = (count != 0), .msg = "No 'oft' entries found listed as OPEN" },
	   { .test = (count < 2), .msg = "Too many entries marked as OPEN in 'oft'" }
	   });
}

void test_open_3(int result) {
  int i;
  for (i=0; i<NUM_FD; i++)
    if (strcmp(fsd->root_dir.entry[oft[i].de].name, "testfile1") == 0) break;

  if (i >= NUM_FD)
    _fs_assert("Open File Table entry fileptr initialized to 0", 1, (detail_t[]) \
	    { { .test = FAIL, .msg = "No file found with name 'testfile1'" } });
  else
    _fs_assert("Open File Table entry fileptr initialized to 0", 1, (detail_t[]) {
	     { .test = (oft[i].fileptr == 0), .msg = "Initial file 'fileptr' set to non-zero value" }
	     });
}

void test_open_4(int result) {
  int i;
  for (i=0; i<NUM_FD; i++)
    if (strcmp(fsd->root_dir.entry[oft[i].de].name, "testfile1") == 0) break;

  if (i >= NUM_FD)
    _fs_assert("Open File Table entry flag field matches provided flags", 1, (detail_t[]) \
	    { { .test = FAIL, .msg = "No file found with name 'testfile1'" } });
  else
    _fs_assert("Open File Table entry flag field matches provided flags", 1, (detail_t[]) \
	    { { .test = (oft[i].flag == O_RDWR), .msg = "Open file's flag not set" } });
}

void test_open_5(int result) {
  int i;
  for (i=0; i<NUM_FD; i++)
    if (strcmp(fsd->root_dir.entry[oft[i].de].name, "testfile1") == 0) break;

  if (i >= NUM_FD)
    _fs_assert("Open File Table entry inode entry copied from block device", 1, (detail_t[]) \
	    { { .test = FAIL, .msg = "No file found with name 'testfile1'" } });
  else {
    _fs_assert("Open File Table entry inode entry copied from block device", 1, (detail_t[]) {
	     { .test = (oft[i].in.blocks[BLOCK_IDX] == BLOCK_VAL),
		 .msg = ".in differs from block device's values" }
	     });
  }
}

void test_open_6(int result) {
  int i;
  for (i=0; i<NUM_FD; i++)
    if (strcmp(fsd->root_dir.entry[oft[i].de].name, "testfile1") == 0) break;

  if (i >= NUM_FD)
    _fs_assert("'fs_open' function returns correct table index", 1, (detail_t[]) \
	    { { .test = FAIL, .msg = "No file found with name 'testfile1'" } });
  else
    _fs_assert("'fs_open' function returns correct table index", 1, (detail_t[]) \
	    { { .test = (result == i), .msg =  "'fs_open' return value does not match 'oft' index" } });
}

void test_open_7(int result) {
  _fs_assert("File fails to open if already open", 1, (detail_t[])	\
	  { { .test = (result == SYSERR), .msg = "Got OK" } });
}

void test_open_8(int result) {
  _fs_assert("File fails to open if it does not exist", 1, (detail_t[]) \
	  { { .test = (result == SYSERR), .msg = "Got OK" } });
}

/*
 * 'fs_close' tests
 *      TEST 1 - File Table entry is set to FS_CLOSED
 */

void test_close_1(int result) {
  _fs_assert("Open File Table entry set to closed", 1, (detail_t[]) \
	  { { .test = (oft[result].state == FSTATE_CLOSED), .msg = "'oft' lists file as OPEN" } });
}

/*
 * 'fs_write' tests
 */

void test_write_1(int fd, char* testbuf) {
  _fs_assert("Filesize matches first write [1]", 1, (detail_t[]) \
	     { { .test = (oft[fd].in.size == 20),
		    .msg = "Filesize does not match number of written bytes" } });
}

void test_write_2(int fd, char* testbuf) {
  char block[20];
  bs_read(oft[fd].in.blocks[0], 0, &block, 20);
  _fs_assert("File data matches written data [1]", 1, (detail_t[])	\
	     { { .test = (strncmp(block, testbuf, 20) == 0),
		    .msg = "Contents of block does not match written data" } });
}

void test_write_3(int fd, char* testbuf) {
  _fs_assert("Filesize matches first two writes [2]", 1, (detail_t[]) \
	     { { .test = (oft[fd].in.size == 512),
		    .msg = "Filesize does not match number of written bytes" } });
}

void test_write_4(int fd, char* testbuf) {
  char block[492];
  bs_read(oft[fd].in.blocks[0], 20, &block, 492);
  _fs_assert("File data matches written data [2]", 1, (detail_t[]) \
	     { { .test = (strncmp(block, &(testbuf[20]), 20) == 0),
		    .msg = "Contents of block does not match written data" } });
}

void test_write_5(int fd, char* testbuf) {
  _fs_assert("Filesize matches first three writes [3]", 1, (detail_t[]) \
	     { { .test = (oft[fd].in.size == 1024),
		    .msg = "Filesize does not match number of written bytes" } });
}

void test_write_6(int fd, char* testbuf) {
  char block[512];
  bs_read(oft[fd].in.blocks[1], 0, &block, 512);
  _fs_assert("File data matches written data[3]", 1, (detail_t[]) \
	     { { .test = (strncmp(block, &(testbuf[512]), 512) == 0),
		    .msg = "Contents of block does not match written data" } });
}

void test_write_7(int fd, char* testbuf) {
  _fs_assert("Filesize matches all writes [4]", 1, (detail_t[]) \
	     { { .test = (oft[fd].in.size == 1624),
		    .msg = "Filesize does not match number of written bytes" } });
}

void test_write_8(int fd, char* testbuf) {
  char block[600];
  bs_read(oft[fd].in.blocks[2], 0, &block, 512);
  bs_read(oft[fd].in.blocks[3], 0, &(block[512]), 88);
  _fs_assert("File data matches written data[4]", 1, (detail_t[]) \
	     { { .test = (strncmp(block, &(testbuf[1024]), 600) == 0),
		    .msg = "Contents of blocks do not match written data" } });
}

/*
 * 'fs_read' tests
 */
void test_read_1(int fd, char* buf, char* testbuf) {
  _fs_assert("File pointer offset after read [1]", 1, (detail_t[]) \
	  { { .test = (oft[fd].fileptr == 30),
		 .msg = "'fileptr' has not advanced after read operation" } });
}

void test_read_2(int fd, char* buf, char* testbuf) {
  _fs_assert("Content read matches data on disk [1]", 1, (detail_t[]) \
	  { { .test = (strncmp(buf, &(testbuf[0]), 30) == 0),
		 .msg = "Contents of read buffer does not match original data" } });
}

void test_read_3(int fd, char* buf, char* testbuf) {
  _fs_assert("File pointer offset after read [2]", 1, (detail_t[]) \
	  { { .test = (oft[fd].fileptr == 512),
		 .msg = "'fileptr' has not advanced after read operation" } });
}

void test_read_4(int fd, char* buf, char* testbuf) {
  _fs_assert("Content read matches data on disk [2]", 1, (detail_t[]) \
	  { { .test = (strncmp(buf, &(testbuf[30]), 482) == 0),
		 .msg = "Contents of read buffer does not match original data" } });
}

void test_read_5(int fd, char* buf, char* testbuf) {
  _fs_assert("File pointer offset after read [3]", 1, (detail_t[]) \
	  { { .test = (oft[fd].fileptr == 1024),
		 .msg = "'fileptr' has not advanced after read operation" } });
}

void test_read_6(int fd, char* buf, char* testbuf) {
  _fs_assert("Content read matches data on disk [3]", 1, (detail_t[]) \
	  { { .test = (strncmp(buf, &(testbuf[512]), 412) == 0),
		 .msg = "Contents of read buffer does not match original data" } });
}

void test_read_7(int fd, char* buf, char* testbuf) {
  _fs_assert("File pointer offset after read [4]", 1, (detail_t[]) \
	  { { .test = (oft[fd].fileptr == 1624),
		 .msg = "'fileptr' has not advanced after read operation" } });
}

void test_read_8(int fd, char* buf, char* testbuf) {
  _fs_assert("Content read matches data on disk [4]", 1, (detail_t[]) \
	  { { .test = (strncmp(buf, &(testbuf[1024]), 600) == 0),
		 .msg = "Contents of read buffer does not match original data" } });
}


shellcmd xsh_fstest(int nargs, char *args[]) {
  char test_buff[1700];
  char testnum = 0;
  char test_bitmask[fsd->freemasksz];
  int fd=-1, result, i;
  detail = 1;
  
  printf("Running filesystem implementation tests...\n");
  if (nargs == 1) {
    testnum = TEST_OPEN | TEST_CLOSE | TEST_CREATE | TEST_WRITE | TEST_READ;
  }
  else {
    for (i=1; i<nargs; i++)
      if (strcmp(args[i], "create") == 0)
	testnum |= TEST_CREATE;
      else if (strcmp(args[i], "open") == 0)
	testnum |= TEST_OPEN;
      else if (strcmp(args[i], "close") == 0)
	testnum |= TEST_CLOSE;
      else if (strcmp(args[i], "write") == 0)
	testnum |= TEST_WRITE;
      else if (strcmp(args[i], "read") == 0)
	testnum |= TEST_READ;
      else if (strcmp(args[i], "--quiet") == 0)
	detail = 0;
      else
	printf("Unknown TEST name - %s\n", args[i]);
    if (testnum == 0) testnum = TEST_OPEN | TEST_CLOSE | TEST_CREATE | TEST_READ | TEST_WRITE;
  }
  
  for (i=0; i<1700; i++) {
    test_buff[i] = '0' + (i % 74);
  }
  
  if (testnum & TEST_CREATE) {
    if (fsd->root_dir.numentries)
      printf("WARNING - File already created, skipping 'create' tests\n");
    else {
      printf("\n------ Testing 'fs_create' ------\n");
      memcpy(test_bitmask, fsd->freemask, fsd->freemasksz);
      result = fs_create("testfile1");      
      test_create_1(test_bitmask, result);
      test_create_2(test_bitmask, result);
      test_create_3(test_bitmask, result);
      test_create_4(test_bitmask, result);
      test_create_5(test_bitmask, result);

      memcpy(test_bitmask, fsd->freemask, fsd->freemasksz);
      result = fs_create("testfile2");
      test_create_6(test_bitmask, result);
      test_create_7(test_bitmask, result);

      result = fs_create("testfile2");
      test_create_8(test_bitmask, result);

      fs_create("testfile3");
      fs_create("testfile4");
      fs_create("testfile5");
      fs_create("testfile6");
      fs_create("testfile7");
      fs_create("testfile8");
      fs_create("testfile9");
      fs_create("testfile10");
      fs_create("testfile11");
      fs_create("testfile12");
      fs_create("testfile13");
      fs_create("testfile14");
      fs_create("testfile15");
      fs_create("testfile16");
      result = fs_create("testfile17");
      test_create_9(test_bitmask, result);
      
    }
  }

  if (testnum & TEST_OPEN) {
    if (fsd->root_dir.numentries == 0)
      printf("WARNING - No files found in filesystem, skipping 'open' tests\n");
    else {
      int badfd, i;
      printf("\n------ Testing 'fs_open' ------\n");
      for (i=0; i<DIR_SIZE; i++) {
	if (strcmp(fsd->root_dir.entry[i].name, "testfile1") == 0) {
	  inode_t inode;
	  bs_read(fsd->root_dir.entry[i].inode_block, 0, &inode, sizeof(inode));
	  inode.blocks[BLOCK_IDX] = BLOCK_VAL;
	  bs_write(fsd->root_dir.entry[i].inode_block, 0, &inode, sizeof(inode));
	}
      }
      fd = fs_open("testfile1", O_RDWR);
      test_open_1(fd);
      test_open_2(fd);
      test_open_3(fd);
      test_open_4(fd);
      test_open_5(fd);
      test_open_6(fd);

      badfd = fs_open("testfile1", O_RDWR);
      test_open_7(badfd);
      badfd = fs_open("badname", O_RDWR);
      test_open_8(badfd);
    }
  }

  if (testnum & TEST_CLOSE) {
    if (fsd->root_dir.numentries == 0)
      printf("WARNING - No files found in filesystem, skipping 'close' tests\n");
    else {
      printf("\n------ Testing 'fs_close' ------\n");
      if (fd == -1)
	fd = fs_open("testfile1", O_RDWR);
      fs_close(fd);
      test_close_1(fd);
    }
  }

  if (testnum & TEST_WRITE) {
    char* head = test_buff;
    if (fsd->root_dir.numentries == 0)
      printf("\nWARNING - No files found in filesystem, skipping 'write' tests\n");
    else {
      printf("\n------ Testing 'fs_write' ------\n");

      fd = fs_open("testfile1", O_RDWR);
      printf("  ~~Writing small block~~\n");
      fs_write(fd, head, 20);
      head += 20;
      test_write_1(fd, test_buff);
      test_write_2(fd, test_buff);

      printf("  ~~Filling current block~~\n");
      fs_write(fd, head, 492);
      head += 492;
      test_write_3(fd, test_buff);
      test_write_4(fd, test_buff);

      printf("  ~~Writing exactly one block~~\n");
      fs_write(fd, head, 512);
      head += 512;
      test_write_5(fd, test_buff);
      test_write_6(fd, test_buff);

      printf("  ~~Writing more than one block~~\n");
      fs_write(fd, head, 600);
      head += 600;
      test_write_7(fd, test_buff);
      test_write_8(fd, test_buff);

      fs_close(fd);
    }
  }

  if (testnum & TEST_READ) {
    if (fsd->root_dir.numentries == 0)
      printf("\nWARNING - No files found in filesystem, skipping 'read' tests\n");
    else {
      inode_t inode;
      bs_read(fsd->root_dir.entry[0].inode_block, 0, &inode, sizeof(inode_t));
      if (inode.size <= 1500)
	printf("\nWARNING - Cannot run 'read' tests, files do not contain data (run 'write' test)\n");
      else {
	char test1[30];
	char test2[482];
	char test3[512];
	char test4[600];
	printf("\n------ Testing 'fs_read' ------\n");

	fd = fs_open("testfile1", O_RDWR);

	printf("  ~~Reading small block~~\n");
	fs_read(fd, test1, 30);
	test_read_1(fd, test1, test_buff);
	test_read_2(fd, test1, test_buff);

	printf("  ~~Reading rest of block~~\n");
	fs_read(fd, test2, 482);
	test_read_3(fd, test2, test_buff);
	test_read_4(fd, test2, test_buff);

	printf("  ~~Reading full block~~\n");
	fs_read(fd, test3, 512);
	test_read_5(fd, test3, test_buff);
	test_read_6(fd, test3, test_buff);

	printf("  ~~Reading multiple blocks~~\n");
	fs_read(fd, test4, 600);
	test_read_7(fd, test4, test_buff);
	test_read_8(fd, test4, test_buff);

	fs_close(fd);
      }
    }
  }
  
  return 0;
}
