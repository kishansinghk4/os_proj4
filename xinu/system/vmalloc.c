
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
		return (char *)SYSERR;
	}

	unsigned long c_v_add = proctab[currpid].v_add_counter;
	unsigned long n_v_add = proctab[currpid].v_add_counter = proctab[currpid].v_add_counter + (n_pages * 4096);

	unsigned int start_index = 	c_v_add >> 22;
	unsigned int end_index   = 	n_v_add >> 22;

	proctab[currpid].avail_v_heap = proctab[currpid].avail_v_heap - n_pages;

	kprintf("vmalloc: c_v_add-> 0x%08x, n_v_add-> 0x%08x, start_index-> 0x%08x, end_index-> 0x%08x\n", c_v_add, n_v_add, start_index, end_index);

	pd_t* pde   = (proctab[currpid].pdbr+ (4*start_index));
	unsigned long temp_v_add  = c_v_add;

    unsigned int addr, pt_addr;

	do
	{
   		//c_pde->pd_avail       = 1;

        //pd_t* pde = (char *)pd_v;
        if( pde->pd_pres == 0 )
        {
            //pd entry is invalid, so allocate a page table and update this entry
			if(is_pdpt_full())
			{
				return SYSERR;
			}
    		addr = get_free_page_pdpt();        
			pt_addr = (struct pt_t *)addr;						// this will be used as page table
			kprintf("new PT address -> 0x%08X\n", pt_addr);

            reset_pt(pt_addr);

            //initialize the page table content
            void* pt_v  = (unsigned int *)pt_addr;
            kprintf("temp_v_add = %08x\n", temp_v_add); 
            int pt_strt = (temp_v_add & 0x003ff000) >> 12;    // to extract bit12 to bit21 to index into pt
            kprintf("pt_strt = %d\n", pt_strt);
            pt_v = pt_v + (4*pt_strt); 
            for(int k=pt_strt; ( ( k<(PAGE_SIZE/ENTRY_SIZE) ) && (n_pages != 0) ) ; k++)
            {
                pt_t* pte = (char *)pt_v;
                pte->pt_pres        =  0; 
                pte->pt_valid       =  1; 
                pte->pt_swap        =  0; 
                pte->pt_write       =  1; 
                pte->pt_pwt       	=  1; 
                pte->pt_pcd       	=  1; 
                //pte->pt_base      =  j;
                pt_v                += 4;
				//j					=  j+1;
                n_pages             =  n_pages - 1;
                temp_v_add          =  temp_v_add + 4096;
            }

        	print_pt(pt_addr);
            pt_addr             =  pt_addr >> 12;
            pde->pd_pres        =  1; 
            pde->pd_valid       =  1; 
            pde->pd_write       =  1; 
            pde->pd_pwt       	=  1; 
            pde->pd_pcd       	=  1; 
            pde->pd_base        =  pt_addr;
		    kprintf("new PT address -> 0x%08X is written in PD entry ->  0x%08X\n", pt_addr, pde);

		    kprintf("%dth enrty in pd->0x%08x is valoidated, enrty address->0x%08x \n", start_index, proctab[currpid].pdbr,  pde); 

        }
        else
        {
            //pd entry is invalid, so allocate a page table and update this entry
            addr = pde->pd_base << 12;  
			pt_addr = (struct pt_t *)addr;						// this will be used as page table
			kprintf("Old PT address -> 0x%08X\n", pt_addr);

            //reset_pt(pt_addr);

            //initialize the page table content
            void* pt_v = (unsigned int *)pt_addr;
            kprintf("temp_v_add = %08x\n", temp_v_add); 
            int pt_strt = (temp_v_add & 0x003ff000) >> 12;    // to extract bit12 to bit21 to index into pt
            kprintf("pt_strt = %d\n", pt_strt); 
            pt_v = pt_v + (4*pt_strt); 
            for(int k=pt_strt; ( ( k<(PAGE_SIZE/ENTRY_SIZE) ) && (n_pages != 0) ) ; k++)
            {
                pt_t* pte = (char *)pt_v;
                pte->pt_pres        =  0; 
                pte->pt_valid       =  1; 
                pte->pt_swap        =  0; 
                pte->pt_write       =  1; 
                pte->pt_pwt       	=  1; 
                pte->pt_pcd       	=  1; 
                //pte->pt_base      =  j;
                pt_v                += 4;
				//j					=  j+1;
                n_pages             =  n_pages - 1;
                temp_v_add          =  temp_v_add + 4096;
            }
        	print_pt(pt_addr);
        }

        //pd_v += 4;

        pde = pde + 1;
		start_index++;

        kprintf("The temp_virtual_address = 0x%08x\n", temp_v_add);

	}while(start_index<=end_index);


	print_pd(proctab[currpid].pdbr);

	write_cr3(old_cr3);
	
	restore(mask);
	return (char*) c_v_add; 
}

