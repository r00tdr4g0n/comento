/*
 * File        : queue_syscall.c
 * Author      : wonyong Choi
 * Create date : 2023-07-17
 * Modify date : 2023-07-17
 */

#include <linux/kernel.h>
#include <linux/syscalls.h>

#define MAX_QUEUE_SIZE 256
#define DATA_SIZE 8
#define MAX_QUEUE_CNT (MAX_QUEUE_SIZE/DATA_SIZE)

typedef long long int DATA_TYPE, *LP_DATA_TYPE;

DATA_TYPE queue[MAX_QUEUE_CNT];
int front, rear;

SYSCALL_DEFINE1(enqueue_syscall, DATA_TYPE, val)
{
	if ((rear + 1) % MAX_QUEUE_CNT == front) {
		printk(KERN_INFO "queue is full.");
		return -1;
	}

	rear = (rear + 1) % MAX_QUEUE_CNT;
	queue[rear] = val;

	printk(KERN_INFO "Enqueue : %lld", val);

	return 0;
}

SYSCALL_DEFINE0(dequeue_syscall)
{
	long long int ret = 0;
	
	if (rear == front) {
		printk(KERN_INFO "queue is empty.");
		return -1;
	}

	front = (front + 1) % MAX_QUEUE_CNT;
	ret = queue[front];

	printk(KERN_INFO "Dequeue : %lld", ret);

	return ret;
}
