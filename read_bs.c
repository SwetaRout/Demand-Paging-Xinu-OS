#include <conf.h>
#include <kernel.h>
#include <mark.h>
#include <bufpool.h>
#include <proc.h>
#include <paging.h>

SYSCALL read_bs(char *dst, bsd_t bs_id, int page) {

  /* fetch page page from map map_id
     and write beginning at dst.
  */
//kprintf("\n here \n");
   void * phy_addr = BACKING_STORE_BASE + bs_id*BACKING_STORE_UNIT_SIZE + page*NBPG;
//kprintf("\n here ppppp %s %u\n",dst,(unsigned long)phy_addr);
   bcopy(phy_addr, (void*)dst, NBPG);
//kprintf("\n here qqqq\n");
}


