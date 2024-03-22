/*
 * Copyright 2022 Yonggang Luo
 * SPDX-License-Identifier: MIT
 *
 * C11 <time.h> emulation library
 */

#ifndef C11_TIME_H_INCLUDED_
#define C11_TIME_H_INCLUDED_

#include <time.h>

/*---------------------------- macros ---------------------------*/

/* Refer to https://htmlpreview.github.io/?https://icube-forge.unistra.fr/icps/c23-library/-/raw/main/README.html#time_monotonic-time_active-time_thread_active */
#if defined(TIME_UTC) && \
   defined(TIME_MONOTONIC) && \
   defined(TIME_ACTIVE) && \
   defined(TIME_THREAD_ACTIVE) && \
   defined(TIME_MONOTONIC_RAW)
/* all needed time base is implemented */
#else
#define _TIMESPEC_GET_NEED_IMPL
#endif

#ifdef _TIMESPEC_GET_NEED_IMPL
#undef TIME_UTC
#undef TIME_MONOTONIC
#undef TIME_ACTIVE
#undef TIME_THREAD_ACTIVE
#undef TIME_MONOTONIC_RAW
/* c11 */
#define TIME_UTC 1
/* c23 */
#define TIME_MONOTONIC 2
#define TIME_ACTIVE 3
#define TIME_THREAD_ACTIVE 4
#define TIME_MONOTONIC_RAW 5
#define timespec_get c23_timespec_get
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------- types ----------------------------*/

/*
 * On MINGW `struct timespec` present but `timespec_get` may not present;
 * On MSVC `struct timespec` and `timespec_get` present at the same time;
 * So detecting `HAVE_STRUCT_TIMESPEC` in meson script dynamically.
 */
#ifndef HAVE_STRUCT_TIMESPEC
struct timespec
{
    time_t tv_sec;  // Seconds - >= 0
    long   tv_nsec; // Nanoseconds - [0, 999999999]
};
#endif

/*-------------------------- functions --------------------------*/

#if defined(_TIMESPEC_GET_NEED_IMPL)
#define _TIMESPEC_GET_NEED_DECL
#elif defined(__APPLE__) && defined(__cplusplus) && (__cplusplus < 201703L)
/* On macOS, the guard for declaration of timespec_get is by
 * (defined(__cplusplus) && __cplusplus >= 201703L),
 * fix the declaration for C++14 and lower here
 */
#define _TIMESPEC_GET_NEED_DECL
#endif

#ifdef _TIMESPEC_GET_NEED_DECL
/*-------------------- 7.25.7 Time functions --------------------*/
// 7.25.6.1
int
timespec_get(struct timespec *ts, int base);
#undef _TIMESPEC_GET_NEED_DECL
#endif

#ifdef __cplusplus
}
#endif

#endif /* C11_TIME_H_INCLUDED_ */
