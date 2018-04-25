/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <stdio.h>
#include <paging.h>
/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
SYSCALL kill(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	int	dev;
//	int next,current,i;
	disable(ps);

//kprintf("\n here \n");
	releaseres(pid);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);
	
	send(pptr->pnxtkin, pid);

	freestk(pptr->pbase, pptr->pstklen);
	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;	/* suicide */
			resched();

	case PRWAIT:	semaph[pptr->psem].semcnt++;

	case PRREADY:	dequeue(pid);
			pptr->pstate = PRFREE;
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	default:	pptr->pstate = PRFREE;
	}
	restore(ps);
	return OK;
}
void releaseres(int pid)
{
	STATWORD ps;
        //struct  pentry  *pptr;          /* points to proc. table for pid*/
//        int     dev;
        int next,current,i,bs,off;
	disable(ps);
	next=-1;
	current=-1;
	i=0;
	//kprintf("\n here kill \n");
	free_bsm(proctab[currpid].store);
	extern int fq_ptr_head;
	while(i < NFRAMES)
	{
		if(frm_tab[i].fr_pid == pid)
		{
		 	if(frm_tab[i].fr_type == FR_PAGE)
			{
				fq_ptr[i].fr_count=0;
				if(fq_ptr_head==i)
				{
					fq_ptr_head=fq_ptr[fq_ptr_head].next;
					fq_ptr[i].next=-1;
				}
				else
				{
					next=fq_ptr[fq_ptr_head].next;
					current=fq_ptr_head;
					while(next!=i)
					{
						current=next;
						next=fq_ptr[next].next;
					}
					fq_ptr[current].next = fq_ptr[next].next;
					fq_ptr[next].next = -1;
				}
			}
			frm_tab[i].fr_dirty = 0;
		  	frm_tab[i].fr_pid = -1;
		  	frm_tab[i].fr_refcnt = 0;
		  	frm_tab[i].fr_status = FRM_UNMAPPED;
		  	frm_tab[i].fr_type = FR_PAGE;
		  	frm_tab[i].fr_vpno = NVPAGES;
		}
		i++;
	}	
	//free_frm(proctab[pid].pdbr-FRAME0);
	restore(ps);
	return(OK);
}
