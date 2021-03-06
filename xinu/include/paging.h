/* paging.h */
#ifndef _PAGING_H_
#define _PAGING_H_

typedef unsigned int	 bsd_t;

/* Structure for a page directory entry */

typedef struct {

  unsigned int pd_pres	: 1;		/* page table present?		*/
  unsigned int pd_write : 1;		/* page is writable?		*/
  unsigned int pd_user	: 1;		/* is use level protection?	*/
  unsigned int pd_pwt	: 1;		/* write through cachine for pt?*/
  unsigned int pd_pcd	: 1;		/* cache disable for this pt?	*/
  unsigned int pd_acc	: 1;		/* page table was accessed?	*/
  unsigned int pd_mbz	: 1;		/* must be zero			*/
  unsigned int pd_fmb	: 1;		/* four MB pages?		*/
  unsigned int pd_global: 1;		/* global (ignored)		*/
  unsigned int pd_valid : 1;		/* pd entry is valid?		*/
  unsigned int pd_avail : 2;		/* for programmer's use		*/
  unsigned int pd_base	: 20;		/* location of page table?	*/
} pd_t;

/* Structure for a page table entry */

typedef struct {

  unsigned int pt_pres	: 1;		/* page is present?		*/
  unsigned int pt_write : 1;		/* page is writable?		*/
  unsigned int pt_user	: 1;		/* is use level protection?	*/
  unsigned int pt_pwt	: 1;		/* write through for this page? */
  unsigned int pt_pcd	: 1;		/* cache disable for this page? */
  unsigned int pt_acc	: 1;		/* page was accessed?		*/
  unsigned int pt_dirty : 1;		/* page was written?		*/
  unsigned int pt_mbz	: 1;		/* must be zero			*/
  unsigned int pt_global: 1;		/* should be zero in 586	*/
  unsigned int pt_valid : 1;		/* pt entry is valid?		*/
  unsigned int pt_swap  : 1;		/* present is swap?		*/
  unsigned int pt_avail : 1;		/* for programmer's use		*/
  unsigned int pt_base	: 20;		/* location of page?		*/
} pt_t;

typedef struct{
  unsigned int pg_offset : 12;		/* page offset			*/
  unsigned int pt_offset : 10;		/* page table offset		*/
  unsigned int pd_offset : 10;		/* page directory offset	*/
} virt_addr_t;

typedef struct{
  unsigned int fm_offset : 12;		/* frame offset			*/
  unsigned int fm_num : 20;		/* frame number			*/
} phy_addr_t;


typedef struct{
    bool8           avail;
    unsigned int    pg_base_addr;
    unsigned int    pte_addr;
    pid32           pg_owner; 
}free_track_t;

uint32 npages;


unsigned long GOLDEN_PD_BASE;
unsigned long ALL_HEAP_SIZE;

unsigned long fss_used_size;
unsigned long pdpt_used_size;
unsigned long swap_used_size;

/* Macros */

#define PAGE_SIZE       4096    /* number of bytes per page		 		 */
#define MAX_HEAP_SIZE   4096    /* max number of frames for virtual heap		 */
#define MAX_SWAP_SIZE   4096    /* size of swap space (in frames) 			 */
#define MAX_FSS_SIZE    2048    /* size of FSS space  (in frames)			 */
#define MAX_PT_SIZE	    256	/* size of space used for page tables (in frames)	 */

#define ENTRY_SIZE      4   /* size of each page directory entry and page tabel entry  */



free_track_t    pdpt_free_track[MAX_PT_SIZE];
free_track_t    fss_free_track[MAX_FSS_SIZE];
free_track_t    swap_free_track[MAX_SWAP_SIZE];


#endif
