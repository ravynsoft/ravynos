/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2019 Robert Manner <robert.manner@oneidentity.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef SUDO_PLUGIN_PYHELPERS_CPYCHECKER_H
#define	SUDO_PLUGIN_PYHELPERS_CPYCHECKER_H

/* Helper macros for cpychecker */

#if defined(WITH_CPYCHECKER_RETURNS_BORROWED_REF_ATTRIBUTE)
    #define CPYCHECKER_RETURNS_BORROWED_REF \
        __attribute__((cpychecker_returns_borrowed_ref))
#else
    #define CPYCHECKER_RETURNS_BORROWED_REF
#endif

#ifdef WITH_CPYCHECKER_NEGATIVE_RESULT_SETS_EXCEPTION_ATTRIBUTE
    #define CPYCHECKER_NEGATIVE_RESULT_SETS_EXCEPTION \
        __attribute__ ((cpychecker_negative_result_sets_exception))
#else
    #define CPYCHECKER_NEGATIVE_RESULT_SETS_EXCEPTION
#endif

#if defined(WITH_CPYCHECKER_STEALS_REFERENCE_TO_ARG_ATTRIBUTE)
  #define CPYCHECKER_STEALS_REFERENCE_TO_ARG(n) \
   __attribute__((cpychecker_steals_reference_to_arg(n)))
#else
 #define CPYCHECKER_STEALS_REFERENCE_TO_ARG(n)
#endif

#endif // SUDO_PLUGIN_PYHELPERS_CPYCHECKER_H
