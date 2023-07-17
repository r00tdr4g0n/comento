/*
 * File         : stack_syscall.c
 * Author       : Wonyong Choi
 * Create date  : 2023-07-17
 * Modifiy date : 2023-07-17
 */

#include <linux/kernel.h>
#include <linux/syscalls.h>

#define MAX_STACK_SIZE 256
#define DATA_SIZE 8
#define MAX_STACK_CNT (MAX_STACK_SIZE / DATA_SIZE)

typedef long long int DATA_TYPE, *LP_DATA_TYPE;

DATA_TYPE stack[MAX_STACK_CNT];
int top = -1;

SYSCALL_DEFINE1(push_syscall, DATA_TYPE, val)
{
	if (top >= MAX_STACK_CNT) {
		printk(KERN_INFO "stack is full.");
		return -1;
	}

	top++;
	*((LP_DATA_TYPE)stack + top) = val;

	printk(KERN_INFO "push : %lld", val);

	return 0;
}

SYSCALL_DEFINE0(pop_syscall)
{
	DATA_TYPE ret = 0;

	if (top < 0) {
		printk(KERN_INFO "stack is empty.");
		return -1;
	}

	ret = *((LP_DATA_TYPE)stack + top);
	*((LP_DATA_TYPE)stack + top) = 0;
	top--;

	printk(KERN_INFO "pop : %lld", ret);

	return ret;
}
