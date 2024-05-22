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

#define NTHR 100
#define NTEST 100

struct thr_arg_s
{
	int thr_num;
};

static void test_match(int thr_num,int test_num)
{
	FcPattern *pat;
	FcPattern *match;
	FcResult  result;

	FcInit();

	pat = FcNameParse((const FcChar8 *)"New Century Schoolbook");
		
	FcConfigSubstitute(0,pat,FcMatchPattern);
	FcDefaultSubstitute(pat);
	
	match = FcFontMatch(0,pat,&result);
		
	FcPatternDestroy(pat);
	FcPatternDestroy(match);
}

static void *run_test_in_thread(void *arg)
{
	struct thr_arg_s *thr_arg=(struct thr_arg_s *)arg;
	int thread_num = thr_arg->thr_num;
	int i=0;

	for(;i<NTEST;i++) test_match(thread_num,i);

	printf("Thread %d: done\n",thread_num);

	return NULL;
}

int main(int argc,char **argv)
{
	pthread_t threads[NTHR];
	struct thr_arg_s thr_args[NTHR];
	int i, j;

	printf("Creating %d threads\n",NTHR);

	for(i = 0;i<NTHR;i++)
	{
		int result;
		thr_args[i].thr_num=i;
		result = pthread_create(&threads[i],NULL,run_test_in_thread,
					(void *)&thr_args[i]);
		if(result!=0)
		{
			fprintf(stderr,"Cannot create thread %d\n",i);
			break;
		}
	}

	for(j=0;j<i;j++)
	{
		pthread_join(threads[j],NULL);
		printf("Joined thread %d\n",j);
	}

	FcFini();

	return 0;
}
