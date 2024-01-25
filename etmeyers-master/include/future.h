#include<xinu.h>

typedef enum {
	  FUTURE_EMPTY,
	    FUTURE_WAITING,
	      FUTURE_READY
} future_state_t;

typedef enum {
	  FUTURE_EXCLUSIVE,
	    FUTURE_SHARED,
	      FUTURE_QUEUE
} future_mode_t;

typedef struct future_t {
	  void *data;
	    uint size;
	      future_state_t state;
	        future_mode_t mode;
		  pid32 pid;
} future_t;

future_t* future_alloc(future_mode_t mode, uint size, uint nelems);
syscall future_free(future_t*);
syscall future_get(future_t*, void*);
syscall future_set(future_t*, void*);

syscall future_prod(future_t*, int val);
syscall future_cons(future_t*);

syscall xsh_futest(int nargs, char* args[]);

extern struct future_t futtab[];
