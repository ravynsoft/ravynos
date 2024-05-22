/*
 * fontconfig/test/test-pthread.c
 *
 * Copyright © 2000 Keith Packard
 * Copyright © 2013 Raimund Steger
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the author(s) not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  The authors make no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * THE AUTHOR(S) DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fontconfig/fontconfig.h>

struct thr_arg_s
{
    int thr_num;
};

static void
run_query (void)
{
    FcPattern *pat = FcPatternCreate (), *match;
    FcResult result;

    FcPatternAddString (pat, FC_FAMILY, (const FcChar8 *) "sans-serif");
    FcPatternAddBool (pat, FC_SCALABLE, FcTrue);
    FcConfigSubstitute (NULL, pat, FcMatchPattern);
    FcDefaultSubstitute (pat);
    match = FcFontMatch (NULL, pat, &result);
    if (result != FcResultMatch || !match)
    {
	fprintf (stderr, "ERROR: No matches found\n");
    }
    if (match)
	FcPatternDestroy (match);
    FcPatternDestroy (pat);
}

static void
run_reinit (void)
{
    if (!FcInitReinitialize ())
    {
	fprintf (stderr, "ERROR: Reinitializing failed\n");
    }
}

#define NTEST 3000

static void *
run_test_in_thread (void *arg)
{
    struct thr_arg_s *thr_arg = (struct thr_arg_s *) arg;
    int thread_num = thr_arg->thr_num;

    fprintf (stderr, "Worker %d: started (round %d)\n", thread_num % 2, thread_num / 2);
    if ((thread_num % 2) == 0)
    {
	run_query ();
    }
    else
    {
	run_reinit ();
    }
    fprintf (stderr, "Worker %d: done (round %d)\n", thread_num % 2, thread_num / 2);

    return NULL;
}

int
main (int argc, char **argv)
{
    pthread_t threads[NTEST];
    struct thr_arg_s thr_arg[NTEST];
    int i, j;

    for (i = 0; i < NTEST; i++)
    {
	int result;

	fprintf (stderr, "Thread %d (worker %d round %d): creating\n", i, i % 2, i / 2);
	thr_arg[i].thr_num = i;
	result = pthread_create (&threads[i], NULL, run_test_in_thread,
				 (void *) &thr_arg[i]);
	if (result != 0)
	{
	    fprintf (stderr, "Cannot create thread %d\n", i);
	    break;
	}
    }
    for (j = 0; j < i; j++)
    {
	pthread_join(threads[j], NULL);
	fprintf (stderr, "Joined thread %d\n", j);
    }
    FcFini ();

    return 0;
}
