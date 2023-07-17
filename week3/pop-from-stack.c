#include <stdio.h>
#include <unistd.h>

#define __NR_pop_syscall 454

int main() 
{
	long long int ret = syscall(__NR_pop_syscall);

	if (ret == -1) {
		printf("Failed to pop from stack.\n");
		return -1;
	}

	printf("pop [%lld] from stack.\n", ret);

	return 0;
}
