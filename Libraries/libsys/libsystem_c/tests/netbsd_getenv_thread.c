/*	$NetBSD: t_getenv_thread.c,v 1.2 2012/03/15 02:02:23 joerg Exp $ */

/*-
 * Copyright (c) 2010 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Matthias Scheler.
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
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <sys/cdefs.h>
__RCSID("$NetBSD: t_getenv_thread.c,v 1.2 2012/03/15 02:02:23 joerg Exp $");

#include <darwintest.h>

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define	THREADED_NUM_THREADS	8
#define	THREADED_NUM_VARS	16
#define	THREADED_VAR_NAME	"THREADED%zu"
#define	THREADED_RUN_TIME	10

static void	 *thread_putenv(void *);
static void	 *thread_setenv(void *);
static void	 *thread_unsetenv(void *);

static void *
thread_putenv(void *arg)
{
	time_t endtime;
	size_t i;
	static char vars[THREADED_NUM_VARS][128];

	for (i = 0; i < THREADED_NUM_VARS; i++) {
		(void)snprintf(vars[i], sizeof(vars[i]),
		    THREADED_VAR_NAME "=putenv %ld", i, lrand48());
	}

	endtime = *(time_t *)arg;
	do {
		char name[128];

		i = lrand48() % THREADED_NUM_VARS;
		(void)strlcpy(name, vars[i], sizeof(name));
		*strchr(name, '=') = '\0';

		T_QUIET; T_ASSERT_POSIX_ZERO(unsetenv(name), NULL);
		T_QUIET; T_ASSERT_POSIX_ZERO(putenv(vars[i]), NULL);
	} while (time(NULL) < endtime);

	return NULL;
}

static void *
thread_setenv(void *arg)
{
	time_t endtime;

	endtime = *(time_t *)arg;
	do {
		size_t i;
		char name[32], value[64];

		i = lrand48() % THREADED_NUM_VARS;
		(void)snprintf(name, sizeof(name), THREADED_VAR_NAME, i);
		(void)snprintf(value, sizeof(value), "setenv %ld", lrand48());

		T_QUIET; T_ASSERT_POSIX_ZERO(setenv(name, value, 1), NULL);
	} while (time(NULL) < endtime);

	return NULL;
}

static void *
thread_unsetenv(void *arg)
{
	time_t endtime;

	endtime = *(time_t *)arg;
	do {
		size_t i;
		char name[32];

		i = lrand48() % THREADED_NUM_VARS;
		(void)snprintf(name, sizeof(name), THREADED_VAR_NAME, i);

		T_QUIET; T_ASSERT_POSIX_ZERO(unsetenv(name), NULL);
	} while (time(NULL) < endtime);

	return NULL;
}

T_DECL(putenv_thread, "Test concurrent access by putenv(3)")
{
	pthread_t threads[THREADED_NUM_THREADS];
	time_t endtime = time(NULL) + THREADED_RUN_TIME;

	for (int i = 0; i < THREADED_NUM_THREADS; i++) {
		T_QUIET; T_ASSERT_POSIX_ZERO(
				pthread_create(&threads[i], NULL, thread_putenv, &endtime),
				NULL);
	}
	for (int i = 0; i < THREADED_NUM_THREADS; i++) {
		T_QUIET; T_ASSERT_POSIX_ZERO(pthread_join(threads[i], NULL), NULL);
	}
	T_PASS("putenv_thread() completed successfully");
}

T_DECL(setenv_thread, "Test concurrent access by setenv(3)")
{
	pthread_t threads[THREADED_NUM_THREADS];
	time_t endtime = time(NULL) + THREADED_RUN_TIME;

	for (int i = 0; i < THREADED_NUM_THREADS; i++) {
		T_QUIET; T_ASSERT_POSIX_ZERO(
				pthread_create(&threads[i], NULL, thread_setenv, &endtime),
				NULL);
	}
	for (int i = 0; i < THREADED_NUM_THREADS; i++) {
		T_QUIET; T_ASSERT_POSIX_ZERO(pthread_join(threads[i], NULL), NULL);
	}
	T_PASS("setenv_thread() completed successfully");
}

T_DECL(unsetenv_thread, "Test unsetenv(3) with threads",
		T_META_ENVVAR("MallocStackLogging=lite"))
{
	pthread_t threads[THREADED_NUM_THREADS];
	time_t endtime = time(NULL) + THREADED_RUN_TIME;

	for (int i = 0; i < THREADED_NUM_THREADS; i++) {
		T_QUIET; T_ASSERT_POSIX_ZERO(
				pthread_create(&threads[i], NULL, thread_unsetenv, &endtime),
				NULL);
	}
	for (int i = 0; i < THREADED_NUM_THREADS; i++) {
		T_QUIET; T_ASSERT_POSIX_ZERO(pthread_join(threads[i], NULL), NULL);
	}
	T_PASS("unsetenv_thread() completed successfully");
}

