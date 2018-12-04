
#include <xinu.h>

void pagefault_handler()
{
	//kprintf("\n------------- in pf handler ------------------\n");
	unsigned int old_cr3 = read_cr3();
	//kprintf("old written cr3 is -> 0x%08x\n", old_cr3);

	write_cr3(GOLDEN_PD_BASE | 0x18);

	//kprintf(" cr3 in hardware -> 0x%08x\n", new_cr3);

	uint32 faulty_addr = read_cr2();
	//kprintf("\n------------- 0x%08x ------------------\n", faulty_addr);
	//kprintf("The faulty addr is -> 0x%08x\n", faulty_addr);
    //kprintf("fss used pages is->%d\n", fss_used_size);
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

    bool8 found;
	uint32 fss_phy_page_base_addr;
	if(pte->pt_swap == 0) 	//it's a first time access
	{
		if(!is_fss_full())
		{
			fss_phy_page_base_addr 		=  get_free_page_fss();
            found = FALSE;
            for(int i = 0;i < MAX_FSS_SIZE; i++)
            {
                if(fss_free_track[i].pg_base_addr == fss_phy_page_base_addr)
                {
                    fss_free_track[i].pte_addr = pte; 
                    fss_free_track[i].pg_owner = currpid;
                    found = TRUE;
                    break; 
                }
            }


            if(!found)
            {
                kprintf("******************************** page_handler()-> page was not found in cam search -> swap=0 ->ffs_full=0 *********************************\n ");
            }
		}
		else
		{
            fss_phy_page_base_addr      = evict_random_page_fss();
            
            pt_t* evicted_pte;

            found = FALSE;
            for(int i = 0;i < MAX_FSS_SIZE; i++)
            {
                if(fss_free_track[i].pg_base_addr == fss_phy_page_base_addr)
                {
                    evicted_pte = fss_free_track[i].pte_addr;
                    found = TRUE;
                    break;
                }
            }

            if(!found)
            {
                kprintf("******************************** page_handler()-> page was not found in cam search -> else-> swap=0 ->ffs_full=1 *********************************\n ");
            }
            
            if(evicted_pte->pt_swap == 1 || evicted_pte->pt_pres == 0)
            {
                kprintf("******************************** page_handler()-> e_swap is %d and e_pt_pres is %d-> (c_swap=0 && ffs_full=1) *********************************\n", evicted_pte->pt_swap, evicted_pte->pt_pres);
            }

            //get a free page from swap space
            uint32 swap_phy_page_addr = get_free_page_swap();

            memcpy((char *)swap_phy_page_addr, (char *)fss_phy_page_base_addr, PAGE_SIZE );

            //updaing pte entry corresponsing to the evicted page from fss
            evicted_pte->pt_pres  = 0;
            evicted_pte->pt_swap     = 1;
            evicted_pte->pt_base  = (swap_phy_page_addr >> 12);

            found = FALSE;
            for(int i = 0;i < MAX_FSS_SIZE; i++)
            {
                if(fss_free_track[i].pg_base_addr == fss_phy_page_base_addr)
                {
                    fss_free_track[i].pte_addr = pte;
                    fss_free_track[i].pg_owner = currpid;
                    found = TRUE;
                    break;
                }
            }

            if(!found)
            {
                kprintf("******************************** page_handler()-> page was not found in cam search -> swap=0 ->ffs_full=1 *********************************\n ");
            }
		}

        //check if the below code still holds
		pte->pt_base  		=  (fss_phy_page_base_addr >> 12);
		pte->pt_pres  		=  1;
        pte->pt_write       =  1; 
        pte->pt_pwt       	=  1; 
        pte->pt_pcd       	=  1;
        pte->pt_swap       	=  0;

        //print_fss_fr_trk_struct();
		//print_pt(c_pt_base_addr);
	}
	else
	{
        if(!is_fss_full())
        {
			fss_phy_page_base_addr 		=  get_free_page_fss();
            found = FALSE;
            for(int i = 0;i < MAX_FSS_SIZE; i++)
            {
                if(fss_free_track[i].pg_base_addr == fss_phy_page_base_addr)
                {
                    fss_free_track[i].pte_addr = pte; 
                    fss_free_track[i].pg_owner = currpid;
                    found = TRUE;
                    break; 
                }
            }

             
            if(!found)
            {
                kprintf("******************************** page_handler()-> page was not found in cam search -> swap=1 ->ffs_full=0 *********************************\n ");
            }

            memcpy((char *)fss_phy_page_base_addr, (char *)(pte->pt_base << 12), PAGE_SIZE );

            //adding page back to the swap space
            remove_page_from_swap(pte->pt_base << 12);
            
        }
        else
        {
            fss_phy_page_base_addr      = evict_random_page_fss();
            
            pt_t* evicted_pte;

            found = FALSE;
            for(int i = 0;i < MAX_FSS_SIZE; i++)
            {
                if(fss_free_track[i].pg_base_addr == fss_phy_page_base_addr)
                {
                    evicted_pte = fss_free_track[i].pte_addr;
                    found = TRUE;
                    break;
                }
            }

            if(!found)
            {
                kprintf("******************************** page_handler()-> page was not found in cam search -> swap=1 ->ffs_full=1 *********************************\n ");
            }
            
            if(evicted_pte->pt_swap == 1 || evicted_pte->pt_pres == 0)
            {
                kprintf("******************************** page_handler()-> e_swap is %d and e_pt_pres is %d-> (c_swap=1 && ffs_full=1) *********************************\n", evicted_pte->pt_swap, evicted_pte->pt_pres);
            }

            //get a free page from swap space
            uint32 swap_phy_page_addr = (pte->pt_base)<<12;

            char t_arr[PAGE_SIZE];

            char *temp = t_arr;
            memcpy(temp, (char *)fss_phy_page_base_addr, PAGE_SIZE );
            memcpy((char *)fss_phy_page_base_addr,(char *)swap_phy_page_addr, PAGE_SIZE );
            memcpy((char *)swap_phy_page_addr, temp, PAGE_SIZE );

            //updaing pte entry corresponsing to the evicted page from fss
            evicted_pte->pt_pres        = 0;
            evicted_pte->pt_swap        = 1;
            evicted_pte->pt_base        = (swap_phy_page_addr >> 12);

            found = FALSE;
            for(int i = 0;i < MAX_FSS_SIZE; i++)
            {
                if(fss_free_track[i].pg_base_addr == fss_phy_page_base_addr)
                {
                    fss_free_track[i].pte_addr = pte;
                    fss_free_track[i].pg_owner = currpid;
                    found = TRUE;
                    break;
                }
            }


            if(!found)
            {
                kprintf("******************************** page_handler()-> page was not found in cam search -> -> swap=1 ->ffs_full=1 *********************************\n ");
            }
		}

        //check if the below code still holds
		pte->pt_base  		=  (fss_phy_page_base_addr >> 12);
		pte->pt_pres  		=  1;
        pte->pt_write       =  1; 
        pte->pt_pwt       	=  1; 
        pte->pt_pcd       	=  1;
        pte->pt_swap       	=  0;

        //print_fss_fr_trk_struct();
	    //print_pt(c_pt_base_addr);
	}

	//kprintf("address 0x%08x resolved\n", faulty_addr);
	write_cr3(old_cr3);
}
