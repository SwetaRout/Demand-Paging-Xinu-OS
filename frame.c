/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>
#define MAX_INT 99999999
//int replace;
/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_frm()
{
//  kprintf("To be implemented! init frm\n");
  STATWORD ps;
  disable (ps);
  int i = 0;
    
  while(i < NFRAMES) 
  {
    frm_tab[i].fr_status = FRM_UNMAPPED;
    frm_tab[i].fr_pid = -1;
    frm_tab[i].fr_vpno = -1;
    frm_tab[i].fr_refcnt = 0;
    frm_tab[i].fr_type = -1;
    frm_tab[i].fr_dirty = -1;
    frm_tab[i].fr_count=0;
    i++;
  }
  restore(ps);
  return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{
//  kprintf("To be implemented! GET FRAME\n");
  STATWORD ps;
  disable (ps);
  int i;
  int flag = 0;
  int fr;
  struct fifo_queue* frm_avail;
  for (i = 0; i < NFRAMES; i++) 
  {
    if (frm_tab[i].fr_status == FRM_UNMAPPED) 
    {
//      kprintf("UNMAPPED FRAME = %d\n\n",i+1024);
      *avail = i;
      frm_tab[i].fr_status == FRM_MAPPED;
      restore(ps);
      return OK;
    }
  }
// kprintf("\n$$$$$$$$$$$$$HERE MAPPED\n*****************\n");
extern int page_replace_policy;
//if(page_replace_policy==LFU)
if(page_replace_policy==SC)
	fr=replaceSC();
else
	fr=replaceLFU();
  if(fr>-1)
  {
	free_frm(fr);
//kprintf("\n free %d\n",fr);
	*avail=fr;
	restore(ps);
	return OK;
  }
// kprintf("\####### No frame available.. return sys err########\n");
  restore (ps);
  return SYSERR;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{

//	kprintf("To be implemented! FREE FRAME\n");
  	STATWORD ps;
	disable(ps);
	unsigned long v_addr,pdbr;	
	unsigned int pt_offset,pd_offset;
	pd_t *pd_entry; 
	pt_t *pt_entry;
	if(frm_tab[i].fr_type == FR_TBL || frm_tab[i].fr_type == FR_DIR || frm_tab[i].fr_type == FR_PAGE)
	{
		if(i<frm_tab[i].fr_type)
		{
			restore(ps);
			return SYSERR;
		}
	}
	if(frm_tab[i].fr_type == FR_PAGE)
	{
	//	kprintf("\n WRITE BS \n");
		write_bs((i+FRAME0)*NBPG,proctab[frm_tab[i].fr_pid].store,frm_tab[i].fr_vpno-proctab[frm_tab[i].fr_pid].vhpno);	
		v_addr = frm_tab[i].fr_vpno;
		pdbr = proctab[frm_tab[i].fr_pid].pdbr;
		pd_offset = v_addr / 1024;
		pt_offset = v_addr&1023;
		pd_entry = pdbr + (pd_offset*sizeof(pd_t));
		pt_entry = (pd_entry->pd_base*NBPG) + (pt_offset*sizeof(pt_t));
		
		pt_entry->pt_pres = 0;
	//	kprintf("\n..................PRINT pt_acc value = %d\n",pt_entry->pt_acc);
		frm_tab[pd_entry->pd_base-FRAME0].fr_refcnt--;
		if(frm_tab[pd_entry->pd_base-FRAME0].fr_refcnt == 0)
		{
			frm_tab[pd_entry->pd_base-FRAME0].fr_pid = -1;
			frm_tab[pd_entry->pd_base-FRAME0].fr_status = FRM_UNMAPPED;
			frm_tab[pd_entry->pd_base-FRAME0].fr_type = FR_PAGE;
			frm_tab[pd_entry->pd_base-FRAME0].fr_vpno = NVPAGES;
			fq_ptr[pd_entry->pd_base-FRAME0].fr_count=0;
			pd_entry->pd_pres = 0;
		}
	//	kprintf("\n END WRITE \n");
	}
	restore(ps);
	return OK;
}
int replaceSC()
{
	//kprintf("\n\n...........GET FREE FRAME\n");
	STATWORD ps;
	disable(ps);
	int frm=-1,current=0,next = -1,prev = -1,pt,pg_offset;
	virt_addr_t *virt_addr; 
	pd_t *pd_entry;
	pt_t *pt_entry; 
	extern int page_replace_policy;
	extern int fq_ptr_head,fq_ptr_tail;
//	kprintf("\n #######3 queue ends head = %d and tail = %d\n",fq_ptr_head,fq_ptr_tail);

	while(current!=-1)
	{
		virt_addr = (virt_addr_t*)&(frm_tab[current].fr_vpno);
		pd_entry =  proctab[currpid].pdbr+virt_addr->pd_offset * sizeof(pd_t);
		pt_entry = (pt_t*)(pd_entry->pd_base*NBPG + virt_addr->pt_offset*sizeof(pt_t));
		frm = fq_ptr_head;
		if(pt_entry->pt_acc==1)
		{
			pt_entry->pt_acc=0;
		}
		else
		{
			if(prev!=-1)
			{
				fq_ptr[prev].next = fq_ptr[current].next;
			//	fq_ptr[current].next = -1;
				fq_ptr[fq_ptr_tail].next=frm;
                        	fq_ptr_tail=frm;
                	        fq_ptr[frm].next=-1;
		

				kprintf("\n SC replace frm = %d\n",frm+FRAME0);
// kprintf("\n #######3replaceeeee queue ends head = %d and tail = %d\n",fq_ptr_head,fq_ptr_tail)	;			
				restore(ps);
				return(frm);
			}
			fq_ptr_head = fq_ptr[frm].next;
//kprintf("\n before head= %d and current = %d and current.next=%d\n",fq_ptr_head,current,fq_ptr[current].next);
//			fq_ptr[current].next = -1;
                        fq_ptr[fq_ptr_tail].next=frm;
                        fq_ptr_tail=frm;
			fq_ptr[frm].next=-1;

			kprintf("\n SC replace frame = %d\n",frm+FRAME0);
//	kprintf("\n #######3replaceeeee queue ends head = %d and tail = %d\n",fq_ptr_head,fq_ptr_tail);
			restore(ps);
			return(frm);
		}
		prev = current;
		current = fq_ptr[current].next;
	}
	fq_ptr_head = fq_ptr[current].next;
//	fq_ptr[current].next = -1;
        fq_ptr[fq_ptr_tail].next=frm;
	fq_ptr_tail=frm;
	fq_ptr[frm].next=-1;

	return frm;
}
void insertframe(int fr_num)
{
	//kprintf("INSERT FRAME %d\n",fr_num);
	STATWORD ps;
	disable(ps);
	extern int fq_ptr_head,fq_ptr_tail;
	if(fq_ptr_head == -1)
	{
		fq_ptr_head=fr_num;
		restore(ps);
		return OK;
	}
	int current=fq_ptr_head,next=fq_ptr[fq_ptr_head].next;
//kprintf("\n ######## queue starts \n");

	while(next!=-1)
	{
//	kprintf("%d->",current);
		current=next;
		next = fq_ptr[next].next;
	}
	fq_ptr[current].next=fr_num;
	fq_ptr[fr_num].next=-1;
	fq_ptr_tail=fr_num;
//	fq_ptr[fr_num].fr_count++;
	//kprintf("%d->%d->%d",fq_ptr[current].fr_no,fq_ptr[current].next,fq_ptr[fr_num].next);
//kprintf("\n###### %d queue end  q head = %d and q tail = %d#########\n",fr_num,fq_ptr_head,fq_ptr_tail);
	restore(ps);
	return OK;
}
int replaceLFU()
{
	STATWORD ps;
	disable(ps);
//kprintf("\n ###################request replace \n");	
	int frame_number=0,curr_frm=0,nxt_frm = -1,prev_frm = -1,page_table_new,page_offset,min_prev=-1;
	unsigned int virt_pt_offset,virt_pd_offset;
	unsigned long pdbr,v_addr; 
	extern int fq_ptr_head,fq_ptr_tail;
	virt_addr_t *virt_addr; 
	frame_number=fq_ptr_head;
//kprintf("\n q head = %d, q tail = %d\n",fq_ptr_head,fq_ptr_tail);
	for(curr_frm=fq_ptr_head;curr_frm != -1;prev_frm = curr_frm,curr_frm = fq_ptr[curr_frm].next){
//kprintf("\n inside loop cur cont = %d and vpno=%d\n",fq_ptr[curr_frm].fr_count,frm_tab[curr_frm].fr_vpno);
//kprintf("%d(%d,%d)->",curr_frm,fq_ptr[curr_frm].fr_count,frm_tab[curr_frm].fr_vpno);
		v_addr = frm_tab[curr_frm].fr_vpno;

		virt_addr = (virt_addr_t*)&v_addr;
		virt_pt_offset = virt_addr->pt_offset;
		virt_pd_offset = virt_addr->pd_offset;

		pdbr = proctab[currpid].pdbr;
		if(fq_ptr[curr_frm].fr_count<fq_ptr[frame_number].fr_count)
		{
//				kprintf("\n curr frm = %d its count = %d min=%d\n",curr_frm,fq_ptr[curr_frm].fr_count,fq_ptr[frame_number].fr_count);
			min_prev=prev_frm;
			frame_number = curr_frm;
		}
		else if(fq_ptr[curr_frm].fr_count == fq_ptr[frame_number].fr_count)
		{
			if(frm_tab[curr_frm].fr_vpno > frm_tab[frame_number].fr_vpno)
			{
				min_prev=prev_frm;
				frame_number = curr_frm;
			}
		}
	}
	kprintf("\n LFU replaced frame = %d \n",frame_number+FRAME0);
	if(min_prev!=-1)
        {
        	fq_ptr[min_prev].next = fq_ptr[frame_number].next;
                fq_ptr[frame_number].next = -1;
	}
	else
	{
		fq_ptr_head  = fq_ptr[frame_number].next;
		fq_ptr[frame_number].next = -1;
	}
//
//	fq_ptr[frame_number].fr_count++;
//	fq_ptr[pt_entry->pt_base-(FRAME0+bs_page_offset)].fr_count++;	
	restore(ps);
	return frame_number;
}

