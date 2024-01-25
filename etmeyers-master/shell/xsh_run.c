#include <xinu.h>
#include <stdlib.h>
#include <shprototypes.h>

//Function Prototypes
#include <prodcons.h>
#include <prodcons_bb.h>
#include <run.h>
#include <future.h>
#include <fs.h>

sid32 run_mutex;

int arr[5];
sid32 write_mutex;
sid32 read_mutex;
int head;
int tail;

shellcmd xsh_run(int nargs, char *args[])
{

	if ((nargs == 1) || (strncmp(args[1], "list", 4) == 0)) {
		printf("futest\n");
		printf("hello\n");
		printf("list\n");
		printf("memtest\n");
		printf("prodcons\n");
		printf("prodcons_bb\n");
		return OK;
	}

	run_mutex = semcreate(0);	
	if(strncmp(args[1], "hello", 5) == 0) {
		/* create a process with the function as an entry point. */
		resume (create((void *) xsh_hello, 4096, 20, "hello", 2, nargs - 1, &(args[1])));
	}

	else if(strncmp(args[1], "prodcons_bb", 11) == 0) {

		if(nargs != 6)
			return(1);

		int numProdProc = atoi(args[2]);
		int numConProc = atoi(args[3]);
		int prodCount = atoi(args[4]);
		int conCount = atoi(args[5]);

		if((numProdProc*prodCount) != (numConProc*conCount))
			return(1);

		write_mutex = semcreate(1);
	       	read_mutex = semcreate(0);
		head = 0;
		tail = 0;

		for(int i=0; i<5; i++)
			arr[i] = 0;

		for(int i=0; i<numProdProc; i++)
		{
			char prodID[13];
			sprintf(prodID, "producer_%d", i);
			resume(create(producer_bb, 4096, 20, prodID, 1, prodCount));  
		}

		for(int i=0; i<numConProc; i++)
		{
			char conID[13];
			sprintf(conID, "consumer_%d", i);
			resume(create(consumer_bb, 4096, 20, conID, 1, conCount));
		}

		for(int i=0; i<numConProc-1; i++)
			wait(run_mutex);
	}	

	else if (strncmp(args[1], "prodcons", 8) == 0) {
	       /* create a process with the function as an entry point. */
	       resume(create((void *) xsh_prodcons, 4096, 20, "prodcons", 2, nargs-1, &(args[1])));
	}

	else if (strncmp(args[1], "futest", 6) == 0)
	{
		resume(create((void *) xsh_futest, 4096, 20, "futest", 2, nargs-1, &(args[1])));
	}

	else if (strncmp(args[1], "memtest", 7) == 0)
	{
		resume(create((void *) xsh_memtest, 4096, 20, "memtest", 2, nargs-1, &(args[1])));
		signal(run_mutex);
	}
	else if (strncmp(args[1], "fstest", 6) == 0)
	{
		//resume(create((void *) xsh_fstest, 4096, 20, "fstest", 2, nargs-1, &(args[1])));
		//signal(run_mutex);
	}

	else {
		printf("Unknown command!\n");
		return(1);
	}

	wait(run_mutex);
	return(0);
}
