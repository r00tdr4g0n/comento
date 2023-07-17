#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define __NR_enqueue_syscall 455

int main(int argc, char* argv[])
{
	if (argc != 2) {
		printf("Usage : %s [value]\n", argv[0]);
		return -1;
	}

	long long int val = atoi(argv[1]);

	if (!val) {
		printf("Invalid value entered.\n");
		return -1;
	}

	if (syscall(__NR_enqueue_syscall, val) == -1) {
		printf("Failed to enqueue.");
		return -1;
	}

	printf("Enqueue : %lld\n", val);

	return 0;
}
