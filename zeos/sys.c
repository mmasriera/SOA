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