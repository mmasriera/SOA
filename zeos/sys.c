/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#define LECTURA 0
#define ESCRIPTURA 1

#define WRITE_SIZE 4


int check_fd(int fd, int permissions) {
	if (fd!=1) return -9; /*EBADF*/
	if (permissions!=ESCRIPTURA) return -13; /*EACCES*/
	return 0;
}

int sys_ni_syscall() {
	return -38; /*ENOSYS*/
}

int sys_getpid() {
	return current()->PID;
}

int sys_fork() {
	int PID=-1;

	// creates the child process

	return PID;
}

void sys_exit()
{  
}

int sys_write(int fd, char * buffer, int size) {
	printk("sys_write");
	//parameter checking	
	int err = check_fd(fd,ESCRIPTURA);
	if (err)
		return err;
	if (buffer == NULL || size <= 0)
		return -1;		// possibly we should create many ERR codes

	//parameters OK
	char auxBuff[4];	// our own buffer
	int result = 0;
	while(size > 4) {
		printk("hello!");
		// in case that parameter buffer (buffer) is bigger than or buffer (auxBuff) iterate over the parameter buffer
		copy_from_user(buffer,auxBuff,WRITE_SIZE);
		result += sys_write_console(auxBuff,WRITE_SIZE);

		buffer += WRITE_SIZE; 	// offset
		size -= WRITE_SIZE;		// substract from size the size of our partial write
	}	

	// base case, size <= 4
	copy_from_user(buffer,auxBuff,size);
	result += sys_write_console(auxBuff,size);

	return result;
	
}








