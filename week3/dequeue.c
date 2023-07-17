#include <stdio.h>
#include <unistd.h>

#define __NR_dequeue_syscall 456

int main(int argc, char* argv[])
{
	if (argc != 1) {
		printf("Usage : %s\n", argv[0]);
		return -1;
	}

	long long int ret = syscall(__NR_dequeue_syscall);
	if (ret == -1) {
		printf("Failed to dequeue.\n");
		return -1;
	}

	printf("Dequeue : %lld\n", ret);

	return 0;
}
