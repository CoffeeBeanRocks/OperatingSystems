#include <xinu.h>
#include <heap.h>

extern heap_t heaptab[];
void initheap(char* startaddr, uint32 size) {
	  /* This function is called when the heap is constructed */
	  /*   Use as needed to setup the free list               */
          
          if(size == 0) 
		  return SYSERR;

	  struct memblk *memptr = (struct memblk*) startaddr;
	  memptr->mlength = size;
	  memptr->mnext = NULL;

	  heaptab[getpid()].freelist = (char*) memptr;

	  return;
}

void* malloc(uint32 size) {
	  /* Implement memory allocation within the process heap here       */
	  /*   Your implementation MUST NOT use getmem                      */
	  /*   Your implementation SHOULD use explicit in-memory free lists 
	   *   
	   *   Size is in bits
	   *   Char pointers are 4 bits
	   *   struct memblk* are 8 bits = 1 byte                            */

	  intmask mask;
	  mask = disable();

	  size = (uint32) roundmb(size);

	  if(size==0)
		  return (char*) SYSERR;
          	
	  struct memblk *freeptr = (struct memblk*) heaptab[getpid()].freelist;
	  struct memblk *prevptr = NULL;	

	  while(freeptr != NULL)
	  {
		if(freeptr->mlength > size)
		{
			struct memblk *newblk = freeptr;
			
			freeptr = (struct memblk*) ((char*)freeptr + size);
			freeptr->mlength = (uint32) ((char*)freeptr->mlength - size);
			freeptr->mnext = newblk->mnext;

			newblk->mlength = size;

			if(prevptr != NULL)
				prevptr->mnext = freeptr;
			else 
				heaptab[getpid()].freelist = (char*) freeptr;
			
			restore(mask);
			return newblk;	
		}
		else if(freeptr->mlength == size)
		{
			if(prevptr != NULL)
				prevptr->mnext = freeptr->mnext;
			else 
				heaptab[getpid()].freelist = (char*) freeptr->mnext;

			restore(mask);
			return freeptr;

		}
		prevptr = freeptr;
		freeptr = freeptr->mnext;
	  }

	  restore(mask);
	  return (char*)SYSERR;
}

void free(char* block, uint32 size) {
	  /* Implement memory free within process heap here */
	  /*   Your implementation MUST implement coalesing */
	  
          intmask mask;
	  mask = disable();

	  struct memblk *newblk = (struct memblk*) block;

	  //Place free block in correct spot in the free list
	  struct memblk *freeptr = (struct memblk*) heaptab[getpid()].freelist;
	  struct memblk *prevptr = NULL;
	  while(freeptr != NULL && ((char*) freeptr) < (char*) newblk)
	  {
	  	prevptr = freeptr;
		freeptr = freeptr->mnext;
	  }

	  newblk->mnext = freeptr;
	  newblk->mlength = size;

	  if(prevptr == NULL)
		  heaptab[getpid()].freelist = (char*) newblk;
	  else
		  prevptr->mnext = newblk;

	  //coalese
	  if(freeptr->mnext != NULL && (char*)freeptr + freeptr->mlength == (char*)freeptr->mnext)
	  {
		freeptr->mlength = freeptr->mlength + ((struct memblk*)freeptr->mnext)->mlength;
		freeptr->mnext = ((struct memblk*)freeptr->mnext)->mnext;
	  }
	  else if(prevptr != NULL && (char*)prevptr + prevptr->mlength == (char*)freeptr)
	  {
		prevptr->mlength = prevptr->mlength + freeptr->mlength;
		prevptr->mnext = freeptr->mnext;
	  }

	  restore(mask);
	  return;
}
