#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define __NR_push_syscall 453

int main(int argc, char* argv[])
{
	if (argc != 2) {
		printf("Usage: %s [value]\n", argv[0]);
		return -1;
	}

	long long int val = atoi(argv[1]);
	if (!val) {
		printf("Invalid value entered.\n");
		return -1;
	}

	if (syscall(__NR_push_syscall, val) == -1) {
		printf("Failed to push into stack.\n");
		return -1;
	}

	printf("push [%lld] into stack.\n", val);

	return 0;
}
