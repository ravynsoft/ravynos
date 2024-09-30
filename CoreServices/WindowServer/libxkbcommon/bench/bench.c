/*
 * Copyright © 2015 Kazunobu Kuriyama <kazunobu.kuriyama@nifty.com>
 *                  Ran Benita <ran234@gmail.com>
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
 */

#include "config.h"

#include <assert.h>
#include <stdio.h>

#include "bench.h"
#include "../src/utils.h"

#ifndef _WIN32
#include <sys/time.h>
#else
#include <windows.h>
#include <stdint.h>

struct timeval {
    long tv_sec, tv_usec;
};

static int
gettimeofday(struct timeval *tv, void *unused)
{
    static const uint64_t EPOCH = ((uint64_t) 116444736000000000ULL);

    SYSTEMTIME system_time;
    FILETIME file_time;
    uint64_t t;

    GetSystemTime(&system_time);
    SystemTimeToFileTime(&system_time, &file_time);
    t = (uint64_t) file_time.dwLowDateTime;
    t += ((uint64_t) file_time.dwHighDateTime) << 32;

    tv->tv_sec  = (long) ((t - EPOCH) / 10000000L);
    tv->tv_usec = (long) (system_time.wMilliseconds * 1000);
    return 0;
}
#endif

void
bench_start(struct bench *bench)
{
    struct timeval val;
    (void) gettimeofday(&val, NULL);
    bench->start = (struct bench_time) {
        .seconds = val.tv_sec,
        .microseconds = val.tv_usec,
    };
}

void
bench_stop(struct bench *bench)
{
    struct timeval val;
    (void) gettimeofday(&val, NULL);
    bench->stop = (struct bench_time) {
        .seconds = val.tv_sec,
        .microseconds = val.tv_usec,
    };
}

void
bench_elapsed(const struct bench *bench, struct bench_time *result)
{
    result->seconds = bench->stop.seconds - bench->start.seconds;
    result->microseconds = bench->stop.microseconds - bench->start.microseconds;
    if (result->microseconds < 0) {
        result->microseconds += 1000000;
        result->seconds--;
    }
}

char *
bench_elapsed_str(const struct bench *bench)
{
    struct bench_time elapsed;
    char *buf;
    int ret;

    bench_elapsed(bench, &elapsed);
    ret = asprintf(&buf, "%ld.%06ld", elapsed.seconds, elapsed.microseconds);
    assert(ret >= 0);

    return buf;
}
