/* vcreate.c - vcreate */
    
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <paging.h>

/*
static unsigned long esp;
*/

LOCAL	newpid();
/*------------------------------------------------------------------------
 *  create  -  create a process to start running a procedure
 *------------------------------------------------------------------------
 */
SYSCALL vcreate(procaddr,ssize,hsize,priority,name,nargs,args)
	int	*procaddr;		/* procedure address		*/
	int	ssize;			/* stack size in words		*/
	int	hsize;			/* virtual heap size in pages	*/
	int	priority;		/* process priority > 0		*/
	char	*name;			/* name (for debugging)		*/
	int	nargs;			/* number of args that follow	*/
	long	args;			/* arguments (treated like an	*/
					/* array in the code)		*/
{
	//kprintf("To be implemented! VCREATE\n");
	STATWORD ps;
	disable(ps);
	int available_bs;
	int pid = create(procaddr, ssize, priority, name, nargs, args);
	if (pid == SYSERR || get_bsm(&available_bs) == SYSERR || hsize>NPAGES || bsm_map(pid,NVPAGES,available_bs,hsize)==SYSERR) 
	{
//		kprintf("pid = %d, bs = %d ,hsize=%d",pid,available_bs,hsize);
		restore(ps);
		return SYSERR;
	}
//	kprintf("\n bs id = %d\n",available_bs);
	bsm_tab[available_bs].bs_vheap=1;
        struct mblock *b_store = BACKING_STORE_BASE + (available_bs * BACKING_STORE_UNIT_SIZE);
        b_store->mlen = NBPG*hsize;
        b_store->mnext = 0;
//kprintf("\n bs id = %d\n",available_bs);
	proctab[pid].store = available_bs;
	proctab[pid].vhpno = NVPAGES;
	proctab[pid].vhpnpages = hsize;
	proctab[pid].vhead=getmem(sizeof(struct vblock));
	proctab[pid].vhead->vlen=hsize*NBPG;
	proctab[pid].vhead->vaddr=NBPG*NBPG;
	proctab[pid].vtail=getmem(sizeof(struct vblock));
	proctab[pid].vtail->vlen=0;
	proctab[pid].vtail->vaddr=NBPG*(hsize+NBPG);
	proctab[pid].vhead->mnext=proctab[pid].vtail;
	proctab[pid].vtail->mnext=NULL;
//kprintf("\n bs id = %d\n",available_bs);
	restore(ps);
	return pid;
}

/*------------------------------------------------------------------------
 * newpid  --  obtain a new (free) process id
 *------------------------------------------------------------------------
 */
LOCAL	newpid()
{
	int	pid;			/* process id to return		*/
	int	i;

	for (i=0 ; i<NPROC ; i++) {	/* check all NPROC slots	*/
		if ( (pid=nextproc--) <= 0)
			nextproc = NPROC-1;
		if (proctab[pid].pstate == PRFREE)
			return(pid);
	}
	return(SYSERR);
}
