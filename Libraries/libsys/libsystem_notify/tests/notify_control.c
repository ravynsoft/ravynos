#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <notify_private.h>

int main(int argc, char *argv[])
{
	pid_t pid;
	int i;
	uint32_t status;

	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-s"))
		{
			pid = atoi(argv[++i]);
			status = notify_suspend_pid(pid);
			if (status != 0) printf("suspend pid %d failed status %u\n", pid, status);
			else printf("suspend pid %d OK\n", pid);
		}
		else if (!strcmp(argv[i], "-r"))
		{
			pid = atoi(argv[++i]);
			status = notify_resume_pid(pid);
			if (status != 0) printf("resume pid %d failed status %u\n", pid, status);
			else printf("resume pid %d OK\n", pid);
		}
	}

	return 0;
}
