#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

int valid_check_bs(bsd_t bs_id, unsigned int npages)
{
	if (bs_id < 0 || bs_id > MAXMAP || npages <= 0 || npages > NPAGES)// || bsm_tab[bs_id].bs_sem==1)
	{
		//kprintf("\nxxxxxxxxx bs=%d,pages=%d\n",bs_id,npages);
		return 0;
	}
	if(bsm_tab[bs_id].bs_status == BSM_MAPPED && bsm_tab[bs_id].bs_vheap==1)
	{
	//	kprintf("\n ########3 vheap flag = %d\n",bsm_tab[bs_id].bs_vheap);
		return 0;
	}
	return 1;
}
int get_bs(bsd_t bs_id, unsigned int npages) {

  /* requests a new mapping of npages with ID map_id */
	//kprintf("To be implemented! GET_BS %d    %d   %d\n",bsm_tab[bs_id].bs_vheap,bsm_tab[bs_id].bs_status,bs_id);
	STATWORD ps;
	disable(ps);
	if(valid_check_bs(bs_id,npages)==1)
	{
		if (bsm_tab[bs_id].bs_status == BSM_UNMAPPED) {
		//	 kprintf("mapped 111111111 id = %d and npages=%d currpid = %d\n\n",bs_id,npages,currpid);
			restore(ps);
			return npages;
		}
		else 
		{
//			kprintf("mapped 222222222id = %d and npages=%d currpid = %d\n\n",bs_id,bsm_tab[bs_id].bs_npages,currpid);
			bsm_tab[bs_id].bs_status = BSM_MAPPED;
	  		bsm_tab[bs_id].bs_pid = currpid;
			bsm_tab[bs_id].bs_sem = 0;
			bsm_tab[bs_id].bs_vheap=0;
			npages = bsm_tab[bs_id].bs_npages;
			restore(ps);
			return npages;
		}
	}
        else
        {
                restore(ps);
                return SYSERR;
        }

	restore(ps);
//	kprintf("NPAGES = %d\n\n\n",npages);
	return npages;
}


