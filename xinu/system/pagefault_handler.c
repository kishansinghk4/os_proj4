
#include <xinu.h>

void pagefault_handler()
{
	kprintf("\n------------- in pf handler ------------------\n");
	unsigned int old_cr3 = read_cr3();
	//kprintf("old written cr3 is -> 0x%08x\n", old_cr3);

	write_cr3(GOLDEN_PD_BASE | 0x18);

	//kprintf(" cr3 in hardware -> 0x%08x\n", new_cr3);

	uint32 faulty_addr = read_cr2();
	kprintf("The faulty addr is -> 0x%08x\n", faulty_addr);
    kprintf("fss used pages is->%d\n", fss_dyn_size);
	//getting current process's pdbr value
	uint32 crr_pdbr = proctab[currpid].pdbr;
	//kprintf("The curr pdbr is -> 0x%08x\n", crr_pdbr);

	uint32 pd_index = faulty_addr >> 22;
	//kprintf("The pd_index is -> 0x%08x\n", pd_index);
	uint32 pt_index = (faulty_addr<<10) >> 22;
	//kprintf("The pt_index -> 0x%08x\n", pt_index);

	// pde will point to the pd entry which was looked up by the hardware 
	//	which resulted in the page fault
	
	pd_t *pde = (struct pd_t *) (crr_pdbr + (4*pd_index));
	//kprintf("The pde is -> 0x%08x and its content -> 0x%08x\n", pde, *pde);

	// at this point the pde entry must have been validated in vmalloc()
	
	if(pde->pd_valid == 0 || pde->pd_pres == 0)
	{
		kprintf("*****************************ERROR***********************-> pagefault_handler: pd_valid or pd_pres is not 1\n");
	}
	
	uint32 c_pt_base = pde->pd_base;
	//kprintf("The c_pt_base is -> 0x%08x\n", c_pt_base);
	uint32 c_pt_base_addr = c_pt_base << 12;
	//kprintf("The c_pt_base_addr is -> 0x%08x\n", c_pt_base_addr);

	
	// pte will point to the pt entry which was looked up by the hardware 
	//	which resulted in the page fault
	pt_t *pte = c_pt_base_addr + (4*pt_index);

	// at this point the pte entry must have been validated in vmalloc()
	// if pt_pres bit is 1, page fault shouldn't have been raised	

	if(pte->pt_valid == 0 || pte->pt_pres == 1)
	{
		kprintf("*****************************ERROR***********************-> pagefault_handler: pt_valid is 0 or pt_pres is 1\n");
	}

	if(pte->pt_swap == 0) 	//it's a first time access
	{
		uint32 t_addr;
		if(!is_fss_full())
		{
			t_addr 		=  get_free_page_fss();
		}
		else
		{
			kprintf("*****************************ERROR***********************: The code for FSS eviction is to be written\n");
		}
		pte->pt_base  		=  (t_addr >> 12);
		pte->pt_pres  		=  1;
        pte->pt_write       =  1; 
        pte->pt_pwt       	=  1; 
        pte->pt_pcd       	=  1;

		//print_pt(c_pt_base_addr);

	}
	else
	{
		kprintf("*****************************ERROR***********************: The code for Swap is to be written\n");
	}

	kprintf("address 0x%08x resolved\n", faulty_addr);
	write_cr3(old_cr3);
}
