/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>
#include <utils.h>
#include <io.h>
#include <mm.h>
#include <mm_address.h>
#include <sched.h>
#include <errno.h>

#define LECTURA 0
#define ESCRIPTURA 1

#define MAX_WRITE_SIZE 128
char auxBuff[MAX_WRITE_SIZE];

int globalPID = 1000;

int check_fd (int fd, int permissions) {
	if (fd!=1) return -9; /*EBADF*/
	if (permissions!=ESCRIPTURA) return -13; /*EACCES*/
	return 0;
}


int sys_ni_syscall () {
	return -38; /*ENOSYS*/
}

extern int zeos_ticks;

sys_gettime () {

	return zeos_ticks;
}

int sys_getpid () {

	return current()->PID;
}


int ret_from_fork () {

	return 0;
}

/**
*/
int sys_fork () {
		
	struct list_head *freeNode;
	union task_union *childUnion;

	// A --> agafar un pcb lliure de la freequeue

	if (list_empty(&freequeue))
		return -ENOMEM;

	freeNode = list_first(&freequeue);
	list_del(freeNode);
	childUnion = (union task_union*) list_head_to_task_struct(freeNode);

	// B --> heretar les dades de sistema: (PCB i pila)
	copy_data(current(), childUnion, sizeof(union task_union));

	// C --> inicialitzar el nou diretori de pagines del fill
	allocate_DIR((struct task_struct*) childUnion);

	// D --> buscar pag fisiques on mapejar les pag logiques de data+stack del fill
	int pag, physicalPage, i;
	page_table_entry *childPT = get_PT(&childUnion->task);

	for (pag = 0; pag < NUM_PAG_DATA; ++pag) { //NUM_PAG_DATA a mm_address.h

		physicalPage = alloc_frame();

		if (physicalPage != -1) //si hi ha frame, s'associa pag logica a frame
			set_ss_pag(childPT, PAG_LOG_INIT_DATA_P0 + pag, physicalPage); //PAG_LOG_INIT_DATA_P0 a mm_address,h
		
		else { 
		//si no hi ha mes pags lliures, es desalocaten les alocatades fins al moment
			for (i = 0; i < pag; ++i) {

				free_frame(get_frame(childPT, PAG_LOG_INIT_DATA_P0 + i));
				del_ss_pag(childPT, PAG_LOG_INIT_DATA_P0 + i);
			}
			//i es retorna el pcb a la freequeue
			list_add_tail(freeNode, &freequeue);
			return -EAGAIN; 	
		}
	}

	// E --> heretar dades d'ususari
	// E.I --> crear un nou espai d'@
	page_table_entry *parentPT = get_PT(current()); //opbtenim la taula de pagines del pare
	/* 	E.I.A --> les entrades de la taula per a codi+data del sistema, i per a el codi d'usuari
			poden ser una copia de les del pare	
	 	E.I.B --> les entrades per a data+stack, han d'apuntar a les noves pags alocatades*/
	//kernel
	for (pag = 0; pag < NUM_PAG_KERNEL; ++pag)
		set_ss_pag(childPT, pag, get_frame(parentPT, pag));
	//code
	for (pag = 0; pag < NUM_PAG_CODE; ++pag)
		set_ss_pag(childPT, PAG_LOG_INIT_DATA_P0 + pag, get_frame(parentPT, PAG_LOG_INIT_DATA_P0 + pag));

	/* E.II --> copiar data+stack de pare a fill
		no podem accedir a les pags fisiques del fill perque no estan mapejades a la taila del pare
		i tampoc podem mapejar-les directament perque les pags logiques del pare son les mateixes.
		Solucio: mapejar-les temporalment a noves entrades de la taula*/	
	int NPkernelAndCode = NUM_PAG_KERNEL + NUM_PAG_CODE;
	for (pag = NPkernelAndCode; pag < NPkernelAndCode + NUM_PAG_DATA; ++pag) {
		// E.II.A --> fer servir entrades del pare temporalment
		set_ss_pag(parentPT, pag + NUM_PAG_DATA, get_frame(parentPT, pag));
		// E.II.B --> copar les pagines de data+stack
		copy_data((void *)(pag << 12), (void *)((pag + NUM_PAG_DATA) << 12), PAGE_SIZE);
		// E.II.C --> allberar les pagines 
		del_ss_pag(parentPT, pag + NUM_PAG_DATA);
	}
	
	//E.II.C --> fer un flush del TLB perque el pare no pugui accedir a les pagines del fill
	set_cr3(get_DIR(current()));

	// F --> assignar un PID al fill 
	childUnion->task.PID = ++globalPID;

	// G --> inicialitza els altres caps del pcb fill

	int regEBP;
	__asm__ __volatile__ (
		"movl %%ebp, %0;"
		: "=g" (regEBP)
		:
	);

	regEBP = (regEBP - (int)(current())) + (int)(childUnion);
	childUnion->task.kernel_esp = regEBP + sizeof(DWord);

	DWord tempEBP =*(DWord *)regEBP;

	// H --> preparar el fill per el canvi de context
	childUnion->task.kernel_esp = sizeof(DWord);
	*(DWord *)(childUnion->task.kernel_esp) = (DWord)&ret_from_fork;
	childUnion->task.kernel_esp = sizeof(DWord);
	*(DWord *)(childUnion->task.kernel_esp) = tempEBP;

	// I --> posar el fill a la ready list
	list_add_tail(&(childUnion->task.list), &readyqueue);

	// J --> retornar el PID del fill
	return childUnion->task.PID;
}

void sys_exit()
{  
}

int sys_write(int fd, char * buffer, int size) {

	//parameter checking	
	int err = check_fd(fd, ESCRIPTURA),
		written = 0,	//number of chars written
		finalW = 0;

	if (err)
		return err;
	if (buffer == NULL)
		return -ENULLBUFFER;
	if (size <= 0)
		return -ENEGATIVESIZE;

	while (size >= MAX_WRITE_SIZE) {
		// in case that parameter buffer (buffer) is bigger than or buffer (auxBuff) iterate over the parameter buffer
		if (copy_from_user(buffer, auxBuff, MAX_WRITE_SIZE) >= 0) {

			written = sys_write_console(auxBuff, MAX_WRITE_SIZE); //written should be MAX_WRITE_SIZE
			finalW += written;
			buffer += written;
			size -= written;
		}
	}
	
	if (copy_from_user(buffer, auxBuff, size) >= 0) {

		written = sys_write_console(auxBuff, size);
		finalW += written;
		buffer += written;
		size -= written;
	}

	return finalW;
}

