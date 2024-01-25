#include <xinu.h>
#include <heap.h>

heap_t heaptab[NPROC];

syscall getheap(pid32 pid, uint32 hsize) {
	  intmask mask;
	    mask = disable();
	      
	      if (hsize < MINHEAP) {
		          restore(mask);
			      return SYSERR;
			        }
	        hsize = (uint32) roundmb(hsize);
		  char* heap = getmem(hsize);
		    if (heap == (char*)SYSERR) {
			        restore(mask);
				    return SYSERR;
				      }
		      heaptab[pid].size = hsize;
		        heaptab[pid].startaddr = heap;
			  heaptab[pid].freelist = heap;
			    initheap(heap, hsize);
			      
			      restore(mask);
			        return OK;
}

syscall freeheap(pid32 pid) {
	  intmask mask;
	    mask = disable();

	      if (freemem(heaptab[pid].startaddr, heaptab[pid].size) == SYSERR) {
		          restore(mask);
			      return SYSERR;
			        }
	        
	        restore(mask);
		  return OK;
}
