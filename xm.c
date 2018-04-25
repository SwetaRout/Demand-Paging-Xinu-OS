/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage, bsd_t source, int npages)
{
//  kprintf("xmmap - to be implemented!  XMMAP\n");
  STATWORD ps;
  disable(ps);
//  kprintf("\n virtpage=%d, npages=%d,bsm pages = %d,source=%d, max=%d\n",virtpage,npages,bsm_tab[source].bs_npages,source,MAXMAP);
  if ((virtpage < NBPG) || (npages < 1) || (npages > NPAGES) || (source < 0) || (source > MAXMAP-1))
  {
	restore(ps);
//	kprintf("HERE");
  	return SYSERR;
  }
  /*if(bsm_tab[source].bs_status == BSM_UNMAPPED)
  {
	restore(ps);
	return SYSERR;
  }*/
  if(bsm_tab[source].bs_status == BSM_MAPPED && bsm_tab[source].bs_vheap==1)
  {
//kprintf("\n map=%d, heap=%d\n",bsm_tab[source].bs_status,bsm_tab[source].bs_vheap);
	restore(ps);
        return SYSERR;
  }
  if(bsm_tab[source].bs_status == BSM_MAPPED)
  {
	if(npages>bsm_tab[source].bs_npages)
	{
		restore(ps);
		return SYSERR;
	}
	restore(ps);
	return OK;
 }
  //kprintf("HERE 111111");
  bsm_map(currpid, virtpage, source, npages);
  restore(ps);
  return (OK);
}



/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage)
{
  //kprintf("\n unmap \n");
  STATWORD ps;
  disable(ps);
  int available,pg_num;
  if ((virtpage < NBPG))
  {
	restore(ps);
  	return SYSERR;
  }
  if(bsm_lookup(currpid, (virtpage*NBPG), &available, &pg_num)==SYSERR)
  {
	restore(ps);
	return SYSERR;
  }
  bsm_unmap(currpid, virtpage, 1);
  //unsigned int id = ((proctab[currpid].pdbr)<<12);
//  write_cr3(id);
  restore(ps);
  return (OK);
}
