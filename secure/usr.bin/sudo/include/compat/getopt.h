/*	$OpenBSD: getopt.h,v 1.2 2008/06/26 05:42:04 ray Exp $	*/
/*	$NetBSD: getopt.h,v 1.4 2000/07/07 10:43:54 ad Exp $	*/
/*	$FreeBSD: head/include/getopt.h 203963 2010-02-16 19:28:10Z imp $ */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2000 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Dieter Baron and Thomas Klausner.
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

#ifndef COMPAT_GETOPT_H
#define COMPAT_GETOPT_H

/*
 * GNU-like getopt_long()/getopt_long_only() with 4.4BSD optreset extension.
 */
#define no_argument        0
#define required_argument  1
#define optional_argument  2

struct option {
	/* name of long option */
	const char *name;
	/*
	 * one of no_argument, required_argument, and optional_argument:
	 * whether option takes an argument
	 */
	int has_arg;
	/* if not NULL, set *flag to val when option found */
	int *flag;
	/* if flag not NULL, value to set *flag to; else return value */
	int val;
};

sudo_dso_public int sudo_getopt_long(int, char * const *, const char *,
	    const struct option *, int *);
#undef getopt_long
#define getopt_long(_a, _b, _c, _d, _e) \
	sudo_getopt_long((_a), (_b), (_c), (_d), (_e))

sudo_dso_public int sudo_getopt_long_only(int, char * const *, const char *,
	    const struct option *, int *);
#undef getopt_long_only
#define getopt_long_only(_a, _b, _c, _d, _e) \
	sudo_getopt_long_only((_a), (_b), (_c), (_d), (_e))
#if 0
sudo_dso_public int sudo_getopt(int, char * const [], const char *);
#undef getopt
#define getopt(_a, _b, _c)	sudo_getopt((_a), (_b), (_c))
#endif

extern	char *optarg;			/* getopt(3) external variables */
extern	int opterr;
extern	int optind;
extern	int optopt;
extern	int optreset;
 
#endif /* !COMPAT_GETOPT_H */
