/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint()
{
//kprintf("\n\n*********** CHECK FOR PAGE FAULT **********\n\n");
	STATWORD ps;
	disable (ps);
	unsigned long v_addr;
	unsigned int pg_offset, pt_offset,pd_offset,pdbr;
	virt_addr_t *virt_addr;
	int frm,pt,bs,bs_page_offset;
	pt_t *pt_tab;
	pd_t *pd_tab;
	pdbr = proctab[currpid].pdbr;
	v_addr = read_cr2();
	//kprintf("\n %u read cr2\n",v_addr);
	virt_addr = (virt_addr_t *)&v_addr;
	pg_offset = virt_addr->pg_offset;
	pt_offset = virt_addr->pt_offset;
	pd_offset = virt_addr->pd_offset;
	
	pd_tab = pdbr + pd_offset*sizeof(pd_t);
	if(pd_tab->pd_pres == 0) {
	  	if((pt=create_page_table()) == SYSERR)
		{
			kill(currpid);
			restore(ps);
			return SYSERR;
		}
	//	kprintf("frame num = %d\n\n",pt);
		frm_tab[pt].fr_type = FR_TBL;
                frm_tab[pt].fr_status = FRM_MAPPED;
                frm_tab[pt].fr_pid = currpid;
	  	pd_tab->pd_pres = 1;
		pd_tab->pd_write = 1;
		pd_tab->pd_base = pt+FRAME0;
        }
	else
	{
	
	}
	pt_tab = (pt_t*)(pd_tab->pd_base * NBPG+ pt_offset*sizeof(pt_t));
//	kprintf("\n pt = %d\n",pt_tab->pt_pres);
	if(pt_tab->pt_pres == 0)
	{
//kprintf("\n bsm lookup pfint= %d \n",v_addr/4096);
		if(bsm_lookup(currpid,v_addr,&bs,&bs_page_offset) == SYSERR || get_frm(&frm)==SYSERR)
		{
		//	kprintf("\n killingggggggg=%u v_addr=%d bs_vp=%d bs=%d off=%d\n",currpid,v_addr/4096,bsm_tab[bs].bs_vpno,bs,bs_page_offset);
			kill(currpid);
			restore(ps);
			return SYSERR;
		}
/*		if(bsm_tab[bs].bs_vheap==0 && frm_tab[bs_page_offset].fr_refcnt>1)
		{
			pt_tab->pt_base=bsm_tab[bs].multiprocframe[bs_page_offset];
			restore(ps);
			return OK;
		}*/
		
/*		if(bsm_tab[bs].bs_vheap==0 && frm_tab[bs_page_offset].fr_refcnt==1)
		{
			bsm_tab[bs].multiprocframe[bs_page_offset]=FRAME0+frm;
		}*/
//if(bs==11)
		//kprintf("frame_num bs = %d counter =  %d pg num = %d\n",frm,pd_tab->pd_base - FRAME0,pd_tab->pd_base);

                frm_tab[pd_tab->pd_base - FRAME0].fr_refcnt++;
		frm_tab[frm].fr_vpno = v_addr/NBPG;
		//fq_ptr[pd_tab->pd_base-FRAME0].fr_count++;

                frm_tab[frm].fr_pid = currpid;
                frm_tab[frm].fr_type = FR_PAGE;
                frm_tab[frm].fr_status = FRM_MAPPED;
		pt_tab->pt_pres = 1;
		pt_tab->pt_write = 1;
		pt_tab->pt_base = frm+FRAME0;
//kprintf("\n############ frm = %d  bs=%d  bs_pg_off=%d ##########\n",frm,bs,bs_page_offset);
		
		unsigned long *addr=(FRAME0+frm)*NBPG;
		read_bs(addr,bs,bs_page_offset);
		

//	        if(page_replace_policy==3 && replace == 1)

//              	kprintf("\n replace = %d     base = %d,  off=%d\n",replace,pt_tab->pt_base,bs_page_offset);
//                fq_ptr[pt_tab->pt_base-FRAME0].fr_count++;


		//kprintf("\n############ frame inserted = %d     %d##########\n",frm,bs);
		insertframe(frm);
//		updatefrequency();
  	}
//	kprintf("\n pdbr = %u\n",pdbr);	
	if(pt_tab->pt_dirty == 1)
	{
//		kprintf("\n###################### dirty #####################\n");
		write_bs((char *)pt_tab,bs,bs_page_offset);
	}
//kprintf("\n off = %d dirty = %d, acc=%d, write=%d, pwt=%d, pcd=%d,mbz=%d,base=%d\n",pt_tab->pt_base-(FRAME0+bs_page_offset),pt_tab->pt_dirty,pt_tab->pt_acc,pt_tab->pt_write,pt_tab->pt_pwt,pt_tab->pt_pcd,pt_tab->pt_mbz,pt_tab->pt_base);
	extern int page_replace_policy;
	if(page_replace_policy==LFU)
		fq_ptr[pt_tab->pt_base-FRAME0].fr_count++;

	write_cr3(pdbr);
	restore(ps);
	return OK;
}

int create_page_table() 
{
//	kprintf("\n\n ************ CREATE PAGE TABLE ***********\n\n");
	int i,frame_num;
	unsigned int frame_addr;
	pt_t *pt_tab;
	if(get_frm(&frame_num) == SYSERR) {
		return SYSERR;
	}
//kprintf("\n\n ************ CREATE PAGE TABLE %d***********\n\n",frame_num);
	frame_addr = (FRAME0 + frame_num)*NBPG;
	pt_tab = (pt_t*)frame_addr;
	for(i = 0;i < 1024; i++)
	{
		pt_tab[i].pt_pres = 0;
		pt_tab[i].pt_write = 0;
		pt_tab[i].pt_user = 0;
		pt_tab[i].pt_pwt = 0;
		pt_tab[i].pt_pcd = 0;
		pt_tab[i].pt_acc = 0;
		pt_tab[i].pt_dirty = 0;
		pt_tab[i].pt_mbz = 0;
		pt_tab[i].pt_global = 0;
		pt_tab[i].pt_avail = 0;
		pt_tab[i].pt_base = 0;
		if(i<4)
		{
			//pt_tab[i].pt_pres = 1;
        		//pt_tab[i].pt_write = 1;
		}
	}
	//insertframe(frame_num);
	return frame_num;
}
