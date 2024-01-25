#include <future.h>

future_t* future_alloc(future_mode_t mode, uint size, uint nelem) {
	intmask mask;
	mask = disable();

	future_t *ftrptr = (future_t*) getmem(sizeof(future_t));
	ftrptr->state = FUTURE_EMPTY;

	ftrptr->data = getmem(size);
	ftrptr->size = size;

	restore(mask);
	return ftrptr;
}

syscall future_free(future_t* f) {
	if(f->state == FUTURE_WAITING)
		kill(f->pid);

	f->state = FUTURE_EMPTY;

	freemem((char*)f->data, f->size);

	return freemem((char*)f, f->size);
}

syscall future_get(future_t* f, void* out)
{
	intmask mask;
	mask = disable();

	if(f->state == FUTURE_WAITING)
	{
		restore(mask);
		return SYSERR;
	}
	else if(f->state == FUTURE_EMPTY)
	{
		f->state = FUTURE_WAITING;
		f->pid = getpid();
		suspend(f->pid);
	}

	f->state = FUTURE_EMPTY;
	memcpy(out, f->data, f->size);

	restore(mask);
	return OK;
}

syscall future_set(future_t* f, void* in)
{
	intmask mask;
	mask = disable();

	if(f->state == FUTURE_READY)
	{
		restore(mask);
		return SYSERR;
	}

	memcpy(f->data, in, sizeof(in));

	if(f->state == FUTURE_WAITING)
	{		
		resume(f->pid);	
	}
	
	f->state = FUTURE_READY;

	restore(mask);
	return OK;
}


