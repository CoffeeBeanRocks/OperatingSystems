#include <xinu.h>
#include <prodcons.h>
#include <stdlib.h>
#include <run.h>

int n;                 // Definition for global variable 'n'
/* Now global variable n is accessible by all the processes i.e. consume and produce */

sid32 w_mutex;
sid32 r_mutex;
sid32 p_mutex;

shellcmd xsh_prodcons(int nargs, char *args[]) {
	  // Argument verifications and validations
	int count = 2000;    // local varible to hold count
	  //
	  //     // TODO: check args[1], if present assign value to count
	  //
	  //       // create the process producer and consumer and put them in ready queue.
	  //         // Look at the definations of function create and resume in the system folder for reference.
	  //           // TODO: call the 'resume' and 'create' functions to start two processes that execute
	  //             //       your 'producer' and 'consumer' code.
	
	if(nargs == 2)
	{
		if(!atoi(args[1]))
			return(1);
		else
			count = atoi(args[1]);
	}
	
	p_mutex = semcreate(0);
	w_mutex = semcreate(1);
	r_mutex = semcreate(0);

	int pidProd = create(producer, 1024, 20, "producer", 1, count);
	int pidCons = create(consumer, 1024, 20, "consumer", 1, count);

	resume(pidProd);
	resume(pidCons);

	wait(p_mutex);
	signal(run_mutex);
	return(0);	  
}
