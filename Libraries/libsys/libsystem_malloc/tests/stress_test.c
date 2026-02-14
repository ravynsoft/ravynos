/*
 * malloc_stress:  Stress the heck out of malloc(), and do a lot of
 *                 sanity checks that the malloc()'ed buffers are legit.
 *
 * usage:  malloc_stress [options...]
 *
 * Default:  Do random-sized malloc() calls until malloc() returns NULL
 *           or some signal kills the process.
 *           Randomly do a free() 10% of the time.
 *
 * Options:
 *    -min #bytes   malloc() at least #bytes per malloc() call.
 *    -max #bytes   malloc() no more than #byte per malloc() call.
 *    -mem #bytes   Stop when #bytes has been allocated.
 *    -calls #      Stop when #calls to malloc() have been executed.
 *    -time #sec    Stop if the number of seconds has elapsed.
 *    -seed #       Set the random seed to #.
 *    -free %       Randomly free() some % of the time.
 *    -dbg          Dump internal structures (could be voluminous!)
 *
 * Exits with status code:
 *    0    PASS
 *    1    FAIL (plus lots of stdout output)
 *    99   Illegal arguments or internal error.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <err.h>

#if DARWINTEST
#include <darwintest.h>
T_GLOBAL_META(T_META_RUN_CONCURRENTLY(true));

#endif

/* globals */
int rseed;	 /* initial random seed value */
int min_bytes; /* minimum bytes to malloc */
int max_bytes; /* maximum bytes to malloc */

long long max_mem;		 /* maximum total memory to allocate */
long long mem_allocated; /* memory allocated so far */

int free_pct; /* call free() this pct of the time */

int max_malloc_calls;  /* Maximum number of malloc() calls to make */
int malloc_calls_made; /* Actual count of malloc()'s done */
int malloc_bufs;	   /* # of active malloc()'ed buffers (calls - freed) */

int time_limit; /* How many seconds should this program run? */

int debug_dump; /* Set to 1 to dump internal structures */

/* Array size for remembering malloc info */
#define MAX_MALLOC_INFO 10000000 /* 10 million */

/* Remember significant info of each malloc() done. */
struct malloc_info {
	void *buf_ptr;		   /* the ptr returned by malloc() */
	int buf_size;		   /* count of bytes allocated */
	int set_val;		   /* the value the buffer was set to */
	int this_buffer_freed; /* has this buffer been free()'d? */
} minfo_array[MAX_MALLOC_INFO];

/* Generate a random percentage (1-100) */
#define D100 (1 + (rand() % 100))

int signal_happened = 0; /* gets set to signal# if one happens */

void
sig_handler(int signo)
{
	signal_happened = signo;
	return;
}

void
trap_signals()
{
	int signum;
	for (signum = 1; signum < NSIG; signum++) {
		if (signal(signum, sig_handler) == SIG_ERR && (signum != SIGKILL && signum != SIGSTOP)) {
#if DARWINTEST
			T_FAIL("Could not trap signal %d (OK)\n", signum);
#else
			printf("INFO:  Could not trap signal %d (OK)\n", signum);
#endif
		};
	}
}

/* Display a brief usage message and exit with status 99 */
void
usage()
{
	printf("\nusage: malloc_stress [options...]\n");
	printf("Default: Allocate up to %d buffers, with a %d percent free() chance.\n", MAX_MALLOC_INFO, free_pct);
	printf("         Buffer sizes range from %d to %d bytes.\n", min_bytes, max_bytes);
	printf("\nOptions:\n");
	printf("   -min #bytes    Minimum buffer size to allocate (at least 1).\n");
	printf("   -max #bytes    Maximum buffer size (up to 2gb allowed).\n");
	printf("   -mem #bytes    Stop when total allocations surpasses this.\n");
	printf("   -free #        Percent chance to free a buffer.\n");
	printf("   -calls #       Maximum allocations to do, then stop.\n");
	printf("   -seed #        Set the random seed to this number.\n");
	printf("   -dbg           Produce some debugging outputs.\n");
	exit(99);
}

/*
 * summarize():  Give a brief synopsis of the number of malloc() calls made,
 *               free() calls made, total memory allocated.
 */
void
summarize()
{
	int mx;

	printf("INFO: %d total malloc() calls made.\n", malloc_calls_made);
	printf("INFO: %d total free() calls made during testing.\n", malloc_calls_made - malloc_bufs);
	printf("INFO: Total memory allocated: %lld bytes\n", mem_allocated);

	if (debug_dump) {
		for (mx = 0; mx < malloc_calls_made; mx++) {
			printf("Buffer index %d: Address = 0x%llx, Size=%d, Fill=%d ", mx, (unsigned long long)(minfo_array[mx].buf_ptr),
					minfo_array[mx].buf_size, minfo_array[mx].set_val);
			if (minfo_array[mx].this_buffer_freed) {
				printf("[freed]");
			}
			printf("\n");
		}
	}
	printf("INFO: Random seed value = %d\n", rseed);

	return;
}

/*
 * When we hit an end condition, every allocated buffer should be free()-able.
 */
void
cleanup()
{
	int mx;
#ifndef DARWINTEST
	printf("Cleanup:  Started.\n");

	printf("Cleanup:  Every allocated buffer should be free-able.\n");
#endif
	for (mx = 0; mx < malloc_calls_made; mx++) {
		if (!(minfo_array[mx].this_buffer_freed)) {
			free(minfo_array[mx].buf_ptr);
			if (signal_happened) {
#if DARWINTEST
				T_FAIL("Signal %d occurred during free(0x%llx)", signal_happened, (unsigned long long)(minfo_array[mx].buf_ptr));
#else
				printf("FAIL:  Signal %d occurred during free(0x%llx)\n", signal_happened,
						(unsigned long long)(minfo_array[mx].buf_ptr));
				exit(1);
#endif
			}
		}
	}
	return;
}

/*
 * Take a ptr and size, and another ptr and size, and verify they do not
 * overlap.  Return 0 if they are disjoint, or 1 if they overlap.
 */
int
check_overlap(void *p1, int len1, void *p2, int len2)
{
	unsigned long long bob1, eob1, bob2, eob2;

	/* make begin and end ptrs in an integer form for easier comparison */
	bob1 = (unsigned long long)p1;
	eob1 = bob1 + (unsigned long long)len1 - 1LL;

	bob2 = (unsigned long long)p2;
	eob2 = bob2 + (unsigned long long)len2 - 1LL;

	/*
	 * The begin and end points of one buffer should not be contained
	 * between the begin and end points of the other buffer.
	 */
	if ((bob1 >= bob2 && bob1 <= eob2) || (eob1 >= bob2 && eob1 <= eob2) || (bob2 >= bob1 && bob2 <= eob1) ||
			(eob2 >= bob1 && eob2 <= eob1)) {
#ifdef DARWINTEST
		T_FAIL("Buffers overlap:  Buffer 1 (0x%llx, %d bytes), Buffer 2 (0x%llx, %d bytes)", bob1, len1, bob2, len2);
#else
		printf("FAIL:  Buffers overlap:  Buffer 1 (0x%llx, %d bytes), Buffer 2 (0x%llx, %d bytes)\n", bob1, len1, bob2, len2);
#endif
		return 1; /* Indicate buffers overlap */
	}

	return 0; /* buffers do not overlap */
}

void do_test(void);

#ifndef DARWINTEST
int
main(int argc, char *argv[])
{
	int argx;

	malloc_calls_made = malloc_bufs = 0;
	mem_allocated = 0;

	/* Set defaults */
	rseed = (int)time(0);
	min_bytes = 1;
	max_bytes = (1024 * 1024);			/* 1mb */
	max_mem = 0;						/* Continue until malloc() returns NULL */
	free_pct = 10;						/* Do a free() 10% of the time */
	max_malloc_calls = MAX_MALLOC_INFO; /* no larger than our array */

	time_limit = 0;

	debug_dump = 0;

	/*
	 * Process arguments.
	 */
	for (argx = 1; argx < argc; argx++) {
		if (strcmp("-min", argv[argx]) == 0) {
			min_bytes = atoi(argv[++argx]);

		} else if (strcmp("-max", argv[argx]) == 0) {
			max_bytes = atoi(argv[++argx]);

		} else if (strcmp("-mem", argv[argx]) == 0) {
			max_mem = atoll(argv[++argx]);

		} else if (strcmp("-seed", argv[argx]) == 0) {
			rseed = atoi(argv[++argx]);

		} else if (strcmp("-free", argv[argx]) == 0) {
			free_pct = atoi(argv[++argx]);

		} else if (strcmp("-calls", argv[argx]) == 0) {
			max_malloc_calls = atoi(argv[++argx]);

		} else if (strcmp("-time", argv[argx]) == 0) {
			time_limit = atoi(argv[++argx]);

		} else if (strcmp("-dbg", argv[argx]) == 0) {
			debug_dump = 1;

		} else if (strcmp("-h", argv[argx]) == 0) {
			usage();

		} else {
			printf("Unknown argument: '%s'\n", argv[argx]);
			usage();
		}
	}

	/* Sanity check the arguments */

	if (min_bytes < 1) {
		printf("Minimum bytes (-min %d) must be at least 1.\n", min_bytes);
		usage();
	}

	if (max_bytes < 1) {
		printf("Maximum bytes (-max %d) must be at least 1.\n", max_bytes);
		usage();
	}

	if (min_bytes > max_bytes) {
		printf("Minimum bytes (-min %d) must not exceed max bytes (-max %d)\n", min_bytes, max_bytes);
		usage();
	}

	if (free_pct < 0 || free_pct > 100) {
		printf("Percentage to free (-free %d) must be between 0 and 100.\n", free_pct);
		usage();
	}

	if (max_malloc_calls < 1) {
		printf("Maximum malloc calls (-calls %d) must be at least 1.\n", max_malloc_calls);
		usage();
	}

	if (max_malloc_calls > MAX_MALLOC_INFO) {
		printf("This program has been compiled for %d max allocations.\n", MAX_MALLOC_INFO);
		printf("To support more, re-compile malloc_stress.c with a larger MAX_MALLOC_INFO\n");
		usage();
	}

	if (time_limit < 0) {
		printf("The maximum execution time specified (%d seconds) must be 0 or positive.\n", time_limit);
		usage();
	}

	/* Describe our inputs and actions to be taken */
	if (max_malloc_calls) {
		printf("Will call malloc() up to %d times.\n", max_malloc_calls);
	} else {
		printf("Will call malloc() repeatedly until it returns NULL.\n");
	}

	if (min_bytes == max_bytes) {
		printf("Will allocate buffers all of size %d bytes.\n", min_bytes);
	} else {
		printf("Will allocate buffers between %d and %d bytes.\n", min_bytes, max_bytes);
	}

	if (free_pct) {
		printf("Will free() a malloc'ed buffer %d%% of the time.\n", free_pct);
	} else {
		printf("free() will NOT be called between malloc's.\n");
	}

	if (time_limit > 0) {
		printf("Will stop after %d elapsed seconds.\n", time_limit);
	}

	srand(rseed);
	printf("Random seed value = %d\n", rseed);
	fflush(stdout);

	do_test();

	return 0;
}
#endif

void
do_test(void)
{
	time_t start_time = time(0);

	/* Trap all signals that are possible to trap */
	trap_signals();

	/*
	 * Loop until we have some reason to quit.
	 */
	for (;;) {
		int buf_size;
		int bx;
		char *malloc_buf;
		int set_buf_val;
		int save_errno;

		/* Have we exceeded our execution time limit? */
		if (time_limit > 0 && (time(0) - start_time >= time_limit)) {
#if DARWINTEST
			cleanup();
			T_PASS("Ran until time limit without incident.");
			return;
#else
			printf("INFO: Execution time limit has been reached.\n");
			summarize();
			cleanup();
			printf("PASS\n");
			exit(0);
#endif
		}

		/* Is this a good time to free() a buffer? */
		if (malloc_bufs > 0 && (D100 < free_pct)) {
			/* Choose a random malloc'd buffer to free up */
			int random_buf;

			/* Find a currently allocated buffer to free */
			for (;;) {
				random_buf = rand() % malloc_calls_made;
				if (!(minfo_array[random_buf].this_buffer_freed)) {
					free(minfo_array[random_buf].buf_ptr);
					/* If a signal happened, Fail */
					if (signal_happened) {
#if DARWINTEST
						T_FAIL("Signal %d caught during free() of buffer 0x%llx\n", signal_happened,
								(unsigned long long)(minfo_array[random_buf].buf_ptr));
						return;
#else
						summarize();
						printf("FAIL:  Signal %d caught during free() of buffer 0x%llx\n", signal_happened,
								(unsigned long long)(minfo_array[random_buf].buf_ptr));
						exit(1);
#endif
					}

					minfo_array[random_buf].this_buffer_freed = 1;
					malloc_bufs--; /* decrease the count of allocated bufs */

					if (debug_dump) {
						printf("INFO: Freed buffer index %d, (%d bytes) at address 0x%llx\n", random_buf,
								minfo_array[random_buf].buf_size, (unsigned long long)(minfo_array[random_buf].buf_ptr));
					}

					break;
				}
			}
			continue;
		}

		/* Else it is a good time to malloc() a buffer */
		buf_size = min_bytes + (rand() % (max_bytes - min_bytes + 1));
		errno = 0;
		malloc_buf = malloc(buf_size);
		save_errno = errno;

		/* If a signal was caught, summarize and FAIL */
		if (signal_happened) {
#if DARWINTEST
			T_FAIL("Signal %d caught during malloc(%d).", signal_happened, buf_size);
			return;
#else
			summarize();
			printf("FAIL:  Signal %d caught during malloc()!\n", signal_happened);
			printf("Buffer size being allocated was %d bytes.\n", buf_size);
			exit(1);
#endif
		}

		if (debug_dump) {
			printf("INFO: Allocated buffer (%d bytes) at address 0x%llx\n", buf_size, (unsigned long long)malloc_buf);
		}

		/* Did the malloc() succeed? */
		if (malloc_buf == NULL) {
/* malloc() returned NULL;  make sure it was due to ENOMEM */
#if DARWINTEST
			T_ASSERT_EQ_INT(save_errno, ENOMEM, "malloc failed, but with errno = %d instead of ENOMEM.", save_errno);
			cleanup();
			T_PASS("Ran out of memory without incident.");
			return;
#else
			if (save_errno != ENOMEM) {
				printf("FAIL:  malloc failed, but with errno = %d instead of ENOMEM.\n", save_errno);
				summarize();
				exit(1);
			}
			summarize();
			cleanup(); /* Every malloc()'d buffer should be free()-able */
			printf("PASS\n");
			exit(0);
#endif
		}

		/*
		 * Sanity check that this buffer does not overlap with any other
		 * buffer we have previously allocated.   NOTE:  Be sure to
		 * skip minfo_array elements which have been free()'d previously.
		 */
		for (bx = 0; bx < malloc_calls_made; bx++) {
			/* Skip to the next if this buffer was free()'d before */
			if (minfo_array[bx].this_buffer_freed) {
				continue;
			}

			/*
			 * The new buffer should not overlap with any other buffer.
			 */
			if (check_overlap(malloc_buf, buf_size, minfo_array[bx].buf_ptr, minfo_array[bx].buf_size)) {
#if DARWINTEST
				return;
#else
				summarize();
				printf("FAIL: Allocated buffer overlaps with buffer index %d\n", bx);
				exit(1);
#endif
			}
		}

		/*
		 * Verify that we can read the bytes of the buffer.
		 * Note that malloc() does not (have to) initialize the buffer.
		 * If any signal occurs while reading, FAIL.
		 */
		for (bx = 0; bx < buf_size; bx++) {
			char byte;
			*((volatile char *)&byte) = *((volatile char *)malloc_buf + bx);

			if (signal_happened) {
#if DARWINTEST
				T_FAIL("Signal %d caught reading buffer!", signal_happened);
				return;
#else
				summarize();
				printf("FAIL:  Signal %d caught reading buffer!\n", signal_happened);
				exit(1);
#endif
			}
		}

		/*
		 * One malloc() requirement is that the buffer should be aligned
		 * such that any numeric type can be stored using the base address,
		 * assuming the buffer is large enough to hold that numeric type.
		 * If the buffer is not well-aligned we might get a signal like
		 * SIGSEGV (segmentation violation) or SIGBUS.
		 * Be sure to do this test BEFORE the memset() operation below.
		 */
		if (sizeof(short) <= buf_size) {
			*((short *)malloc_buf) = 1;
		}

		if (sizeof(int) <= buf_size) {
			*((int *)malloc_buf) = 2;
		}

		if (sizeof(long) <= buf_size) {
			*((long *)malloc_buf) = 3L;
		}

		if (sizeof(long long) <= buf_size) {
			*((long long *)malloc_buf) = 4LL;
		}

		if (sizeof(float) <= buf_size) {
			*((float *)malloc_buf) = 5.0;
		}

		if (sizeof(double) <= buf_size) {
			*((double *)malloc_buf) = 6.1;
		}

		if (sizeof(long double) <= buf_size) {
			*((long double *)malloc_buf) = 7.2;
		}

		if (signal_happened) {
#if DARWINTEST
			T_FAIL("Signal %d occurred storing numeric types at address 0x%llx (%d bytes)", signal_happened,
					(unsigned long long)malloc_buf, buf_size);
			return;
#else
			printf("\nFAIL:  Signal %d occurred storing numeric types at address 0x%llx (%d bytes)\n", signal_happened,
					(unsigned long long)malloc_buf, buf_size);
			summarize();
			exit(1);
#endif
		}

		/* Pick a random byte value to set the bytes of the buffer to */
		set_buf_val = rand() & 0xFF;
		memset(malloc_buf, set_buf_val, buf_size);
		if (signal_happened) {
#if DARWINTEST
			T_FAIL("Signal %d caught initializing buffer to byte value %d!", signal_happened, set_buf_val);
			return;
#else
			summarize();
			printf("FAIL:  Signal %d caught initializing buffer to byte value %d!\n", signal_happened, set_buf_val);
			exit(1);
#endif
		}

		/* Save the new malloc info */

		minfo_array[malloc_calls_made].buf_ptr = malloc_buf;
		minfo_array[malloc_calls_made].buf_size = buf_size;
		minfo_array[malloc_calls_made].set_val = set_buf_val;
		minfo_array[malloc_calls_made].this_buffer_freed = 0;

		/*
		 * Update counts and sums.  Break out if we hit a requested limit.
		 */
		malloc_calls_made++;
		malloc_bufs++;
		mem_allocated += buf_size;

		if (malloc_calls_made >= max_malloc_calls) {
#if DARWINTEST
			cleanup();
			T_PASS("Maximum malloc calls reached: %d", malloc_calls_made);
			return;
#else
			printf("Maximum malloc calls reached: %d\n", malloc_calls_made);
			summarize();
			cleanup();
			printf("PASS\n");
			exit(0);
#endif
		}

		if (max_mem > 0 && mem_allocated >= max_mem) {
#if DARWINTEST
			cleanup();
			T_PASS("Maximum memory allocation reached: %lld", mem_allocated);
			return;
#else
			printf("Maximum memory allocation reached: %lld\n", mem_allocated);
			cleanup();
			printf("PASS\n");
			exit(0);
#endif
		}

		/* Now verify that no existing buffer has been stepped on */
		for (bx = 0; bx < malloc_calls_made; bx++) {
			int cx;
			unsigned char *bptr = minfo_array[bx].buf_ptr;
			int expected_val = minfo_array[bx].set_val;

			/* A previously free()'d buffer might get re-used */
			if (minfo_array[bx].this_buffer_freed) {
				continue;
			}

			for (cx = 0; cx < minfo_array[bx].buf_size; cx++) {
				if (bptr[cx] != expected_val) {
#if DARWINTEST
					T_FAIL("Allocated buffer [%d] appears to have been stepped on.", bx);
					return;
#else
					summarize();
					printf("FAIL: Allocated buffer [%d] appears to have been stepped on.\n", bx);
					printf("Buffer address: 0x%llx, bad byte index %d\n", (unsigned long long)bptr, cx);
					printf("Expected byte value %d, but found %d\n", expected_val, (int)(bptr[cx]));
					exit(1);
#endif
				}
			}

			/* If any signal occurred, that's a FAIL too. */
			if (signal_happened) {
#if DARWINTEST
				T_FAIL("Signal %d caught validating buffer contents.\n", signal_happened);
				return;
#else
				summarize();
				printf("FAIL:  Signal %d caught validating buffer contents.\n", signal_happened);
				exit(1);
#endif
			}

		} /* end then malloc succeeded */

	} /* end forever loop, until some end-point is reached  */
}

#if DARWINTEST

static void
setup_and_test(void)
{
	/* Set defaults */
	rseed = 42;
	min_bytes = 1;
	max_bytes = (1024 * 1024);			/* 1mb */
	max_mem = (512 * 1024 * 1024);		/* Continue until malloc() returns NULL */
	free_pct = 10;						/* Do a free() 10% of the time */
	max_malloc_calls = MAX_MALLOC_INFO; /* no larger than our array */
	
	time_limit = 15;
	
	debug_dump = 0;
	
	do_test();
}

T_DECL(malloc_stress, "Stress the heck out of malloc()")
{
	setup_and_test();
}

T_DECL(malloc_stress_msl_full, "Stress the heck out of malloc() - enable malloc stack logging, full mode", T_META_ENVVAR("MallocStackLogging=1"))
{
	setup_and_test();
}

T_DECL(malloc_stress_msl_malloc, "Stress the heck out of malloc() - enable malloc stack logging, malloc mode", T_META_ENVVAR("MallocStackLogging=malloc"))
{
	setup_and_test();
}

T_DECL(malloc_stress_msl_vm, "Stress the heck out of malloc() - enable malloc stack logging, vm mode", T_META_ENVVAR("MallocStackLogging=vm"))
{
	setup_and_test();
}

T_DECL(malloc_stress_msl_lite, "Stress the heck out of malloc() - enable malloc stack logging, lite mode", T_META_ENVVAR("MallocStackLogging=lite"))
{
	setup_and_test();
}


#endif