/*  main.c  - main */

#include <xinu.h>

process	main(void)
{




    //printf("End of Heap = %X\n", maxheap);
    //printf("Start of Heap = %X\n", minheap);
    //printf("End of BSS = %X\n", ebss);
    //printf("End of Data = %X\n", edata);
    //printf("End of Text = %X\n", etext);
	
    printf("code/text segment start address = %x\n", &text);
    printf("code/text segment end address = %x\n", &etext);

    printf("Data segment start address = %x\n", &data);
    printf("Data segment end address = %x\n", &edata);
	
    printf("BSS segment start address = %x\n", &bss);
    printf("BSS segment end address = %x\n", &ebss);
    
	printf("Stack segment start address = %x\n", minheap);
    printf("Stack segment end address = %x\n", maxheap);
   
	printf("PDPT segment start address = %x\n", min_pdpt);
    printf("PDPT segment end address = %x\n", max_pdpt);
   
	printf("fss segment start address = %x\n", min_fss);
    printf("fss segment end address = %x\n", max_fss);

	printf("swap segment start address = %x\n", min_swap);
    printf("swap segment end address = %x\n", max_swap);

	/* Run the Xinu shell */

	recvclr();
	resume(create(shell, 8192, 50, "shell", 1, CONSOLE));

	/* Wait for shell to exit and recreate it */

	while (TRUE) {
		receive();
		sleepms(200);
		kprintf("\n\nMain process recreating shell\n\n");
		resume(create(shell, 4096, 20, "shell", 1, CONSOLE));
	}
	return OK;
    
}
