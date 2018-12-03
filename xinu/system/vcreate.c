
#include <xinu.h>

local	int newpid();


pid32 vcreate(void *funcaddr, uint32 ssize, uint32 hsize, pri16 priority, char *name, int nargs, ...)
{

		uint32		savsp, *pushsp;
		intmask 	mask;    	/* Interrupt mask		*/
		pid32		pid;		/* Stores new process id	*/
		struct	procent	*prptr;		/* Pointer to proc. table entry */
		int32		i;
		uint32		*a;		/* Points to list of args	*/
		uint32		*saddr;		/* Stack address		*/

		mask = disable();
        kprintf("\n----------------- vcreate() ----------------\n\n");
		if (ssize < MINSTK)
		{
			ssize = MINSTK;
		}
		ssize = (uint32) roundmb(ssize);
		if ( (priority < 1) || ((pid=newpid()) == SYSERR) || ((saddr = (uint32 *)getstk(ssize)) == (uint32 *)SYSERR) ) 
		{
			restore(mask);
			return SYSERR;
		}

		if((hsize+ALL_HEAP_SIZE) > MAX_HEAP_SIZE)
		{
			kprintf("its a system error..!\n");
			restore(mask);
			return SYSERR;
		}

		ALL_HEAP_SIZE = ALL_HEAP_SIZE + hsize;
    	unsigned int addr, pd_addr;
		if(is_pdpt_full())
		{ 
			return SYSERR;
		}
		addr = get_free_page_pdpt();
		pd_addr = (struct pd_t *)addr;        			// this will be used as page directory
		reset_pd(pd_addr);

		prcount++;
		prptr = &proctab[pid];

		/* Initialize process table entry for new process */
		prptr->prstate = PR_SUSP;	/* Initial state is suspended	*/
		prptr->prprio = priority;
		prptr->pdbr = pd_addr;
		prptr->v_add_counter = 0x4000000;
		prptr->avail_v_heap = hsize;
		prptr->max_v_heap = hsize;
		prptr->prstkbase = (char *)saddr;
		prptr->prstklen = ssize;
		prptr->prname[PNMLEN-1] = NULLCH;
		for (i=0 ; i<PNMLEN-1 && (prptr->prname[i]=name[i])!=NULLCH; i++)
			;
		prptr->prsem = -1;
		prptr->prparent = (pid32)getpid();
		prptr->prhasmsg = FALSE;

		/* Set up stdin, stdout, and stderr descriptors for the shell	*/
		prptr->prdesc[0] = CONSOLE;
		prptr->prdesc[1] = CONSOLE;
		prptr->prdesc[2] = CONSOLE;


		/* mapping a new pd to golden pd */
		pd_t* c_pde = pd_addr;		

		pd_t* g_pde = GOLDEN_PD_BASE;							// struct pointer poining to golden pd
		for(int i =0; i< 8;i++)
		{
            c_pde->pd_pres        	= 	g_pde->pd_pres; 
            c_pde->pd_valid       	= 	g_pde->pd_valid;  
            c_pde->pd_write       	= 	g_pde->pd_write; 
            c_pde->pd_pwt       	= 	g_pde->pd_pwt; 
            c_pde->pd_pcd       	= 	g_pde->pd_pcd;   
            c_pde->pd_base        	= 	g_pde->pd_base; 
			c_pde 					= 	c_pde + 1;
			g_pde 					= 	g_pde + 1;  
		}
		print_pd(pd_addr);


		/* Initialize stack as if the process was called		*/

		*saddr = STACKMAGIC;
		savsp = (uint32)saddr;

		/* Push arguments */
		a = (uint32 *)(&nargs + 1);	/* Start of args		*/
		a += nargs -1;			/* Last argument		*/
		for ( ; nargs > 0 ; nargs--)	/* Machine dependent; copy args	*/
			*--saddr = *a--;	/* onto created process's stack	*/
		*--saddr = (long)INITRET;	/* Push on return address	*/

		/* The following entries on the stack must match what ctxsw	*/
		/*   expects a saved process state to contain: ret address,	*/
		/*   ebp, interrupt mask, flags, registers, and an old SP	*/

		*--saddr = (long)funcaddr;	/* Make the stack look like it's*/
						/*   half-way through a call to	*/
						/*   ctxsw that "returns" to the*/
						/*   new process		*/
		*--saddr = savsp;		/* This will be register ebp	*/
						/*   for process exit		*/
		savsp = (uint32) saddr;		/* Start of frame for ctxsw	*/
		*--saddr = 0x00000200;		/* New process runs with	*/
						/*   interrupts enabled		*/

		/* Basically, the following emulates an x86 "pushal" instruction*/

		*--saddr = 0;			/* %eax */
		*--saddr = 0;			/* %ecx */
		*--saddr = 0;			/* %edx */
		*--saddr = 0;			/* %ebx */
		*--saddr = 0;			/* %esp; value filled in below	*/
		pushsp = saddr;			/* Remember this location	*/
		*--saddr = savsp;		/* %ebp (while finishing ctxsw)	*/
		*--saddr = 0;			/* %esi */
		*--saddr = 0;			/* %edi */
		*pushsp = (unsigned long) (prptr->prstkptr = (char *)saddr);
		restore(mask);
		return pid;
}

/*------------------------------------------------------------------------
 *  newpid  -  Obtain a new (free) process ID
 *------------------------------------------------------------------------
 */
local	pid32	newpid(void)
{
	uint32	i;			/* Iterate through all processes*/
	static	pid32 nextpid = 1;	/* Position in table to try or	*/
					/*   one beyond end of table	*/

	/* Check all NPROC slots */

	for (i = 0; i < NPROC; i++) {
		nextpid %= NPROC;	/* Wrap around to beginning */
		if (proctab[nextpid].prstate == PR_FREE) {
			return nextpid++;
		} else {
			nextpid++;
		}
	}
	return (pid32) SYSERR;
}
