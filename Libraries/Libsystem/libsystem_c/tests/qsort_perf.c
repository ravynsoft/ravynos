/*
 * Randomized test for qsort() routine.
 *
 * Includes code derived from stdlib/FreeBSD/qsort.c and the copyright header
 * in that file applies.
 */

#include <sys/cdefs.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <darwintest.h>
#include <darwintest_utils.h>

static void qsort1(void *aa, size_t n, size_t es,
        int (*cmp)(const void *, const void *));

static int
cmp_int(const void *aa, const void *bb)
{
    int a, b;

    a = *(const int *)aa;
    b = *(const int *)bb;
    return(a - b);
}

#define nelm 1000000

T_DECL(qsort_perf, "qsort perf test", T_META_CHECK_LEAKS(NO))
{
    int i;
    int arr[nelm];
    int save[nelm];
    uint64_t time_elapsed;

    // ----- 25-75 -----

    int k = nelm/4;
    for (i = 0; i < k; i++) {
        save[i] = i;
    }
    for (i = k; i < nelm; i++) {
        save[i] = i - k;
    }

    bcopy(save, arr, sizeof(arr));
    dt_timer_start("25-75 (qsort)");
    qsort(arr, nelm, sizeof(arr[0]), cmp_int);
    time_elapsed = dt_timer_stop("25-75 (qsort)");
    T_LOG("25-75 (qsort): %lld ms", time_elapsed / NSEC_PER_MSEC);
    for (i = 1; i < nelm; i++) {
        if(arr[i - 1] > arr[i]) {
            T_ASSERT_FAIL("arr[%d]=%d > arr[%d]=%d", i - 1, arr[i - 1], i, arr[i]);
            break;
        }
    }

    bcopy(save, arr, sizeof(arr));
    dt_timer_start("25-75 (Bentley)");
    qsort1(arr, nelm, sizeof(arr[0]), cmp_int);
    time_elapsed = dt_timer_stop("25-75 (Bentley)");
    T_LOG("25-75 (Bentley): %lld ms", time_elapsed / NSEC_PER_MSEC);
    for (i = 1; i < nelm; i++) {
        if(arr[i - 1] > arr[i]) {
            T_ASSERT_FAIL("arr[%d]=%d > arr[%d]=%d", i - 1, arr[i - 1], i, arr[i]);
            break;
        }
    }
    // ----- 50-50 -----

    k = nelm/2;
    for (i = 0; i < k; i++) {
        save[i] = i;
    }
    for (i = k; i < nelm; i++) {
        save[i] = i - k;
    }

    bcopy(save, arr, sizeof(arr));
    dt_timer_start("50-50 (qsort)");
    qsort(arr, nelm, sizeof(arr[0]), cmp_int);
    time_elapsed = dt_timer_stop("50-50 (qsort)");
    T_LOG("50-50 (qsort): %lld ms", time_elapsed / NSEC_PER_MSEC);
    for (i = 1; i < nelm; i++) {
        if(arr[i - 1] > arr[i]) {
            T_ASSERT_FAIL("arr[%d]=%d > arr[%d]=%d", i - 1, arr[i - 1], i, arr[i]);
            break;
        }
    }

    bcopy(save, arr, sizeof(arr));
    dt_timer_start("50-50 (Bentley)");
    qsort1(arr, nelm, sizeof(arr[0]), cmp_int);
    time_elapsed = dt_timer_stop("50-50 (Bentley)");
    T_LOG("50-50 (Bentley): %lld ms", time_elapsed / NSEC_PER_MSEC);
    for (i = 1; i < nelm; i++) {
        if(arr[i - 1] > arr[i]) {
            T_ASSERT_FAIL("arr[%d]=%d > arr[%d]=%d", i - 1, arr[i - 1], i, arr[i]);
            break;
        }
    }

    // ----- median-of-3 killer -----

    k = nelm / 2;
    for (i = 1; i <= k; i++) {
        if(i % 2 == 1) {
            save[i - 1] = i;
            save[i] = k + i;
        }
        save[k + i - 1] = 2 * i;
    }

    bcopy(save, arr, sizeof(arr));
    dt_timer_start("median-of-3 killer (qsort)");
    qsort(arr, nelm, sizeof(arr[0]), cmp_int);
    time_elapsed = dt_timer_stop("median-of-3 killer (qsort)");
    T_LOG("median-of-3 (qsort): %lld ms", time_elapsed / NSEC_PER_MSEC);
    for (i = 1; i < nelm; i++) {
        if(arr[i - 1] > arr[i]) {
            T_ASSERT_FAIL("arr[%d]=%d > arr[%d]=%d", i - 1, arr[i - 1], i, arr[i]);
        }
    }

    bcopy(save, arr, sizeof(arr));
    dt_timer_start("median-of-3 killer (Bentley)");
    qsort1(arr, nelm, sizeof(arr[0]), cmp_int);
    time_elapsed = dt_timer_stop("median-of-3 killer (Bentley)");
    T_LOG("median-of-3 (Bentley): %lld ms", time_elapsed / NSEC_PER_MSEC);
    for (i = 1; i < nelm; i++) {
        if(arr[i - 1] > arr[i]) {
            T_ASSERT_FAIL("arr[%d]=%d > arr[%d]=%d", i - 1, arr[i - 1], i, arr[i]);
        }
    }

    // ----- random -----

    for (i = 0; i < nelm; i++) {
        save[i] = random();
    }

    bcopy(save, arr, sizeof(arr));
    dt_timer_start("random (qsort)");
    qsort(arr, nelm, sizeof(arr[0]), cmp_int);
    time_elapsed = dt_timer_stop("random (qsort)");
    T_LOG("random (qsort): %lld ms", time_elapsed / NSEC_PER_MSEC);
    for (i = 1; i < nelm; i++) {
        if(arr[i - 1] > arr[i]) {
            T_ASSERT_FAIL("arr[%d]=%d > arr[%d]=%d", i - 1, arr[i - 1], i, arr[i]);
        }
    }


    bcopy(save, arr, sizeof(arr));
    dt_timer_start("random (Bentley)");
    qsort1(arr, nelm, sizeof(arr[0]), cmp_int);
    time_elapsed = dt_timer_stop("random (Bentley)");
    T_LOG("random (Bentley): %lld ms", time_elapsed / NSEC_PER_MSEC);
    for (i = 1; i < nelm; i++) {
        if(arr[i - 1] > arr[i]) {
            T_ASSERT_FAIL("arr[%d]=%d > arr[%d]=%d", i - 1, arr[i - 1], i, arr[i]);
        }
    }

    T_PASS("All tests completed successfully.");
}

/* qsort1 -- qsort interface implemented by faster quicksort */

#define SWAPINIT(a, es) swaptype =                            \
    (a - (char*) 0) % sizeof(long) || es % sizeof(long) ? 2 : \
    es == sizeof(long) ? 0 : 1;
#define swapcode(TYPE, parmi, parmj, n) {  \
    long i = (n) / sizeof(TYPE);           \
    register TYPE *pi = (TYPE *) (parmi);  \
    register TYPE *pj = (TYPE *) (parmj);  \
    do {                                   \
        register TYPE t = *pi;             \
        *pi++ = *pj;                       \
        *pj++ = t;                         \
    } while (--i > 0);                     \
}
static void swapfunc(char *a, char *b, int n, int swaptype)
{   if (swaptype <= 1) swapcode(long, a, b, n)
    else swapcode(char, a, b, n)
}
#define swap(a, b)                         \
    if (swaptype == 0) {                   \
        long t = * (long *) (a);           \
        * (long *) (a) = * (long *) (b);   \
        * (long *) (b) = t;                \
    } else                                 \
        swapfunc(a, b, es, swaptype)
#define vecswap(a, b, n) if (n > 0) swapfunc(a, b, n, swaptype)

#define min(x, y) ((x)<=(y) ? (x) : (y))

static char *med3(char *a, char *b, char *c, int (*cmp)(const void *, const void *))
{
    return cmp(a, b) < 0 ?
            (cmp(b, c) < 0 ? b : (cmp(a, c) < 0 ? c : a ) )
            : (cmp(b, c) > 0 ? b : (cmp(a, c) < 0 ? a : c ) );
}

static void
qsort1(void *aa, size_t n, size_t es,
      int (*cmp)(const void *, const void *))
{
	char *pa, *pb, *pc, *pd, *pl, *pm, *pn;
	int  d, r, swaptype;
	char *a = aa;

	SWAPINIT(a, es);
	if (n < 7) { /* Insertion sort on small arrays */
		for (pm = a + es; pm < a + n*es; pm += es)
			for (pl = pm; pl > a && cmp(pl-es, pl) > 0; pl -= es)
				swap(pl, pl-es);
		return;
	}
	pm = a + (n/2) * es;
	if (n > 7) {
		pl = a;
		pn = a + (n-1) * es;
		if (n > 40) { /* On big arrays, pseudomedian of 9 */
			d = (n/8) * es;
			pl = med3(pl, pl+d, pl+2*d, cmp);
			pm = med3(pm-d, pm, pm+d, cmp);
			pn = med3(pn-2*d, pn-d, pn, cmp);
		}
		pm = med3(pl, pm, pn, cmp); /* On mid arrays, med of 3 */
	}
	swap(a, pm); /* On tiny arrays, partition around middle */
	pa = pb = a + es;
	pc = pd = a + (n-1)*es;
	for (;;) {
		while (pb <= pc && (r = cmp(pb, a)) <= 0) {
			if (r == 0) { swap(pa, pb); pa += es; }
			pb += es;
		}
		while (pb <= pc && (r = cmp(pc, a)) >= 0) {
			if (r == 0) { swap(pc, pd); pd -= es; }
			pc -= es;
		}
		if (pb > pc) break;
		swap(pb, pc);
		pb += es;
		pc -= es;
	}
	pn = a + n*es;
	r = min(pa-a,  pb-pa);    vecswap(a,  pb-r, r);
	r = min(pd-pc, pn-pd-es); vecswap(pb, pn-r, r);
	if ((r = pb-pa) > es) qsort1(a, r/es, es, cmp);
	if ((r = pd-pc) > es) qsort1(pn-r, r/es, es, cmp);
}
