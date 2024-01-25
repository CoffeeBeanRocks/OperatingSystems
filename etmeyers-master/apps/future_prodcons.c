#include <xinu.h>
#include <stddef.h>
#include <future.h>

sid32 print_sem;

syscall future_prod(future_t *fut, int value) {
	  int status;
	    wait(print_sem);
	      printf("Producing %d\n", value);
	        signal(print_sem);
		  status = (int) future_set(fut, (void*)&value);
		    if (status < 1) {
			        wait(print_sem);
				    printf("future_set failed\n");
				        signal(print_sem);
					    return SYSERR;
					      }
		      return OK;
}

syscall future_cons(future_t *fut) {
	  int i = NULL;
	    int status;
	      status = (int) future_get(fut, (void*)&i);
	        if (status < 1) {
			    wait(print_sem);
			        printf("future_get failed\n");
				    signal(print_sem);
				        return SYSERR;
					  }
		  wait(print_sem);
		    printf("Consumed %d\n", i);
		      signal(print_sem);
		        return OK;
}

