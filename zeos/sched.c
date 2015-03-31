/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>

/**
 * Container for the Task array and 2 additional pages (the first and the last one)
 * to protect against out of bound accesses.
 */
union task_union protected_tasks[NR_TASKS+2]
  __attribute__((__section__(".data.task")));

union task_union *task = &protected_tasks[1]; /* == union task_union task[NR_TASKS] */

#if 0
struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return list_entry( l, struct task_struct, list);
}
#endif

extern struct list_head blocked;


/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry * get_DIR (struct task_struct *t) 
{
	return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry * get_PT (struct task_struct *t) 
{
	return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))<<12);
}


int allocate_DIR(struct task_struct *t) 
{
	int pos;

	pos = ((int)t-(int)task)/sizeof(union task_union);

	t->dir_pages_baseAddr = (page_table_entry*) &dir_pages[pos]; 

	return 1;
}


void cpu_idle (void) {

	__asm__ __volatile__("sti": : :"memory");

	while (1) {
		;
	}
}


void init_idle (void) {

	// 1r node de la freequeue
	struct list_head *firstNode = list_first(&freequeue);
	list_del(firstNode); // l'eliminem per pillar després l'init com a 1a posicio

	// carrego l'idle_task amb el task struct d'aquest 1r element
	idle_task = list_head_to_task_struct(firstNode);
	idle_task->PID = 0;
	// inicilitzo el seu direcori de pagines
	int e = allocate_DIR(idle_task); 
	if(!e) return -E_NOT_DIR_ALLOCATED;

	// perque es pugui restaurar el context despres d'un task switch
	// agafo l'union del idle process per a fer servir la pila
	union task_union * idle_union = (union task_union *) idle_task;
	idle_union->stack[KERNEL_STACK_SIZE - 1] = cpu_idle;
	idle_union->stack[KERNEL_STACK_SIZE - 2] = 0; //posem 0 pero es igualel valor
	// posar el kernel esp al top de la pila
	idle_task->kernel_esp = (unsigned long) &idle_union->stack[KERNEL_STACK_SIZE - 2];
}


void init_task1 (void) {

	// posar el pid a 1
	struct list_head *firstNode = list_first(&freequeue);
	list_del(firstNode); // l'eliminem

	struct task_struct * init_task = list_head_to_task_struct(firstNode);
	init_task->PID = 1;

	// init direcotri de pagines
	int e = allocate_DIR(init_task); 
	if(!e) return -E_NOT_DIR_ALLOCATED;

	// init espai de direccions
	set_user_pages(init_task);

	// posem l'esp al top de la pila
	union task_union * init_union = (union task_union *) init_task;

	// update TSS to make it point the new task segment stack
	tss.esp0 = (unsigned long) &init_union->stack[KERNEL_STACK_SIZE];

	// set its page directory as the current page directory in the system
	set_cr3(get_DIR(init_task));

}


void init_sched () {
	
	int i;
	INIT_LIST_HEAD(&freequeue);
	INIT_LIST_HEAD(&readyqueue);

	for (i = 0; i < NR_TASKS; ++i) 
		list_add(&task[i].task.list, &freequeue);

}


struct task_struct* current()
{
  int ret_value;
  
  __asm__ __volatile__(
  	"movl %%esp, %0"
	: "=g" (ret_value)
  );
  return (struct task_struct*)(ret_value&0xfffff000);
}

/** retorna la @ del pcb on esta el list_head que passem
	: el listHead està en un struct de 4kb, mascara de 12 bits
	--> struct list_head *l
*/
struct task_struct *list_head_to_task_struct (struct list_head *l) {

	return (struct task_struct*) ((unsigned int)l & 0xfffff000);
}

/**
*/
void task_switch(union task_union *newTu) {
	__asm__ __volatile__(
		"pushl %esi;"
		"pushl %edi;"
		"pushl %ebx;"
	);

	inner_task_switch(newTu);

	__asm__ __volatile__(
		"popl %ebx;"
		"popl %edi;"
		"popl %esi;"
	);
}

void inner_task_switch (union task_union* newTu) {

	tss.esp0 = (unsigned long) &newTu->stack[KERNEL_STACK_SIZE];
	
	set_cr3(get_DIR(&newTu->task));

	struct task_struct * curr = current();

	__asm__ __volatile__ (
	 	"movl %%ebp, %0"			
	 	:"=g" (curr->kernel_esp)	// output
	);

	/*CANVI! Restaurem ebp i ja som al nou procés*/
	__asm__ __volatile__ (
		"movl %0, %%esp\n\t"
		:								// output
		:"g" (newTu->task.kernel_esp)	// input
	);

	/*Restaurem ebp de la pila*/
  	__asm__ __volatile__ (
  		"popl %%ebp\n\t"
		:
		:
	);	

  	/*return*/
	__asm__ __volatile__ (
		"ret\n\t"
		:
		:
	);

}



