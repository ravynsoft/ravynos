#include<stdio.h>
#include<sys/resource.h>

static volatile int * array1_ref = NULL;
static long last_stack_addr = 0;

static void
recursive_fn(void)
{
	volatile int array1[1024]; /* leave this as it is */
	int addr;
	last_stack_addr = (long)&addr;
	array1_ref = array1; /* make sure compiler cannot discard storage */
	array1[0] = 0;
	if (array1_ref == 0) {
		/* fool clang -Winfinite-recursion */
		return;
	}
	recursive_fn();
	return;
}

int
main(__unused int argc, __unused const char *argv[])
{
	struct rlimit save;

	if (getrlimit(RLIMIT_STACK, &save) == -1) {
		printf("child: ERROR - getrlimit");
		return 2;
	}
	printf("child: LOG - current stack limits cur=0x%llx, max=0x%llx, inf=0x%llx\n", save.rlim_cur, save.rlim_max, RLIM_INFINITY);

	if(save.rlim_cur >= save.rlim_max) {
		printf("child: ERROR - invalid limits");
		return 2;
	}

	if(save.rlim_max == RLIM_INFINITY) {
		printf("child: ERROR - rlim_max = RLIM_INFINITY");
		return 2;
	}

	save.rlim_cur += 4;

	printf("child: LOG - Raising setrlimit rlim_cur=0x%llx, rlim_max=0x%llx\n", save.rlim_cur, save.rlim_max);

	if (setrlimit(RLIMIT_STACK, &save) == -1) {
		printf("child: ERROR - Raising the limits failed.");
		return 2;
	}

	printf("child: LOG - Make the stack grow such that a SIGSEGV is generated.\n");
	recursive_fn();
	return 0;
}
