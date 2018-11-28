#include <xinu.h>

/* Memory bounds */

void    *minpdpt;       /* Minimum address of PDPT  */
void    *maxpdpt;       /* Maximum level of PDPT */




extern free_track_t    pdpt_free_track[MAX_PT_SIZE];
extern free_track_t    fss_free_track[MAX_FSS_SIZE];
extern free_track_t    swap_free_track[MAX_SWAP_SIZE];




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
    
	kprintf("PDPT Free List Status:\n");
	kprintf("Index      Available      Page_Base_Address(hex)\n");
	kprintf("-----      ---------      ----------------------\n");
    for(int i=0; i<MAX_PT_SIZE; i++)
    {
        kprintf("%04d           %d                %08x\n",i, pdpt_free_track[i].avail, pdpt_free_track[i].pg_base_addr);
    }

	restore(mask);
}

/*-----------------   Code to print fss free tracking structure ---------*/



/*-----------------   Code to print swap free tracking structure ---------*/











