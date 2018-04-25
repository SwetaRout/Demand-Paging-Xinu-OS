/* vgetmem.c - vgetmem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 * vgetmem  --  allocate virtual heap storage, returning lowest WORD address
 *------------------------------------------------------------------------
 */
WORD	*vgetmem(nbytes)
	unsigned nbytes;
{

	//////kprintf("To be implemented! VGETMEM\n");

	STATWORD ps;
        struct  vblock  *p, *q, *leftover;

        disable(ps);
        if (nbytes==0)// || proctab[currpid].vmemlist->mnext == (struct vblock *) NULL)
	{
  //    	kprintf("\n return 1\n");
	        restore(ps);
                return( (WORD *)SYSERR);
        }
        nbytes = (unsigned int) roundmb(nbytes);
        for (q= proctab[currpid].vhead,p=q->mnext ;p != (struct vblock *) NULL ;q=p,p=p->mnext)
	{
//		kprintf("\n vlen=%d  nbytes=%d\n", q->vlen,nbytes);
                if ( q->vlen >= nbytes ) {
                        leftover = getmem(sizeof(struct vblock*));
			q->mnext=leftover;
			leftover->vaddr = (q->vaddr + nbytes);
			leftover->vlen = (q->vlen - nbytes);
			leftover->mnext = p;
//			kprintf("\n returns q not error  %08x\n",q->vaddr);                      
			q->vlen=0; 
                        restore(ps);
                        return( (WORD *)q->vaddr );
                }
	}
//	kprintf("\n return 2\n");
        restore(ps);
   	return (SYSERR);
}


