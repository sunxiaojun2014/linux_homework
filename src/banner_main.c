#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <error.h>
#include <linux/ioctl.h>


// ioctl macro
#define HUADENG_IOC_MAGIC 'o'

#define HUADENG_IOCRESET            _IO(HUADENG_IOC_MAGIC, 0)
#define HUADENG_IOCSLUMINANCE       _IOW(HUADENG_IOC_MAGIC, 1, int)
#define HUADENG_IOCTLUMINANCE       _IO(HUADENG_IOC_MAGIC, 2)
#define HUADENG_IOCGLUMINANCE       _IOR(HUADENG_IOC_MAGIC, 3, int)
#define HUADENG_IOCQLUMINANCE       _IO(HUADENG_IOC_MAGIC, 4)
#define HUADENG_IOCXLUMINANCE       _IOWR(HUADENG_IOC_MAGIC, 5, int)
#define HUADENG_IOCHLUMINANCE       _IO(HUADENG_IOC_MAGIC, 6)
int usage()
{
	printf("banner <devno> r/w/s/i <value>\n");
	return 0;
}

void hd_ioctl(int fd, char *cmd, char *arg)
{
	int val = 0;
	switch(cmd[0])
	{
		case 's':
			val = atoi(arg);
			ioctl(fd, HUADENG_IOCSLUMINANCE, &val);
			break;
		case 't':
			val = atoi(arg);
			ioctl(fd, HUADENG_IOCTLUMINANCE, val);
			break;
		case 'g':
			ioctl(fd, HUADENG_IOCGLUMINANCE, &val);
			break;
		case 'q':
			val = ioctl(fd, HUADENG_IOCQLUMINANCE, 0);
			break;
		case 'x':
			val = atoi(arg);
			ioctl(fd, HUADENG_IOCXLUMINANCE, &val);
			break;
		case 'h':
			val = atoi(arg);
			val = ioctl(fd, HUADENG_IOCHLUMINANCE, val);
			break;
		case 'r':
			ioctl(fd, HUADENG_IOCRESET, 0);
			break;
		default:
			printf("unsupported cmd %s\n", cmd);
			break;
	}

	printf("ioctl cmd %s value %d\n", cmd, val);
}

int main(int argc, char *argv[])
{
	int n, fd;
	char name[20], buffer[20];
	
	if (argc < 3)
	{
		return usage();
	}

	if (sprintf(name, "/dev/huadeng%s", argv[1]) < 0)
	{
		printf("sprintf failed.\n");
		return errno;
	}

	fd = open(name, O_RDWR);
	if (fd < 0) 
	{
		// printf("open file %s failed with %s\n", name, error);
		printf("open file %s failed with %d\n", name, errno);
		return -2;
	}

	if (argv[2][0] == 'r') 
	{
		while((n = read(fd, buffer, sizeof(buffer))) > 0)
		{
			printf("read %d bytes:\n %s\n", n ,buffer);
		}
	}
	else if (argv[2][0] == 'w') 
	{
		if (argc < 4)
		{
			printf("missing what to write\n");
			goto tag_close;
		}

		if ((n = write(fd, argv[3], strlen(argv[3]))) > 0)
		{
			printf("write %d bytes:\n %s\n", n, argv[3]);
		}
		else
		{
			printf("write file %s failed %d\n", name, errno);
		}
	}
	else if (argv[2][0] == 'i')
	{
		if (argc < 5)
		{
			printf("missing cmd or arg to ioctl\n");
			goto tag_close;
		}

		hd_ioctl(fd, argv[3], argv[4]);
	}

tag_close:
	close(fd);
	return 0;
}
