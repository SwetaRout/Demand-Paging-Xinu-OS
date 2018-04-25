#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

SYSCALL release_bs(bsd_t bs_id) {

  /* release the backing store with ID bs_id */
//	kprintf("To be implemented!RELEASE_BS pid =%d\n", bsm_tab[bs_id].bs_pid);
	STATWORD ps;
	disable(ps);
/*if(bsm_tab[bs_id].bs_pid==currpid && bsm_tab[bs_id].bs_status==BSM_MAPPED)
{
	bsm_tab[bs_id].bs_status = BSM_UNMAPPED;
	bsm_tab[bs_id].bs_pid = -1;
  	bsm_tab[bs_id].bs_npages = 0;
	bsm_tab[bs_id].bs_vpno = NVPAGES;
	bsm_tab[bs_id].bs_sem = 0;
	bsm_tab[bs_id].bs_vheap=0;
	free_bsm(bs_id);
}*/
	restore(ps);
   	return OK;
}

