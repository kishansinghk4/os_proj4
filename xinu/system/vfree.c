#include <xinu.h>

char* vfree(char* ptr, uint32 nbytes)
{
	intmask 	mask;    	/* Interrupt mask		*/
	mask = disable();

    kprintf("\n=================== vfree() ======================\n\n");
    uint32 *given_ptr = (uint32 *)ptr;
	kprintf("vfree()->given ptr to free is -> 0x%08x and number of bytes to free is %d\n", given_ptr, nbytes);


	unsigned int old_cr3 = read_cr3();
	kprintf("vfree()->old written cr3 is -> 0x%08x\n", old_cr3);

	write_cr3(GOLDEN_PD_BASE | 0x18);

	unsigned int new_cr3 = read_cr3();
	kprintf("vfree()->new written cr3 is -> 0x%08x\n", new_cr3);

	unsigned int n_pages;
    if((nbytes % PAGE_SIZE) == 0)
    {
	    n_pages = (double)(nbytes/PAGE_SIZE);
    }
    else
    {
	    n_pages = (double)((nbytes/PAGE_SIZE) + 1);
    }

    kprintf("n_pages -> %d\n", n_pages);
	ALL_HEAP_SIZE = ALL_HEAP_SIZE - n_pages;
    

	if(n_pages > (proctab[currpid].max_v_heap - proctab[currpid].avail_v_heap))
	{	
		kprintf("************************* Trying to free unallocated pages..!! *********************\n");
		restore(mask);
		return (char *)SYSERR;
	}

    uint32 rem_bytes_in_c_page = (PAGE_SIZE - ((uint32)given_ptr & 0x00000fff));

    uint32 c_pd_base_addr = ((uint32)given_ptr & 0xfffff000);
    bool8 flag_once = FALSE;    



    int num_pd_entries_to_invalidate;

    if( (n_pages % (PAGE_SIZE/ENTRY_SIZE) ) == 0)
    {
	    num_pd_entries_to_invalidate = (double)(n_pages / (PAGE_SIZE/ENTRY_SIZE) );
    }
    else
    {
	    num_pd_entries_to_invalidate = (double)( (n_pages / (PAGE_SIZE/ENTRY_SIZE)) + 1 );
    }

	kprintf("num_pd_entries_to_invalidate -> %d\n", num_pd_entries_to_invalidate);


	//kprintf("vmalloc: c_v_add-> 0x%08x, n_v_add-> 0x%08x, start_index-> 0x%08x, end_index-> 0x%08x\n", c_v_add, n_v_add, start_index, end_index);
	
    unsigned int pd_index = c_pd_base_addr >> 22;
    unsigned int pt_index;

	pd_t* pde   = (proctab[currpid].pdbr+ (4*pd_index));


    unsigned int addr;

	do
	{
        kprintf("in do while\n");
	    kprintf("num_pd_entries_to_invalidate inside do while -> %d\n", num_pd_entries_to_invalidate);
        //pde entry should be valid and present bit should be one
        if(pde->pd_valid == 0 || pde->pd_pres == 0)
        {
            kprintf("********************ERROR: vfree()-> pd_valid or pd_present is zero ******************************\n");
        }

        //pd entry is invalid, so allocate a page table and update this entry
        //addr = pde->pd_base << 12;  
        unsigned int c_pt_base_addr = (pde->pd_base << 12);  
		//unsigned int c_pt_base_addr = (struct pt_t *)addr;						// this will be used as page table
		kprintf("pt_base_address -> 0x%08X\n", c_pt_base_addr);

        //invalidate the page table content
        unsigned int pt_index = ((c_pd_base_addr & 0x003ff000) >> 12);    // to extract bit12 to bit21 to index into pt
		kprintf("pt_index -> 0x%08X\n", pt_index);
        pt_t* pte   = c_pt_base_addr + (4*pt_index);
		kprintf("pte -> 0x%08X\n", pte);
        for(int k=pt_index; ( ( k<(PAGE_SIZE/ENTRY_SIZE) ) && ( (int)nbytes > 0) ) ; k++)
        {
	        //kprintf("iteration var k -> %d\n", k);
	        //kprintf("nbytes -> %d\n", nbytes);
            //pte entry should be valid
            if(pte->pt_valid == 0)
            {
                kprintf("**************************ERROR: vfree()-> pt_valid****************************************\n");
            }

        
            if(!flag_once)
            {
                nbytes = nbytes - rem_bytes_in_c_page;
                flag_once = TRUE;    
            }  
            else
            {
                nbytes = nbytes - PAGE_SIZE;
            }
	        
            proctab[currpid].avail_v_heap = proctab[currpid].avail_v_heap + 1;
          
            if( pte->pt_pres == 1 && pte->pt_swap == 1 )
            {
                kprintf("************************** ERROR: vfree()-> pt_swap is 1 and pt_pres is 1 ****************************************\n");
            }
            else if( pte->pt_pres == 1 && pte->pt_swap == 0 )
            {
                remove_page_from_fss( pte->pt_base << 12 );
            }
            else if( pte->pt_pres == 0 && pte->pt_swap == 1 )
            {
                remove_page_from_swap( pte->pt_base << 12 );
            }
            //else if( pte->pt_pres == 0 && pte->pt_swap == 0 )
            //{
            //    //kprintf("**************************ERROR: vfree()-> pt_pres or pt_swap is 0 ****************************************\n");
            //}
         
            //remove_page_from_fss( pte->pt_base << 12 );
             
            pte->pt_pres        =  0; 
            pte->pt_valid       =  0; 
            pte->pt_swap        =  0; 
            pte->pt_write       =  0; 
            pte->pt_pwt       	=  0; 
            pte->pt_pcd       	=  0;
            pte->pt_base        =  0; 
            pte++                   ;
        }
        print_pt(c_pt_base_addr);
     
        pde = pde + 1;
        num_pd_entries_to_invalidate--;

	}while(num_pd_entries_to_invalidate > 0);

	print_pd(proctab[currpid].pdbr);

	write_cr3(old_cr3);

//*****************************************remember to increment fss_used_size**************************************************

/* We are not freeing the page table even though we are invalidating all the page table entries. Since the page talble is still valid, we are not invalidiating the corresponding PDE entry.
 */
	restore(mask);
}
