#include <libc.h>

char buff[24];

int pid;

int add (int par1, int par2) {
	
	__asm__ __volatile__(
		"addl  %%ebx,%%eax"
                :"=a"(par1)
                :"a"(par1), "b"(par2)
	);
	return par1;
}

int __attribute__ ((__section__(".text.main")))
  
  main (void) {
	/* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
	/* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

  	//testing write syscall
	write(1,"usuario",7);



	while (1) {}
	return 0;
}
