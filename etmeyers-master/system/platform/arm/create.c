#include <xinu.h>
#include <heap.h>

local   int newpid();

#define roundew(x)  ( (x+3)& ~0x3)

/*------------------------------------------------------------------------
 *  create  -  create a process to start running a procedure
 *------------------------------------------------------------------------
 */
pid32   create(
      void      *procaddr,  /* procedure address        */
      uint32    ssize,      /* stack size in words      */
      pri16     priority,   /* process priority > 0     */
      char      *name,      /* name (for debugging)     */
      uint32    nargs,      /* number of args that follow   */
      ...
    )
{
    /* uint32      savsp; */
    intmask     mask;       /* interrupt mask       */
    pid32       pid;        /* stores new process id    */
    struct  procent *prptr;     /* pointer to proc. table entry */
    int32       i;
    uint32      *a;     /* points to list of args   */
    uint32      *saddr;     /* stack address        */

    mask = disable();
    if (ssize < MINSTK)
        ssize = MINSTK;
    ssize = (uint32) roundew(ssize);
    if (((saddr = (uint32 *)getstk(ssize)) ==
        (uint32 *)SYSERR ) ||
        (pid=newpid()) == SYSERR || priority < 1 ) {
        restore(mask);
        return SYSERR;
    }

    prcount++;
    prptr = &proctab[pid];

    /* initialize process table entry for new process */
    prptr->prstate = PR_SUSP;   /* initial state is suspended   */
    prptr->prprio = priority;
    prptr->prstkbase = (char *)saddr;
    prptr->prstklen = ssize;
    prptr->prname[PNMLEN-1] = NULLCH;
    for (i=0 ; i<PNMLEN-1 && (prptr->prname[i]=name[i])!=NULLCH; i++)
        ;
    prptr->prsem = -1;
    prptr->prparent = (pid32)getpid();
    prptr->prhasmsg = FALSE;

    /* set up initial device descriptors for the shell      */
    prptr->prdesc[0] = CONSOLE; /* stdin  is CONSOLE device */
    prptr->prdesc[1] = CONSOLE; /* stdout is CONSOLE device */
    prptr->prdesc[2] = CONSOLE; /* stderr is CONSOLE device */

    /* Initialize stack as if the process was called        */

    *saddr = STACKMAGIC;

    /* push arguments */
    a = (uint32 *)(&nargs + 1); /* start of args        */
    a += nargs -1;          /* last argument        */
    for ( ; nargs > 4 ; nargs--)    /* machine dependent; copy args */
        *--saddr = *a--;    /* onto created process' stack  */
    *--saddr = (long)procaddr;
    for(i = 11; i >= 4; i--)
        *--saddr = 0;
    for(i = 4; i > 0; i--) {
        if(i <= nargs)
            *--saddr = *a--;
        else
            *--saddr = 0;
    }
    *--saddr = (long)INITRET;   /* push on return address   */
    *--saddr = (long)(ARMV7A_CPSR_SPR | ARMV7A_CPSR_F); /* CPSR, F bit set,     */
                                                      /* Supervisor mode        */

    prptr->prstkptr = (char *)saddr;
    restore(mask);
    return pid;
}

void	userret2(void)
{
  pid32 pid = getpid();
  freeheap(pid);
  kill(pid);			/* Force process to exit */
}


pid32   create2(
      void      *procaddr,  /* procedure address        */
      uint32    ssize,      /* stack size in words      */
      uint32    hsize,      /* heap size in words       */
      pri16     priority,   /* process priority > 0     */
      char      *name,      /* name (for debugging)     */
      uint32    nargs,      /* number of args that follow   */
      ...
    )
{
    /* uint32      savsp; */
    intmask     mask;       /* interrupt mask       */
    pid32       pid;        /* stores new process id    */
    struct  procent *prptr;     /* pointer to proc. table entry */
    int32       i;
    uint32      *a;     /* points to list of args   */
    uint32      *saddr;     /* stack address        */

    mask = disable();
    if (ssize < MINSTK)
      ssize = MINSTK;
    ssize = (uint32) roundew(ssize);
    if (((saddr = (uint32 *)getstk(ssize)) ==
        (uint32 *)SYSERR ) ||
        (pid=newpid()) == SYSERR || priority < 1 ) {
        restore(mask);
        return SYSERR;
    }

    if (getheap(pid, hsize) == SYSERR) {
      restore(mask);
      return SYSERR;
    }

    prcount++;
    prptr = &proctab[pid];

    /* initialize process table entry for new process */
    prptr->prstate = PR_SUSP;   /* initial state is suspended   */
    prptr->prprio = priority;
    prptr->prstkbase = (char *)saddr;
    prptr->prstklen = ssize;
    prptr->prname[PNMLEN-1] = NULLCH;
    for (i=0 ; i<PNMLEN-1 && (prptr->prname[i]=name[i])!=NULLCH; i++)
        ;
    prptr->prsem = -1;
    prptr->prparent = (pid32)getpid();
    prptr->prhasmsg = FALSE;

    /* set up initial device descriptors for the shell      */
    prptr->prdesc[0] = CONSOLE; /* stdin  is CONSOLE device */
    prptr->prdesc[1] = CONSOLE; /* stdout is CONSOLE device */
    prptr->prdesc[2] = CONSOLE; /* stderr is CONSOLE device */

    /* Initialize stack as if the process was called        */

    *saddr = STACKMAGIC;

    /* push arguments */
    a = (uint32 *)(&nargs + 1); /* start of args        */
    a += nargs -1;          /* last argument        */
    for ( ; nargs > 4 ; nargs--)    /* machine dependent; copy args */
        *--saddr = *a--;    /* onto created process' stack  */
    *--saddr = (long)procaddr;
    for(i = 11; i >= 4; i--)
        *--saddr = 0;
    for(i = 4; i > 0; i--) {
        if(i <= nargs)
            *--saddr = *a--;
        else
            *--saddr = 0;
    }
    *--saddr = (long)userret2;   /* push on return address   */
    *--saddr = (long)(ARMV7A_CPSR_SPR | ARMV7A_CPSR_F); /* CPSR, F bit set,     */
                                                      /* Supervisor mode        */

    prptr->prstkptr = (char *)saddr;
    restore(mask);
    return pid;
}


/*------------------------------------------------------------------------
 *  newpid  -  Obtain a new (free) process ID
 *------------------------------------------------------------------------
 */
local   pid32   newpid(void)
{
    uint32  i;          /* iterate through all processes*/
    static  pid32 nextpid = 1;  /* position in table to try or  */
                    /*  one beyond end of table */

    /* check all NPROC slots */

    for (i = 0; i < NPROC; i++) {
        nextpid %= NPROC;   /* wrap around to beginning */
        if (proctab[nextpid].prstate == PR_FREE) {
            return nextpid++;
        } else {
            nextpid++;
        }
    }
    return (pid32) SYSERR;
}
