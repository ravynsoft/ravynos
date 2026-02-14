#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <notify.h>
#include <dispatch/dispatch.h>

#define forever for(;;)

#define MAX_REG 1000
#define QUIT 1000000

int token[MAX_REG];
char *tname[MAX_REG];
int nregs = 0;
pid_t mypid;
int scount = 0;
uint64_t actions = 0;

uint32_t
pick_token(uint32_t x)
{
	uint32_t i, n;
	
	if (nregs == 0) return MAX_REG;
	
	i = (x / 100) % MAX_REG;
	for (n = 0; n < MAX_REG; n++)
	{
		if (token[i] >= 0) return i;
		i = (i + i) % MAX_REG;
	}
	
	return MAX_REG;
}

uint32_t
empty_token(uint32_t x)
{
	uint32_t i, n;
	
	if (nregs == MAX_REG) return MAX_REG;
	
	i = (x / 100) % MAX_REG;
	for (n = 0; n < MAX_REG; n++)
	{
		if (token[i] < 0) return i;
		i = (i + i) % MAX_REG;
	}
	
	return MAX_REG;
}

char *
random_name(uint32_t x)
{
	char *s = NULL;
	asprintf(&s, "Random.Notify.Test.%u.%u", mypid, x);
	return s;
}

void
do_something()
{
	printf(".");
	fflush(stdout);
	scount++;
	if (actions >= QUIT)
	{
		printf("\nTest Complete\n");
		exit(0);
	}
	
	if ((scount % 100) == 0) printf("\n%20llu %10u ", actions, scount);
}

/* 0 */
void
random_cancel(uint32_t x)
{
	uint32_t i = pick_token(x);
	if (i >= MAX_REG) return;

	notify_cancel(token[i]);
	free(tname[i]);
	tname[i] = NULL;
	token[i] = -1;

	nregs--;
}

/* 1 */
void
random_register_check(uint32_t x)
{
	int t;
	uint32_t i = empty_token(x);
	if (i >= MAX_REG) return;

	char *s = random_name(x);
	if (s == NULL) return;

	if (notify_register_check(s, &t) == NOTIFY_STATUS_OK)
	{
		token[i] = t;
		tname[i] = s;
		nregs++;
	}
	else
	{
		free(s);
	}
}

/* 2 */
void
random_register_dispatch(uint32_t x)
{
	int t;
	uint32_t i = empty_token(x);
	if (i >= MAX_REG) return;
	
	char *s = random_name(x);
	if (s == NULL) return;
	
	if (notify_register_dispatch(s, &t, dispatch_get_main_queue(), ^(int x){ do_something(); }) == NOTIFY_STATUS_OK)
	{
		token[i] = t;
		tname[i] = s;
		nregs++;
	}
	else
	{
		free(s);
	}
}

/* 3 */
void
random_register_plain(uint32_t x)
{
	int t;
	uint32_t i = empty_token(x);
	if (i >= MAX_REG) return;
	
	char *s = random_name(x);
	if (s == NULL) return;
	
	if (notify_register_plain(s, &t) == NOTIFY_STATUS_OK)
	{
		token[i] = t;
		tname[i] = s;
		nregs++;
	}
	else
	{
		free(s);
	}
}

/* 4 */
void
random_post(uint32_t x)
{
	uint32_t i;

	if (0 == (x % 10))
	{
		char *s = random_name(x);
		notify_post(s);
		free(s);
		return;
	}

	i = pick_token(x);
	if (i >= MAX_REG) return;

	notify_post(tname[i]);
}

/* 5 */
void
random_check(uint32_t x)
{
	int c;
	uint32_t i = pick_token(x);
	if (i >= MAX_REG) return;

	notify_check(token[i], &c);
}

/* 6 */
void
random_get_state(uint32_t x)
{
	uint64_t v;
	uint32_t i = pick_token(x);
	if (i >= MAX_REG) return;

	notify_get_state(token[i], &v);
}

/* 7 */
void
random_set_state(uint32_t x)
{
	uint64_t v = x;
	uint32_t i = pick_token(x);
	if (i >= MAX_REG) return;
	
	notify_set_state(token[i], v);
}

void
random_notify()
{
	uint32_t x, act, nap;

	forever
	{
		x = arc4random();
		nap = ((x / 10) % 10) * 1000;
		act = x % 8;

		switch (act)
		{
			case 0: random_cancel(x); break;
			case 1: random_register_check(x); break;
			case 2: random_register_dispatch(x); break;
			case 3: random_register_plain(x); break;
			case 4: random_post(x); break;
			case 5: random_check(x); break;
			case 6: random_get_state(x); break;
			case 7: random_set_state(x); break;
			default: break;
		}

		usleep(nap);

		actions++;
	}
}

int
main(int argc, char *argv[])
{
	uint32_t i;

	for (i = 0; i < MAX_REG; i++) token[i] = -1;

//	dispatch_queue_t randomq = dispatch_queue_create("Random Queue", NULL);
	dispatch_queue_t randomq = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
	mypid = getpid();

	printf("%20u %10u ", 0, 0);
	dispatch_async(randomq, ^{ random_notify(); });
	dispatch_main();

	return 0;
}
