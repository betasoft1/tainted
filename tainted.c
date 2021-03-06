/*
 * Tainted: Tool to get the current taint value and print each set bit in
 * 	    human readable format
 *
 * (C) 2014 - Nikolay Aleksandrov <nikolay@redhat.com>
 *
 * Compile with:
 * gcc -o tainted tainted.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PROC_TAINTED "/proc/sys/kernel/tainted"
#define HELP_FMT "%s [-hi]\nWithout command-line options this tool will print\nthe current taint value with information about\neach set bit.\nh - this help\ni - print information about the different taint bits\n"
#define BIT(x) (1UL << x)

/* Extracted from linux/include/linux/kernel.h */
enum {
	TAINT_PROPRIETARY_MODULE,
	TAINT_FORCED_MODULE,
	TAINT_CPU_OUT_OF_SPEC,
	TAINT_FORCED_RMMOD,
	TAINT_MACHINE_CHECK,
	TAINT_BAD_PAGE,
	TAINT_USER,
	TAINT_DIE,
	TAINT_OVERRIDEN_ACPI_TABLE,
	TAINT_WARN,
	TAINT_CRAP,
	TAINT_FIRMWARE_WORKAROUND,
	TAINT_OOT_MODULE,
	TAINT_UNSIGNED_MODULE,
};

/* Extracted from linux/Documentation/sysctl/kernel.txt */
static const char *kernel_taint_descs[] = {
	[TAINT_PROPRIETARY_MODULE] = "A module with a non-GPL license has been loaded, this\nincludes modules with no license.\nSet by modutils >= 2.4.9 and module-init-tools",
	[TAINT_FORCED_MODULE] = "A module was force loaded by insmod -f",
	[TAINT_CPU_OUT_OF_SPEC] = "Unsafe SMP processors: SMP with CPUs not designed for SMP",
	[TAINT_FORCED_RMMOD] = "A module was forcibly unloaded from the system by rmmod -f",
	[TAINT_MACHINE_CHECK] = "A hardware machine check error occurred on the system",
	[TAINT_BAD_PAGE] = "A bad page was discovered on the system",
	[TAINT_USER] = "The user has asked that the system be marked \"tainted\". This\ncould be because they are running software that directly modifies\nthe hardware, or for other reasons",
	[TAINT_DIE] = "The system has died",
	[TAINT_OVERRIDEN_ACPI_TABLE] = "The ACPI DSDT has been overridden with one supplied by the user\ninstead of using the one provided by the hardware",
	[TAINT_WARN] = "A kernel warning has occurred",
	[TAINT_CRAP] = "A module from drivers/staging was loaded",
	[TAINT_FIRMWARE_WORKAROUND] = "The system is working around a severe firmware bug",
	[TAINT_OOT_MODULE] = "An out-of-tree module has been loaded",
	[TAINT_UNSIGNED_MODULE] = "An unsigned module has been loaded in a kernel supporting module\nsignature",
	NULL
};

int main(int argc, char **argv)
{
	char readbuf[16];
	int opt, i, fd;
	long int flags;

	while ((opt = getopt(argc, argv, "hi")) != -1) {
		switch (opt) {
		case 'i':
			for (i = 0; kernel_taint_descs[i]; i++)
				printf("[%d] %lu - %s\n", i, BIT(i), kernel_taint_descs[i]);
			return 0;
		case 'h':
			printf(HELP_FMT, argv[0]);
			return 0;
		default:
			break;
		}
	}
	fd = open(PROC_TAINTED, O_RDONLY);
	if (fd == -1) {
		perror("open");
		return -errno;
	}
	if (read(fd, readbuf, sizeof(readbuf)) == -1) {
		perror("read");
		return -errno;
	}
	readbuf[sizeof(readbuf) - 1] = '\0';
	flags = strtol(readbuf, NULL, 10);
	if (errno == ERANGE || errno == EINVAL) {
		perror("strtol");
		return -errno;
	}
	printf("Taint value: %ld\n", flags);
	for (i = 0; kernel_taint_descs[i] && i < sizeof(long int) * 8; i++) {
		if (flags & BIT(i))
			printf("[%d] %lu - %s\n",
			       i, BIT(i), kernel_taint_descs[i]);
	}

	return 0;
}
