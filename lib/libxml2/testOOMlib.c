/*
 * testOOM.c: Test out-of-memory handling
 *
 * See Copyright for the status of this software.
 *
 * Copyright 2003 Red Hat, Inc.
 * Written by: hp@redhat.com
 */

#include "testOOMlib.h"

#include <stdlib.h>
#include <string.h>

#define _TEST_INT_MAX 2147483647
#ifndef TRUE
#define TRUE (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif
#ifndef NULL
#define NULL ((void*)0)
#endif

#include <libxml/xmlmemory.h>

static int fail_alloc_counter = _TEST_INT_MAX;
static int n_failures_per_failure = 1;
static int n_failures_this_failure = 0;
static int n_blocks_outstanding = 0;

/**
 * set_fail_alloc_counter:
 * @until_next_fail: number of successful allocs before one fails
 *
 * Sets the number of allocations until we simulate a failed
 * allocation. If set to 0, the next allocation to run
 * fails; if set to 1, one succeeds then the next fails; etc.
 * Set to _TEST_INT_MAX to not fail anything.
 */
static void
set_fail_alloc_counter (int until_next_fail)
{
  fail_alloc_counter = until_next_fail;
}

/**
 * get_fail_alloc_counter:
 *
 * Returns the number of successful allocs until we'll simulate
 * a failed alloc.
 */
static int
get_fail_alloc_counter (void)
{
  return fail_alloc_counter;
}

/**
 * set_fail_alloc_failures:
 * @failures_per_failure: number to fail
 *
 * Sets how many mallocs to fail when the fail alloc counter reaches
 * 0.
 *
 */
static void
set_fail_alloc_failures (int failures_per_failure)
{
  n_failures_per_failure = failures_per_failure;
}

/**
 * decrement_fail_alloc_counter:
 *
 * Called when about to alloc some memory; if
 * it returns #TRUE, then the allocation should
 * fail. If it returns #FALSE, then the allocation
 * should not fail.
 *
 * returns #TRUE if this alloc should fail
 */
static int
decrement_fail_alloc_counter (void)
{
  if (fail_alloc_counter <= 0)
    {
      n_failures_this_failure += 1;
      if (n_failures_this_failure >= n_failures_per_failure)
        {
          fail_alloc_counter = _TEST_INT_MAX;

          n_failures_this_failure = 0;
        }

      return TRUE;
    }
  else
    {
      fail_alloc_counter -= 1;
      return FALSE;
    }
}

/**
 * test_get_malloc_blocks_outstanding:
 *
 * Get the number of outstanding malloc()'d blocks.
 *
 * Returns number of blocks
 */
int
test_get_malloc_blocks_outstanding (void)
{
  return n_blocks_outstanding;
}

void*
test_malloc (size_t bytes)
{
  if (decrement_fail_alloc_counter ())
    {
      /* FAIL the malloc */
      return NULL;
    }

  if (bytes == 0) /* some system mallocs handle this, some don't */
    return NULL;
  else
    {
      void *mem;
      mem = xmlMemMalloc (bytes);

      if (mem)
        n_blocks_outstanding += 1;

      return mem;
    }
}

void*
test_realloc (void  *memory,
              size_t bytes)
{
  if (decrement_fail_alloc_counter ())
    {
      /* FAIL */
      return NULL;
    }

  if (bytes == 0) /* guarantee this is safe */
    {
      test_free (memory);
      return NULL;
    }
  else
    {
      void *mem;
      mem = xmlMemRealloc (memory, bytes);

      if (memory == NULL && mem != NULL)
        n_blocks_outstanding += 1;

      return mem;
    }
}

void
test_free (void  *memory)
{
  if (memory) /* we guarantee it's safe to free (NULL) */
    {
      n_blocks_outstanding -= 1;

      xmlMemFree (memory);
    }
}

char*
test_strdup (const char *str)
{
  int len;
  char *copy;

  if (str == NULL)
    return NULL;

  len = strlen (str);

  copy = test_malloc (len + 1);
  if (copy == NULL)
    return NULL;

  memcpy (copy, str, len + 1);

  return copy;
}

static int
run_failing_each_malloc (int                n_mallocs,
                         TestMemoryFunction func,
                         void              *data)
{
  n_mallocs += 10; /* fudge factor to ensure reallocs etc. are covered */

  while (n_mallocs >= 0)
    {
      set_fail_alloc_counter (n_mallocs);

      if (!(* func) (data))
        return FALSE;

      n_mallocs -= 1;
    }

  set_fail_alloc_counter (_TEST_INT_MAX);

  return TRUE;
}

/**
 * test_oom_handling:
 * @func: function to call
 * @data: data to pass to function
 *
 * Tests how well the given function responds to out-of-memory
 * situations. Calls the function repeatedly, failing a different
 * call to malloc() each time. If the function ever returns #FALSE,
 * the test fails. The function should return #TRUE whenever something
 * valid (such as returning an error, or succeeding) occurs, and #FALSE
 * if it gets confused in some way.
 *
 * Returns #TRUE if the function never returns FALSE
 */
int
test_oom_handling (TestMemoryFunction  func,
                   void               *data)
{
  int approx_mallocs;

  /* Run once to see about how many mallocs are involved */

  set_fail_alloc_counter (_TEST_INT_MAX);

  if (!(* func) (data))
    return FALSE;

  approx_mallocs = _TEST_INT_MAX - get_fail_alloc_counter ();

  set_fail_alloc_failures (1);
  if (!run_failing_each_malloc (approx_mallocs, func, data))
    return FALSE;

  set_fail_alloc_failures (2);
  if (!run_failing_each_malloc (approx_mallocs, func, data))
    return FALSE;

  set_fail_alloc_failures (3);
  if (!run_failing_each_malloc (approx_mallocs, func, data))
    return FALSE;

  set_fail_alloc_counter (_TEST_INT_MAX);

  return TRUE;
}
