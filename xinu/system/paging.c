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

#define PDPT 1
#define FSS  2
#define SWAP 3


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
        addr = addr+ 4096;
    }

    
/*-----------------   Code to initialize FSS region   ------------------*/

    addr = (unsigned int)min_fss;
    for(int i=0; i<MAX_FSS_SIZE; i++)
    {
        fss_free_track[i].avail            = TRUE;
        fss_free_track[i].pg_base_addr     = addr;
        addr = addr+ 4096;
    }

/*-----------------   Code to initialize SWAP region   ------------------*/

    addr = (unsigned int)min_swap;
    for(int i=0; i<MAX_SWAP_SIZE; i++)
    {
        swap_free_track[i].avail            = TRUE;
        swap_free_track[i].pg_base_addr     = addr;
        addr = addr+ 4096;
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



/*-----------------   Code to print swap free tracking structure ---------*/



/*------------------  Code to return free page base address from requesting region --------*/

unsigned int get_free_page(int region)
{
    int i=0;
    if(region == PDPT) 
    {
        while(pdpt_free_track[i].avail == FALSE)
        {
            i++;
        }
        pdpt_free_track[i].avail = FALSE;
        return pdpt_free_track[i].pg_base_addr;
    }
    else if(region == FSS)
    {
        return 0;
    }
    else if(region == SWAP) 
    {
        return 0;
    }

}


/*----------------    Code to reset page directory  -------------------*/
void reset_pd(unsigned int addr)
{
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

}



/*----------------    Code to reset page table  -------------------*/
void reset_pt(unsigned int addr)
{
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

}



void print_pd(unsigned int addr)
{
	intmask	mask;			/* Saved interrupt mask		*/
	mask = disable();
	kprintf("\nPD at address = 0x%08x\n", addr);
	kprintf("PDE_Address      Content\n");
	kprintf("-----------      ---------\n");
    for(int i=0; i<(PAGE_SIZE/ENTRY_SIZE); i++)
    {
        void* v = (unsigned int *)addr;
        pd_t* pde = (char *)v;
        kprintf("0x%08x         0x%08x\n", pde, pde->pd_base);
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
	kprintf("PTE_Address      Content\n");
	kprintf("-----------      ---------\n");
    for(int i=0; i<(PAGE_SIZE/ENTRY_SIZE); i++)
    {
        void* pt_v = (unsigned int *)addr;
        pt_t* pte = (char *)pt_v;
        kprintf("0x%08x         0x%08x\n", pte, pte->pt_base);
        addr += 4;
        if(i==260) break;
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
	addr = get_free_page(PDPT);
	pd_addr = (struct pt_t *)addr;        			// this will be used as page directory
	GOLDEN_PD_BASE = addr;							// saving the base address of golden page directiry

	kprintf("GOLDEN_PD_BASE-> 0x%08X\n", GOLDEN_PD_BASE);

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
    		addr = get_free_page(PDPT);        
			pt_addr = (struct pt_t *)addr;						// this will be used as page table
			kprintf("new PT address -> 0x%08X\n", pt_addr);

            reset_pt(pt_addr);

            //initialize the page table content
            void* pt_v = (unsigned int *)pt_addr;
            for(int k=0; ( ( k<(PAGE_SIZE/ENTRY_SIZE) ) && (count != 0) ) ; k++)
            {
                pt_t* pte = (char *)pt_v;
                pte->pt_pres        =  1; 
                pte->pt_avail       =  1; 
                pte->pt_write       =  1; 
                pte->pt_pwt       	=  1; 
                pte->pt_pcd       	=  1; 
                pte->pt_base        =  j;
                pt_v                += 4;
				j					=  j+1;
                count               =  count - 1;
            }
            if(i == 0 || i == 14)
			{
				print_pt(pt_addr);
			}
            pt_addr             = pt_addr >> 12;
            pde->pd_pres        = 1; 
            pde->pd_avail       = 1; 
            pde->pd_write       = 1; 
            pde->pd_pwt       	=  1; 
            pde->pd_pcd       	=  1; 
            pde->pd_base        = pt_addr;
			kprintf("new PT address -> 0x%08X is written in PD entry ->  0x%08X\n", pt_addr, pde);

        }
        pd_v += 4;
    }

    print_pd(pd_addr);

}

