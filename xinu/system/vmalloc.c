
#include <xinu.h>

char* vmalloc(uint32 nbytes)
{
	
	intmask 	mask;    	/* Interrupt mask		*/
	mask = disable();

	unsigned int old_cr3 = read_cr3();
	kprintf("old written cr3 is -> 0x%08x\n", old_cr3);

	write_cr3(GOLDEN_PD_BASE | 0x18);

	unsigned int n_pages = (double)(nbytes/4096);
	kprintf("vmalloc: nbytes-> %d, n_pages->%d\n", nbytes, n_pages);

	if(n_pages > proctab[currpid].avail_v_heap)
	{	
		kprintf("heap size is exhausted..!\n");
		restore(mask);
		return SYSERR;
	}

	unsigned long c_v_add = proctab[currpid].v_add_counter;
	unsigned long n_v_add = proctab[currpid].v_add_counter = proctab[currpid].v_add_counter + (n_pages * 4096);

	unsigned int start_index = 	c_v_add >> 22;
	unsigned int end_index   = 	n_v_add >> 22;

	proctab[currpid].avail_v_heap = proctab[currpid].avail_v_heap - n_pages;

	kprintf("vmalloc: c_v_add-> 0x%08x, n_v_add-> 0x%08x, start_index-> 0x%08x, end_index-> 0x%08x\n", c_v_add, n_v_add, start_index, end_index);

	pd_t* c_pde = (proctab[currpid].pdbr+ (4*start_index));
		
	do
	{
   		c_pde->pd_avail       = 1;
		kprintf("%dth enrty in pd->0x%08x is valoidated, enrty address->0x%08x \n", start_index, proctab[currpid].pdbr,  c_pde); 
		c_pde = c_pde + 1;
		start_index++; 
	}while(start_index<end_index);
	
	print_pd(proctab[currpid].pdbr);
	write_cr3(old_cr3);
	
	restore(mask);
	return (char*) c_v_add; 
}

