# 1 "sys_call_table.S"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "sys_call_table.S"
# 1 "include/asm.h" 1
# 2 "sys_call_table.S" 2
# 1 "include/segment.h" 1
# 3 "sys_call_table.S" 2

.globl sys_call_table; .type sys_call_table, @function; .align 0; sys_call_table:
 .long sys_ni_syscall
 .long sys_ni_syscall
 .long sys_fork	//2
 .long sys_ni_syscall
 .long sys_write //4
 .long sys_ni_syscall
 .long sys_ni_syscall
 .long sys_ni_syscall
 .long sys_ni_syscall
 .long sys_ni_syscall
 .long sys_gettime //10
 .long sys_ni_syscall
 .long sys_ni_syscall
 .long sys_ni_syscall
 .long sys_ni_syscall
 .long sys_ni_syscall
 .long sys_ni_syscall
 .long sys_ni_syscall
 .long sys_ni_syscall
 .long sys_ni_syscall
 .long sys_getpid //20


.globl MAX_SYSCALL
MAX_SYSCALL = (. - sys_call_table)/4
