#include <sys/ioctl.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define KEYRINGCTL_DEVICE_NAME		"keyringctl"
#define KEYRINGCTL_MAGIC_NUMBER		'K'
#define KEYRINGCTL_DEVICE_IOCTL_ADD	_IOW(KEYRINGCTL_MAGIC_NUMBER, 0, unsigned int)

int main(int argc, char* argv[])
{
	int fd = -1;
	int num = 0;

	if (argc != 2) {
		printf("Usage : %s [number]\n", argv[0]);
		return -1;
	}

	fd = open("/dev/" KEYRINGCTL_DEVICE_NAME, O_RDWR);
	num = atoi(argv[1]);

	if (!num) {
		printf("Error : Invalid value entered.\n");
		return -1;
	}

	if (fd < 0) {
		printf("Failed to open device.\n");
		return -1;
	}

	if (ioctl(fd, KEYRINGCTL_DEVICE_IOCTL_ADD, &num) < 0) {
		printf("Failed to do ioctl command.\n");
		return -1;
	}

	return 0;
}
