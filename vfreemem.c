/* vfreemem.c - vfreemem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>
extern struct pentry proctab[];
/*------------------------------------------------------------------------
 *  vfreemem  --  free a virtual memory block, returning it to vmemlist
 *------------------------------------------------------------------------
 */
SYSCALL	vfreemem(block, size)
	struct	vblock	*block;
	unsigned size;
{
	//kprintf("To be implemented! VFREEMEM\n");
	STATWORD ps;
	struct	vblock *p,*q,*tmp;
	int bs;
	disable(ps);
//	kprintf("\n block = %d , %d\n",block,NVPAGES*NBPG);
	//if (block->vaddr < NVPAGES*NBPG ||(block->vaddr>(((proctab[currpid].vhpnpages)*NBPG)+(NBPG*NBPG))) ||size == 0) 
	if (size==0 || block>(((proctab[currpid].vhpnpages)*NBPG)+(NBPG*NBPG)) || (block)<( NBPG*NBPG) )
	{
		kprintf("\n########## return errr\n");
		restore(ps);
		return SYSERR;
	}
	size = (unsigned)roundmb(size);
	q=&proctab[currpid].vhead;
//	kprintf("\n size=%d\n",size);
	p=q->mnext;
//	block->vlen = size;
//	block->mnext = p;	
	while(p->vaddr <= block) {
		q=p;
		p=p->mnext;
	}
	if(p->vaddr-size==q->vaddr)
	{
		q->vlen =p->vlen +size;
	        q->mnext=p->mnext;
	}
	else
	{
		q->vlen=size;
		tmp=getmem(sizeof(struct vblock));
		tmp->vaddr = q->vaddr +size;
		tmp->vlen = 0;
                tmp->mnext =p;
		q->mnext =tmp;
	}
//	get_bsm(&bs);
	//bsm_tab[bs].bs_vheap=0;
//	kprintf("\n bs=%d \n",bs);
	restore(ps);
	return(OK);
}
