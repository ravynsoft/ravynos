/*****************************************************************************
 *
 * mtdev - Multitouch Protocol Translation Library (MIT license)
 *
 * Copyright (C) 2010 Henrik Rydberg <rydberg@euromail.se>
 * Copyright (C) 2010 Canonical Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 ****************************************************************************/

#include <src/match.h>
#include <stdio.h>
#include <time.h>

#define ITS 4000000
static const int n1 = 4;
static const int n2 = 4;

static void test1()
{
	int A[DIM2_FINGER] = {
		1013,
		3030660,
		3559354,
		12505925,
		19008450,
		6946421,
		6118613,
		698020,
		3021800,
		1017,
		37573,
		3242018,
		8152794,
		1266053,
		942941,
		462820,
	};
	int index[DIM_FINGER], i;
	mtdev_match(index, A, 4, 4);
	for (i = 0; i < 4; i++)
		printf("match[%d] = %d\n", i, index[i]);
}

static void test2()
{
	int A[DIM2_FINGER] = {
		0,
		4534330,
		22653552,
		12252500,
		685352,
		4534330,
		0,
		9619317,
		28409530,
		6710170,
		22653552,
		9619317,
		0,
		47015292,
		29788572,
		2809040,
		10428866,
		38615920,
		17732500,
		719528,
		12113945,
		28196220,
		46778656,
		405,
		14175493,
	};
	int index[DIM_FINGER], i;
	mtdev_match(index, A, 5, 5);
	for (i = 0; i < 5; i++)
		printf("match[%d] = %d\n", i, index[i]);
}

static void speed1()
{
	/* column-by-column matrix */
	int A[DIM2_FINGER];
	int x1[DIM_FINGER] = { 1, 5, 2, 3, 4, 5, 6, 7, 8 };
	int y1[DIM_FINGER] = { 1, 5, 2, 3, 4, 6, 6, 7, 8 };
	int x2[DIM_FINGER] = { 1, 3, 2, 4, 5, 6, 7, 8 };
	int y2[DIM_FINGER] = { 1, 3, 2, 4, 5, 6, 7, 8 };
	int index[DIM_FINGER];
	int i, j, k;

	clock_t t1 = clock();
	for (k = 0; k < ITS; k++) {
		for (i = 0; i < n1; i++) {
			for (j = 0; j < n2; j++) {
				A[i + n1 * j] =
					(x1[i] - x2[j]) * (x1[i] - x2[j]) +
					(y1[i] - y2[j]) * (y1[i] - y2[j]);
			}
		}
		mtdev_match(index, A, n1, n2);
	}
	clock_t t2 = clock();

	printf("%lf matches per second\n",
	       ITS * ((float)CLOCKS_PER_SEC / (t2 - t1)));

	for (i = 0; i < n1; i++)
		printf("match[%d] = %d\n", i, index[i]);

}

static void speed2()
{
	struct trk_coord p1[] = {
		{ 1, 1 },
		{ 5, 5 },
		{ 2, 2 },
		{ 3, 3 },
		{ 4, 4 },
	};
	struct trk_coord p2[] = {
		{ 1, 1 },
		{ 3, 3 },
		{ 2, 2 },
		{ 4, 4 },
		{ 5, 5 },
	};
	const unsigned char *p;
	int i;

	clock_t t1 = clock();
	for (i = 0; i < ITS; i++)
		p = mtdev_match_four(p1, n1, p2, n2);
	clock_t t2 = clock();

	printf("%lf matches per second\n",
	       ITS * ((float)CLOCKS_PER_SEC / (t2 - t1)));

	for (i = 0; i < n2; i++)
		printf("match[%d] = %d\n", i, p[i]);

}

int main(int argc, char *argv[])
{
	printf("test1\n");
	test1();
	printf("test2\n");
	test2();
	printf("speed1\n");
	speed1();
	printf("speed2\n");
	speed2();
	printf("done\n");
	return 0;
}
