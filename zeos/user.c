#include <libc.h>

void workload1 () {

	int f = 10000,
		res = 0;
		i, j;

	for (int i = 0; i < f * f; ++i)
		for (int j = 0; j < f * f; ++j)
			res += res + i * j - j % i + i + j - j;
}

void workload2 () {


}

void workload3 () {

	
}


int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */



  while (1);
}
