#include <xinu.h>

/* Memory bounds */

void    *minpdpt;       /* Minimum address of PDPT  */
void    *maxpdpt;       /* Maximum level of PDPT */




extern free_track_t    pdpt_free_track[MAX_PT_SIZE];
extern free_track_t    fss_free_track[MAX_FSS_SIZE];
extern free_track_t    swap_free_track[MAX_SWAP_SIZE];

#define PDPT 1
#define FSS  2
#define SWAP 3


/*------------------------------------------------------
  PDPT region initialization
-------------------------------------------------------*/
void	pdptinit(void) {

       //struct	memblk	*memptr;	/* Ptr to memory block		*/

       /* Initialize the free memory list */

       /* Note: we pre-allocate  the "hole" between 640K and 1024K */
	//maxheap already initialized in i386.c
//      maxheap = (void *)0x600000;	/* Assume 64 Mbytes for now */
        minpdpt = (void *)(maxheap+1);
        maxpdpt = (void *) ( minpdpt + (MAX_PT_SIZE*PAGE_SIZE) - 1 );

        return;
}

/*-----------------   Code to initialize FSS region   ------------------*/



/*-----------------   Code to initialize SWAP region   ------------------*/

/*--------------------------------------------------------------
  Initialize free pages tracking structures for pdpt,fss,swap
--------------------------------------------------------------*/
void initialize_fr_trk_structs()
{
    unsigned int addr = (unsigned int)minpdpt;
    for(int i=0; i<MAX_PT_SIZE; i++)
    {
        pdpt_free_track[i].avail            = TRUE;
        pdpt_free_track[i].pg_base_addr     = addr;
        addr += 4096;
    }

    
/*-----------------   Code to initialize FSS region   ------------------*/


/*-----------------   Code to initialize SWAP region   ------------------*/
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
    free_track_t struct_name;
    int i=0;
    if(region == PDPT) 
    {
        pdpt_free_track;
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


/*----------------    Code to reset page  -------------------*/
void reset_page(unsigned int addr)
{
    for(int i=0; i<(PAGE_SIZE/ENTRY_SIZE); i++)
    {
        void* v = (unsigned int *)addr;

        //unsigned int* ch_ptr = (char *)v;
        //*ch_ptr = 0xDEADBEEF;
        //kprintf("\n\nFree page base addr = %08x and its content = %08x\n\n", ch_ptr, *ch_ptr);
        
        pd_t* pde = (char *)v;
 
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
        kprintf("0x%08x         0x%08x\n", pde, *pde);
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
        kprintf("0x%08x         0x%08x\n", pte, *pte);
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
    unsigned int addr = get_free_page(PDPT);        // this will be used as page directory
    //kprintf("\n\nFree page base addr = %08x\n\n", addr);
    //addr = get_free_page(PDPT);
    //kprintf("\n\nFree page base addr = %08x\n\n", addr);
    //addr = get_free_page(PDPT);
    //kprintf("\n\nFree page base addr = %08x\n\n", addr);
    //print_pdpt_fr_trk_struct();
    reset_page(addr);

    //map 57mb(0x00000000 to 0x03900000) of address, so the msb 10 bits varies from 0x0 to 0xE
    //which is nothing but 15 entries
    unsigned int upper_limit = (npages + MAX_PT_SIZE + MAX_FSS_SIZE + MAX_SWAP_SIZE) * PAGE_SIZE;

    unsigned int pde_count;
    
    pde_count = upper_limit >> 22;  //consider msb 10 bits

    //kprintf("\npde_count = %d\n", pde_count);

    void* pd_v = (unsigned int *)addr;
    unsigned int count = (upper_limit / PAGE_SIZE);
    for(int i=0; i<=pde_count; i++)
    {
        pd_t* pde = (char *)pd_v;
        if( (pde->pd_avail & 0x1) == 0 )
        {
            //pd entry is invalid, so allocate a page table and update this entry
            unsigned int pt_addr = get_free_page(PDPT);        // this will be used as page tabel
            reset_page(pt_addr);
            //initialize the page table content
            void* pt_v = (unsigned int *)pt_addr;
            for(int j=0; ( ( j<(PAGE_SIZE/ENTRY_SIZE) ) && (count != 0) ) ; j++)
            {
                pt_t* pte = (char *)pt_v;
                pte->pt_pres        =  1; 
                pte->pt_avail       =  1; 
                pte->pt_base        =  j;
                pt_v                += 4;
                count               =  count - 1;
            }
            if(pde_count == i) print_pt(pt_addr);
            //kprintf( "count = %d, i = %d\n", count, i);
            pt_addr             = pt_addr >> 12;
            pde->pd_pres        = 1; 
            pde->pd_avail       = 1; 
            pde->pd_base        = pt_addr;

        }
        pd_v += 4;
    }

    print_pd(addr);

}









