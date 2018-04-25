/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>
bs_map_t bsm_tab[NUMBS];
fr_map_t frm_tab[NFRAMES];
struct fifo_queue fq_ptr[NFRAMES];
/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_bsm()
{
kprintf("INIT BSM\n");
	STATWORD ps;
	disable(ps);
	int i = 0;
	while(i < NUMBS) 
	{
		bsm_tab[i].bs_status = BSM_UNMAPPED;
	  	bsm_tab[i].bs_pid = -1;
	  	bsm_tab[i].bs_vpno = NVPAGES;
	  	bsm_tab[i].bs_npages = 0;
		bsm_tab[i].bs_sem = 0;
		bsm_tab[i].bs_vheap=0;
		bsm_tab[i].bs_id = i;
		i++;
	}
	restore(ps);
	return (OK);
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail)
{
//kprintf("GET BSM\n");
	STATWORD ps;
	disable(ps);
	int i = 0;
	while(i < NUMBS) 
	{
		if (bsm_tab[i].bs_status == BSM_UNMAPPED)
		{
			*avail = i;
			restore(ps);
			return (OK);
		}  	
		i++;
	}
	restore(ps);
	return (SYSERR);
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{
//kprintf("FREE BSM = %d\n",i);
	STATWORD ps;
	disable(ps);
	bsm_tab[i].bs_status = BSM_UNMAPPED;
  	bsm_tab[i].bs_pid = -1;
  	bsm_tab[i].bs_npages = 0;
	bsm_tab[i].bs_vpno = NVPAGES;
	bsm_tab[i].bs_vheap =0;
	bsm_tab[i].bs_sem = 0;
 	restore(ps);
	return (OK);
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, unsigned long vaddr, int* store, int* pageth)
{
	STATWORD ps;
	disable(ps);
	//kprintf("bsm lookup %u\n\n",vaddr/NBPG);
	unsigned int pg_offset = vaddr/NBPG;
	int i=0;
	while(i < NUMBS) {
//	kprintf("\n i= %d pgoff = %d and vpno=%d max=%d currpid = %d and pid=%d\n",i,pg_offset,bsm_tab[i].bs_vpno, bsm_tab[i].bs_vpno + bsm_tab[i].bs_npages,currpid,bsm_tab[i].bs_pid);
		if (currpid == bsm_tab[i].bs_pid) {
//			kprintf("\n reached here \n");
			if ((pg_offset >= bsm_tab[i].bs_vpno) && (pg_offset <= bsm_tab[i].bs_vpno + bsm_tab[i].bs_npages)) 
			{
//				kprintf("\n i=%d \n",i);
//if(i==11)
//kprintf("\n i= %d pgoff = %d  vp =%d and vpno=%d off=%d\n",i,vaddr,pg_offset,bsm_tab[i].bs_vpno,pg_offset - bsm_tab[i].bs_vpno);	
				*pageth = pg_offset - bsm_tab[i].bs_vpno;
				*store = i;
				restore(ps);
				return OK;
			}
		}
		i++;
	}
	restore(ps);
	return SYSERR;
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages)
{
//kprintf("MAPPING\n");
//kprintf("\n vpno = %d, npages = %d, source = %d map = %d",vpno,npages,source,bsm_tab[source].bs_status);
	//if((npages < 1) || (npages > NPAGES) || (source < 0) || (source > MAXMAP) || bsm_tab[source].bs_status == BSM_UNMAPPED)
//		return SYSERR;
//	if(bsm_tab[source].bs_status == BSM_MAPPED && bsm_tab[source].bs_vheap==1)
//		return SYSERR;
	STATWORD ps;
	disable(ps);
//kprintf("pid. =   %d  && source bs = %d   vpno=%d\n",pid,source,vpno);
	bsm_tab[source].bs_status = BSM_MAPPED;
  	bsm_tab[source].bs_pid = pid;
  	bsm_tab[source].bs_vpno = vpno;
  	bsm_tab[source].bs_npages = npages;
	proctab[currpid].vhpno = vpno;
	bsm_tab[source].bs_sem=1;
	bsm_tab[source].bs_vheap=0;
	proctab[currpid].store = source;
	restore(ps);
	return (OK);
}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag)
{

	STATWORD ps;
	disable(ps);
	int i = 0,source,page_off;
	unsigned long vaddr = vpno*NBPG;
//kprintf("\n\n bsm_unmap pid = %d, curr pid= %d \n\n",pid,currpid);
	for(i=0;i < NFRAMES;i++){
		if(frm_tab[i].fr_pid == pid && frm_tab[i].fr_type == FR_PAGE)
  		{
			bsm_lookup(pid,vaddr,&source,&page_off);
//kprintf("\n source =%d \n");
//			if(source >=10)
//			kprintf("\n before =%d\n",source);
			write_bs( (i+NFRAMES)*NBPG, source, page_off);
// if(source >=10)
  //                      kprintf("\n after = %d \n",source);
  		}
	}

	bsm_tab[source].bs_status = BSM_UNMAPPED;
  	bsm_tab[source].bs_pid = -1;
  	bsm_tab[source].bs_vpno = NVPAGES;
  	bsm_tab[source].bs_npages = 0;
	bsm_tab[source].bs_sem = 0;
	bsm_tab[source].bs_vheap=0;
	restore(ps);
	return (OK);
}
	

