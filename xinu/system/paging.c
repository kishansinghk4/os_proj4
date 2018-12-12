#include <xinu.h>

/* Memory bounds */

void    *min_pdpt;       /* Minimum address of PDPT  */
void    *max_pdpt;       /* Maximum level of PDPT */

void    *min_fss;       /* Minimum address of fss  */
void    *max_fss;       /* Maximum level of fss */

void    *min_swap;       /* Minimum address of swap  */
void    *max_swap;       /* Maximum level of swap */


extern free_track_t    pdpt_free_track[MAX_PT_SIZE];
extern free_track_t    fss_free_track[MAX_FSS_SIZE];
extern free_track_t    swap_free_track[MAX_SWAP_SIZE];



/*------------------------------------------------------
  PDPT region initialization
-------------------------------------------------------*/
void	pdpt_init(void) {

        min_pdpt = (void *)(maxheap+1);
        max_pdpt = (void *) ( min_pdpt + (MAX_PT_SIZE*PAGE_SIZE) - 1 );

        return;
}

/*-----------------   Code to initialize FSS region   ------------------*/

void	fss_init(void) {

        min_fss = (void *)(max_pdpt+1);
        max_fss = (void *) ( min_fss + (MAX_FSS_SIZE*PAGE_SIZE) - 1 );

        return;
}


/*-----------------   Code to initialize SWAP region   ------------------*/

void	swap_init(void) {

        min_swap = (void *)(max_fss+1);
        max_swap = (void *) ( min_swap + (MAX_SWAP_SIZE*PAGE_SIZE) - 1 );

        return;
}


/*--------------------------------------------------------------
  Initialize free pages tracking structures for pdpt,fss,swap
--------------------------------------------------------------*/
void initialize_fr_trk_structs()
{
    unsigned int addr;
	addr = (unsigned int)min_pdpt;
    for(int i=0; i<MAX_PT_SIZE; i++)
    {
        pdpt_free_track[i].avail            = TRUE;
        pdpt_free_track[i].pg_base_addr     = addr;
        addr = addr+ PAGE_SIZE;
    }

    
/*-----------------   Code to initialize FSS region   ------------------*/

    addr = (unsigned int)min_fss;
    for(int i=0; i<MAX_FSS_SIZE; i++)
    {
        fss_free_track[i].avail            = TRUE;
        fss_free_track[i].pg_base_addr     = addr;
        fss_free_track[i].pte_addr         = 0;
        fss_free_track[i].pg_owner         = 0;
        addr = addr+ PAGE_SIZE;
    }

/*-----------------   Code to initialize SWAP region   ------------------*/

    addr = (unsigned int)min_swap;
    for(int i=0; i<MAX_SWAP_SIZE; i++)
    {
        swap_free_track[i].avail            = TRUE;
        swap_free_track[i].pg_base_addr     = addr;
        addr = addr+ PAGE_SIZE;
    }

}


/*-----------------   Code to print pdpt free tracking structure ---------*/
void print_pdpt_fr_trk_struct()
{
	intmask	mask;			/* Saved interrupt mask		*/
	mask = disable();
    
	kprintf("\n\nPDPT Free List Status:\n");
	kprintf("Index      Available      Page_Base_Address(hex)\n");
	kprintf("-----      ---------      ----------------------\n");
    for(int i=0; i<MAX_PT_SIZE; i++)
    {
        kprintf("%04d           %d              0x%08x\n",i, pdpt_free_track[i].avail, pdpt_free_track[i].pg_base_addr);
    }

	restore(mask);
}

/*-----------------   Code to print fss free tracking structure ---------*/


void print_fss_fr_trk_struct()
{
	intmask	mask;			/* Saved interrupt mask		*/
	mask = disable();
    
	kprintf("\n\nfss Free List Status:\n");
	kprintf("Index      Available      Page_Base_Address(hex)      owner    pte_addr    pt_base       valid     present     swap\n");
	kprintf("-----      ---------      ----------------------      -----    --------    -------       -----     -------     ----\n");
    for(int i=0; i<MAX_FSS_SIZE; i++)
    {
        kprintf("%04d           %d              0x%08x                  %d      0x%08x          0x%08x    %d      %d      %d\n",i, fss_free_track[i].avail, fss_free_track[i].pg_base_addr, fss_free_track[i].pg_owner, fss_free_track[i].pte_addr, ((pt_t *)fss_free_track[i].pte_addr)-> pt_base, ((pt_t *)fss_free_track[i].pte_addr)->pt_valid, ((pt_t *)fss_free_track[i].pte_addr)->pt_pres, ((pt_t *)fss_free_track[i].pte_addr)->pt_swap );
    }

	restore(mask);
}

/*-----------------   Code to print swap free tracking structure ---------*/



/*------------------  Code to return free page base address from requesting region --------*/

unsigned int get_free_page_pdpt()
{
	intmask	mask;			/* Saved interrupt mask		*/
	mask = disable();

    int i=0;
    while(pdpt_free_track[i].avail == FALSE)
    {
        i++;
    }
    pdpt_free_track[i].avail = FALSE;
	pdpt_used_size++;

	restore(mask);
    return pdpt_free_track[i].pg_base_addr;
}


unsigned int get_free_page_fss()
{
	intmask	mask;			/* Saved interrupt mask		*/
	mask = disable();

    int i=0;
    while(fss_free_track[i].avail == FALSE)
    {
        i++;
    }
    fss_free_track[i].avail = FALSE;
	fss_used_size++;
    if(fss_used_size<0 || fss_used_size> MAX_FSS_SIZE)
    {
        //kprintf("************************* remove_page_from_fss() -> invalid fss used size -> %d ****************************\n", fss_used_size);
    }
    //kprintf("fss used pages in get_free_page_fss ->%d, MAX_FSS_SIZE->%d\n", fss_used_size, MAX_FSS_SIZE);

    //kprintf("one page is removed from fss -> fss_used_size->%d\n", fss_used_size);
	restore(mask);
    return fss_free_track[i].pg_base_addr;
}


unsigned int get_free_page_swap()
{
	intmask	mask;			/* Saved interrupt mask		*/
	mask = disable();

    int i=0;
    while(swap_free_track[i].avail == FALSE)
    {
        i++;
    }
    swap_free_track[i].avail = FALSE;
	swap_used_size++;

	restore(mask);
    return swap_free_track[i].pg_base_addr;
}


bool8 is_pdpt_full()
{
	intmask	mask;			/* Saved interrupt mask		*/
	mask = disable();

	if(pdpt_used_size >= MAX_PT_SIZE)
	{

	    restore(mask);
		return TRUE;
	}
	else
	{
	    restore(mask);
		return FALSE;
	}
}



bool8 is_fss_full()
{
	intmask	mask;			/* Saved interrupt mask		*/
	mask = disable();

	if(fss_used_size >= MAX_FSS_SIZE)
	{
        //kprintf("fss used pages in is_full ->%d, MAX_FSS_SIZE->%d\n", fss_used_size, MAX_FSS_SIZE);
	    restore(mask);
		return TRUE;
	}
	else
	{
	    restore(mask);
		return FALSE;
	}
}


bool8 is_swap_full()
{
	intmask	mask;			/* Saved interrupt mask		*/
	mask = disable();

	if(swap_used_size >= MAX_SWAP_SIZE)
	{
	    restore(mask);
		return TRUE;
	}
	else
	{
	    restore(mask);
		return FALSE;
	}
}


void remove_page_from_fss(uint32 addr)
{
	intmask	mask;			/* Saved interrupt mask		*/
	mask = disable();
    
    if(fss_used_size <= 0)
    {
        //kprintf("************************* remove_page_from_fss()-> FSS is empty, nothing can be removed ****************************\n");
    }
    else
    {
        for(unsigned int i =0;i<MAX_FSS_SIZE;i++)
        {
            if(fss_free_track[i].pg_base_addr == addr)
            {
                fss_free_track[i].avail = 1;
                fss_used_size--;
                if(fss_used_size<0 || fss_used_size> MAX_FSS_SIZE)
                {
                    //kprintf("************************* remove_page_from_fss() -> invalid fss used size -> %d ****************************\n", fss_used_size);
                }
                //kprintf("one page is added in fss-> fss_used_size->%d\n", fss_used_size);
                return;
            }
        } 
        //kprintf("************************* remove_page_from_fss()-> address not found ****************************\n");
    }

	restore(mask);    
}



void remove_page_from_pdpt(uint32 addr)
{
	intmask	mask;			/* Saved interrupt mask		*/
	mask = disable();
    
    if(pdpt_used_size <= 0)
    {
        //kprintf("************************* remove_page_from_pdpt()-> pdpt is empty, nothing can be removed ****************************\n");
    }
    else
    {
        for(unsigned int i =0;i<MAX_PT_SIZE;i++)
        {
            if(pdpt_free_track[i].pg_base_addr == addr)
            {
                pdpt_free_track[i].avail = 1;
                pdpt_used_size--;
                if(pdpt_used_size<0 || pdpt_used_size>= MAX_PT_SIZE)
                {
                    //kprintf("************************* remove_page_from_pdpt() -> invalid pdpt used size ****************************\n");
                }
                //kprintf("one page is added in pdpt free list\n");
                return;
            }
        } 
        //kprintf("************************* remove_page_from_pdpt()-> address not found ****************************\n");
    }

	restore(mask);    
}

void remove_page_from_swap(uint32 addr)
{
	intmask	mask;			/* Saved interrupt mask		*/
	mask = disable();

    //kprintf("\nremove_page_from_swap function has been called\n");   
    if(swap_used_size <= 0)
    {
        //kprintf("************************* remove_page_from_swap()-> swap is empty, nothing can be removed ****************************\n");
    }
    else
    {
        for(unsigned int i =0;i<MAX_SWAP_SIZE;i++)
        {
            if(swap_free_track[i].pg_base_addr == addr)
            {
                swap_free_track[i].avail = 1;
                swap_used_size--;
                if(swap_used_size<0 || swap_used_size>= MAX_SWAP_SIZE)
                {
                    //kprintf("************************* remove_page_from_swap() -> invalid swap used size ****************************\n");
                }
                return;
            }
        } 
        //kprintf("************************* remove_page_from_swap()-> address not found ****************************\n");
    }

	restore(mask);    
}


/*----------------    Code to reset page directory  -------------------*/
void reset_pd(unsigned int addr)
{
	intmask	mask;			/* Saved interrupt mask		*/
	mask = disable();
    for(int i=0; i<(PAGE_SIZE/ENTRY_SIZE); i++)
    {
        //void* v = (unsigned int *)addr;

        //pd_t* pde = (char *)v;
        pd_t* pde = addr;
 
        pde->pd_pres        = 0; 
        pde->pd_write       = 0; 
        pde->pd_user        = 0; 	
        pde->pd_pwt	        = 0; 
        pde->pd_pcd	        = 0; 
        pde->pd_acc	        = 0; 
        pde->pd_mbz	        = 0; 
        pde->pd_fmb	        = 0; 
        pde->pd_global      = 0; 
        pde->pd_avail       = 0; 
        pde->pd_base        = 0;

        //kprintf("pde entry addr = %08x and its content = 0x%08x\n", pde, *pde);

        addr += 4;
        //if(i == 10) break;
    }

	restore(mask);    
}



/*----------------    Code to reset page table  -------------------*/
void reset_pt(unsigned int addr)
{
	intmask	mask;			/* Saved interrupt mask		*/
	mask = disable();
    for(int i=0; i<(PAGE_SIZE/ENTRY_SIZE); i++)
    {
        //void* v = (unsigned int *)addr;

        //pd_t* pte = (char *)v;
        pt_t* pte = (char *)addr;


  		pte->pt_pres	= 0;
  		pte->pt_write 	= 0;
  		pte->pt_user	= 0;
  		pte->pt_pwt		= 0;
  		pte->pt_pcd		= 0;
  		pte->pt_acc		= 0;
  		pte->pt_dirty 	= 0;
  		pte->pt_mbz		= 0;
  		pte->pt_global	= 0;
  		pte->pt_avail 	= 0;
  		pte->pt_base	= 0;


        //kprintf("pde entry addr = %08x and its content = 0x%08x\n", pde, *pde);

        addr += 4;
        //if(i == 10) break;
    }

    restore(mask);
}



void print_pd(unsigned int addr)
{
	intmask	mask;			/* Saved interrupt mask		*/
	mask = disable();
	kprintf("\nPD at address = 0x%08x\n", addr);
	kprintf("Index     PDE_Address        pd_base      valid      present\n");
	kprintf("-----      ---------         -------      -----      -------\n");
    for(int i=0; i<(PAGE_SIZE/ENTRY_SIZE); i++)
    {
        void* v = (unsigned int *)addr;
        pd_t* pde = (char *)v;
        kprintf("%04d      0x%08x       0x%08x        %d           %d\n",i, pde, pde->pd_base, pde->pd_valid, pde->pd_pres);
        addr += 4;
        if(i==20) break;
    }

    restore(mask);
}



void print_pt(unsigned int addr)
{
	intmask	mask;			/* Saved interrupt mask		*/
	mask = disable();
	kprintf("\nPT at address = 0x%08x\n", addr);
	kprintf("Index      pte_Address     pt_base       valid     present     swap\n");
	kprintf("-----      ---------       -------       -----     -------     ----\n");
    for(int i=0; i<(PAGE_SIZE/ENTRY_SIZE); i++)
    {
        void* pt_v = (unsigned int *)addr;
        pt_t* pte = (char *)pt_v;
        kprintf("%04d       0x%08x       0x%08x      %d         %d       %d\n",i, pte, pte->pt_base, pte->pt_valid, pte->pt_pres, pte->pt_swap);
        addr += 4;
        if(i==11) break;
    }

    restore(mask);
}


/*----------------    Code to populate initial mapping tables  ------------*/
/* we have one page directory and 15 page tables to map 32mb(xinu)+1mb(pdpt)+8mb(fss)+16mb(swap) regions
   We need to allocate these 16 tables from pdpt region and populate the entries 
   First entry of page directory will map to first page table and each entry in page table will map to 4kb space
   Ex: First entry of first page tabel(corresponding to first physical page from 0x00000000 to 0x00000FFF) 
   will have 0x00000 as base address and second entry(corresponding to second page from 0x00001000 to 0x00001FFF) 
   as 0x00001 each entry stores the 20 bits of base address of each page */

void initialize_v_mappings()
{
    unsigned int addr, pd_addr, pt_addr;
	if(is_pdpt_full())
	{ 
		return SYSERR;
	}
	addr = get_free_page_pdpt();
	pd_addr = (struct pd_t *)addr;        			// this will be used as page directory
	GOLDEN_PD_BASE = addr;							// saving the base address of golden page directiry

	//kprintf("GOLDEN_PD_BASE-> 0x%08X\n", GOLDEN_PD_BASE);

    reset_pd(pd_addr);

    //map 57mb(0x00000000 to 0x03900000) of address, so the msb 10 bits varies from 0x0 to 0xE which is nothing but 15 entries
    unsigned int upper_limit = (npages + MAX_PT_SIZE + MAX_FSS_SIZE + MAX_SWAP_SIZE) * PAGE_SIZE;

    unsigned int pde_count;
    
    pde_count = upper_limit >> 22;  //consider msb 10 bits

    //kprintf("\npde_count = %d\n", pde_count);

    void* pd_v = (unsigned int *)pd_addr;
    unsigned int count = (upper_limit / PAGE_SIZE);
	int j=0;
    for(int i=0; i<=pde_count; i++)
    {
        pd_t* pde = (char *)pd_v;
        if( (pde->pd_avail & 0x1) == 0 )
        {
            //pd entry is invalid, so allocate a page table and update this entry
			if(is_pdpt_full())
			{ 
				return SYSERR;
			}
    		addr = get_free_page_pdpt();        
			pt_addr = (struct pt_t *)addr;						// this will be used as page table
			//kprintf("new PT address -> 0x%08X\n", pt_addr);

            reset_pt(pt_addr);

            //initialize the page table content
            void* pt_v = (unsigned int *)pt_addr;
            for(int k=0; ( ( k<(PAGE_SIZE/ENTRY_SIZE) ) && (count != 0) ) ; k++)
            {
                pt_t* pte = (char *)pt_v;
                pte->pt_pres        =  1; 
                pte->pt_valid       =  1; 
                pte->pt_write       =  1; 
                pte->pt_pwt       	=  1; 
                pte->pt_pcd       	=  1; 
                pte->pt_base        =  j;
                pt_v                += 4;
				j					=  j+1;
                count               =  count - 1;
            }
            //if(i == 0 || i == 14)
			//{
				//print_pt(pt_addr);
			//}
            pt_addr             = pt_addr >> 12;
            pde->pd_pres        = 1; 
            pde->pd_valid       = 1; 
            pde->pd_write       = 1; 
            pde->pd_pwt       	= 1; 
            pde->pd_pcd       	= 1; 
            pde->pd_base        = pt_addr;
			//kprintf("new PT address -> 0x%08X is written in PD entry ->  0x%08X\n", pt_addr, pde);

        }
        pd_v += 4;
    }

    //print_pd(pd_addr);

}

unsigned int evict_random_page_fss()
{
	intmask	mask;			/* Saved interrupt mask		*/
	mask = disable();

    int random_num = rand();
    int random_page_num = (random_num % MAX_FSS_SIZE);
    //kprintf("evict_random_page_fss()-> random page num is -> %d\n", random_page_num);
    return fss_free_track[random_page_num].pg_base_addr;

    restore(mask);
}


void release_phy_resources_user_proc(unsigned int pdbr)
{

	intmask	mask;			/* Saved interrupt mask		*/
	mask = disable();
    
	write_cr3(GOLDEN_PD_BASE | 0x18);
    
    if(pdbr == GOLDEN_PD_BASE)
    {
        //kprintf("Wrong pdbr being asked to be cleared\n");
        restore(mask);
        return SYSERR;
    }

    uint32 c_pd_base_addr = (pdbr & 0xfffff000);

    pd_t* pde   = (pdbr + (4*8));     //starting from 8th entry in PD

    int i,j;
    for(i=8; i < (PAGE_SIZE/ENTRY_SIZE) ; i++ )
    {
        if(pde->pd_valid == 1)
        {
            unsigned int c_pt_base_addr = (pde->pd_base << 12);  
            pt_t* pte   = c_pt_base_addr;

            for( j=0; j<(PAGE_SIZE/ENTRY_SIZE) ; j++)
            {
                if(pte->pt_valid == 1 || pte->pt_pres == 1)
                {
                    if( pte->pt_pres == 1 && pte->pt_swap == 1 )
                    {
                        //kprintf("************************** ERROR: vfree()-> pt_swap is 1 and pt_pres is 1 ****************************************\n");
                    }
                    else if( pte->pt_pres == 1 && pte->pt_swap == 0 )
                    {
                        remove_page_from_fss( pte->pt_base << 12 );
                    }
                    else if( pte->pt_pres == 0 && pte->pt_swap == 1 )
                    {
                        remove_page_from_swap( pte->pt_base << 12 );
                    }
                    
                    pte->pt_pres        =  0; 
                    pte->pt_valid       =  0; 
                    pte->pt_swap        =  0; 
                    pte->pt_write       =  0; 
                    pte->pt_pwt       	=  0; 
                    pte->pt_pcd       	=  0;
                    pte->pt_base        =  0; 
	                ALL_HEAP_SIZE = ALL_HEAP_SIZE - 1;
                }
                pte++;
            }
            
            if( j == (PAGE_SIZE/ENTRY_SIZE) )
            {
                //all the entries are invalidated in the above page table so free it
                
                remove_page_from_pdpt( c_pt_base_addr );
                pde->pd_pres        = 0; 
                pde->pd_valid       = 0; 
                pde->pd_write       = 0; 
                pde->pd_pwt       	= 0; 
                pde->pd_pcd       	= 0; 
                pde->pd_base        = 0;
            }
        }
        pde++;
    }


    if( i == (PAGE_SIZE/ENTRY_SIZE) )
    {
        //all the entries are invalidated in the above page directory, so free it
        
        remove_page_from_pdpt( pdbr );
    }


    restore(mask);
}
