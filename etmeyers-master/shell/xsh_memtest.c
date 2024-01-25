#include <xinu.h>
#include <string.h>
#include <stdio.h>
#include <heap.h>

extern heap_t heaptab[];

void _assert(const char* title, const char valid) {
	  kprintf("%s: %s\n", title, (valid ? "PASS" : "FAIL"));
}

char overlap(char* a, char* b, uint32 sz_a, uint32 sz_b) {
	  return (a != (char*)SYSERR && b != (char*)SYSERR) &&	\
		           ((b >= a && b < a+sz_a) || (a >= b && a < b+sz_b));
}

char contains(char* a, char* b, uint32 sz_a, uint32 sz_b) {
	  return (a != (char*)SYSERR && b != (char*)SYSERR) &&	\
		           (b >= a && b + sz_b <= a + sz_a);
}

/* TEST 1: Malloc once */ void test1(void) {
	  pid32 pid = getpid();
	    uint32 size = heaptab[pid].size;
	      char* block = heaptab[pid].startaddr;

	        char* t1_1 = malloc(20);
		  _assert("Malloc once",
				  	  contains(block, t1_1, size, 20));
}

/* TEST 2: Malloc twice */ void test2(void) {
	  pid32 pid = getpid();
	    uint32 size = heaptab[pid].size;
	      char* block = heaptab[pid].startaddr;

	        char* t2_1 = malloc(20);
		  char* t2_2 = malloc(20);
		    _assert("Malloc twice", \
				    	  !overlap(t2_1, t2_2, 20, 20) && \
					  	  contains(block, t2_1, size, 20) && contains(block, t2_2, size, 20));
}

/* TEST 3: Malloc all in one block */ void test3(void) {
	  pid32 pid = getpid();
	    uint32 size = heaptab[pid].size;
	      char* block = heaptab[pid].startaddr;

	        char* t3_1 = malloc(1024);
		  _assert("Malloc all-at-once", \
				  	  contains(block, t3_1, size, 1024));
}

/* TEST 4: Malloc all in small blocks */ void test4(void) {
	  pid32 pid = getpid();
	    uint32 size = heaptab[pid].size;
	      char* block = heaptab[pid].startaddr;

	        char* t4_1 = malloc(256);
		  char* t4_2 = malloc(256);
		    char* t4_3 = malloc(512);
		      _assert("Malloc all-in-pieces", \
				      	  !overlap(t4_1, t4_2, 256, 256) && !overlap(t4_1, t4_3, 256, 512) && !overlap(t4_2, t4_3, 256, 512) && \
					  	  contains(block, t4_1, size, 256) && contains(block, t4_2, size, 256) && \
						  	  contains(block, t4_3, size, 512));
}

/* TEST 5: Overallocate in one block */ void test5(void) {

	  char* t5_1 = malloc(1025);
	    _assert("Overallocation in one request", \
			    	  t5_1 == (char*)SYSERR);
}

/* TEST 6: Overallocate in many blocks */ void test6(void) {

	  char* t6_1 = malloc(256);
	    char* t6_2 = malloc(512);
	      char* t6_3 = malloc(512);
	        _assert("Overallocation after multiple requests", \
					  !overlap(t6_1, t6_2, 256, 512) && t6_3 == (char*)SYSERR);
}

/* TEST 7: Free and reuse */ void test7(void) {
	  pid32 pid = getpid();
	    uint32 size = heaptab[pid].size;
	      char* block = heaptab[pid].startaddr;

	        char* t7_1 = malloc(256);
		  char* t7_2 = malloc(256);
		    char* t7_3 = malloc(512);
		      free(t7_2, 256);
		        char* t7_4 = malloc(20);
			  _assert("Free and reuse block", \
					  	  !overlap(t7_1, t7_3, 256, 512) && !overlap(t7_1, t7_4, 256, 20) && !overlap(t7_3, t7_4, 512, 20) && \
						  	  contains(block, t7_1, size, 256) && contains(block, t7_3, size, 512) && \
							  	  contains(block, t7_4, size, 20));
}

/* TEST 8: Free multiple and coalesce */ void test8(void) {
	  pid32 pid = getpid();
	    uint32 size = heaptab[pid].size;
	      char* block = heaptab[pid].startaddr;

	        char* t8_1 = malloc(256);
		  char* t8_2 = malloc(256);
		    char* t8_3 = malloc(256);
		      free(t8_1, 256);
		        free(t8_2, 256);
			  char* t8_4 = malloc(300);
			    _assert("Free multiple and combine", \
					    	  !overlap(t8_3, t8_4, 256, 300) && \
						  	  contains(block, t8_3, size, 256) && contains(block, t8_4, size, 300));
}

shellcmd xsh_memtest(int nargs, char *args[]) {
	  resume(create2(test1, 1024, 1024, 20, "Test 1", 0));
	    resume(create2(test2, 1024, 1024, 20, "Test 2", 0));
	      resume(create2(test3, 1024, 1024, 20, "Test 3", 0));
	        resume(create2(test4, 1024, 1024, 20, "Test 4", 0));
		  resume(create2(test5, 1024, 1024, 20, "Test 5", 0));
		    resume(create2(test6, 1024, 1024, 20, "Test 6", 0));
		      resume(create2(test7, 1024, 1024, 20, "Test 7", 0));
		        resume(create2(test8, 1024, 1024, 20, "Test 8", 0));

			  return 0;
}
