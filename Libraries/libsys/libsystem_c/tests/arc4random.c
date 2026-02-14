/*-
 * Copyright (c) 2011 David Schultz
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>

#include <darwintest.h>
#include <darwintest_utils.h>

#define LEN (1024 * 32)
static void * stress(void *ptr __unused)
{
	uint32_t value;
	uint8_t buf[LEN + 1];
	buf[LEN] = 0;
	for (int i = 0; i < LEN; i++){
		value = arc4random();
		if (value % 100 == 0)
			arc4random_stir();
		if ((value & 0x70) == 0)
			arc4random_uniform(value);
		if ((value & 0x7) == 0)
			arc4random_buf(buf, (size_t)i);
	}
	T_ASSERT_EQ(buf[LEN], 0, NULL);

	return NULL;
}

T_DECL(arc4random_stress, "arc4random() stress")
{
	const int ncpu = dt_ncpu();

	pthread_t thr[ncpu];
	for (int i = 0; i < ncpu; i++){
		T_ASSERT_POSIX_ZERO(pthread_create(&thr[i], NULL, stress, NULL), NULL);
	}
	for (int i = 0; i < ncpu; i++){
		T_ASSERT_POSIX_ZERO(pthread_join(thr[i], NULL), NULL);
	}
}

/*
 * BUFSIZE is the number of bytes of rc4 output to compare.  The probability
 * that this test fails spuriously is 2**(-BUFSIZE * 8).
 */
#define	BUFSIZE		8

T_DECL(arc4random_fork, "arc4random() shouldn't return the same sequence in child post-fork()")
{
	struct shared_page {
		char parentbuf[BUFSIZE];
		char childbuf[BUFSIZE];
	} *page;
	pid_t pid;
	char c;

	page = mmap(NULL, sizeof(struct shared_page), PROT_READ | PROT_WRITE,
		    MAP_ANON | MAP_SHARED, -1, 0);
	T_ASSERT_NE(page, MAP_FAILED, "mmap()");

	arc4random_buf(&c, 1);

	pid = fork();
	if (pid < 0) {
		T_ASSERT_FAIL("fork() failed");
	} else if (pid == 0) {
		/* child */
		arc4random_buf(page->childbuf, BUFSIZE);
		exit(0);
	} else {
		/* parent */
		int status;
		arc4random_buf(page->parentbuf, BUFSIZE);
		T_ASSERT_EQ(wait(&status), pid, "wait() returns child pid");
	}
	T_EXPECT_NE(memcmp(page->parentbuf, page->childbuf, BUFSIZE), 0, "sequences are distinct");
}

#define  ARC4_EXPECTATION ((2u << 31)  - 1)/2
#define  ITER 10000000
#define  PROB_THRESH sqrt(6*log2(ITER)/(ITER*1.0))
/* Simple randomness test using a chernoff bound
 * If this fails with probability (1 - 2/(ITER)^{4})
 *  arc4random is NOT a uniform distribution between
 *  0 and 2**32 - 1. Note that this test does not guarantee
 *  that arc4random. Is correct, just that it isn't terribly
 *  wrong.
 * */
T_DECL(arc4random_stats, "ensure arc4random() is random")
{
	int i;
	int total = 0;
	for (i = 0; i < ITER; i++) {
		total += arc4random() > ARC4_EXPECTATION;
	}
	double prob = total/(ITER*1.0);
	T_EXPECT_LT(fabs(prob - 0.5), PROB_THRESH, "probability is within threshold");
}
