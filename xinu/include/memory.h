/* memory.h - roundmb, truncmb, freestk */

#define	PAGE_SIZE	4096
#define	HOLESTART	((char *)(640 * 1024))
#define	HOLEEND		((char *)(1024 * 1024))

/*----------------------------------------------------------------------
 * roundmb, truncmb - Round or truncate address to memory block size
 *----------------------------------------------------------------------
 */
#define	roundmb(x)	(char *)( (7 + (uint32)(x)) & (~7) )
#define	truncmb(x)	(char *)( ((uint32)(x)) & (~7) )

/*----------------------------------------------------------------------
 *  freestk  --  Free stack memory allocated by getstk
 *----------------------------------------------------------------------
 */
#define	freestk(p,len)	freemem((char *)((uint32)(p)		\
				- ((uint32)roundmb(len))	\
				+ (uint32)sizeof(uint32)),	\
				(uint32)roundmb(len) )

struct	memblk	{			/* See roundmb & truncmb	*/
	struct	memblk	*mnext;		/* Ptr to next free memory blk	*/
	uint32	mlength;		/* Size of blk (includes memblk)*/
	};
extern	struct	memblk	memlist;	/* Head of free memory list	*/
extern	void	*minheap;		/* Start of heap		*/
extern	void	*maxheap;		/* Highest valid heap address	*/

//extern	struct	memblk	pdptlist;	/* Head of free memory list in pdpt region	*/
extern	void	*min_pdpt;		/* Start of pdpt region		*/
extern	void	*max_pdpt;		/* Highest valid pdpt region	*/


extern	void	*min_fss;		/* Start of pdpt region		*/
extern	void	*max_fss;		/* Highest valid pdpt region	*/


extern	void	*min_swap;		/* Start of pdpt region		*/
extern	void	*max_swap;		/* Highest valid pdpt region	*/

/* Added by linker */

extern	int	text;			/* Start of text segment	*/
extern	int	etext;			/* End of text segment		*/
extern	int	data;			/* Start of data segment	*/
extern	int	edata;			/* End of data segment		*/
extern	int	bss;			/* Start of bss segment		*/
extern	int	ebss;			/* End of bss segment		*/
extern	int	end;			/* End of program		*/
