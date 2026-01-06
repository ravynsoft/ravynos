/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*	$OpenBSD: ar.c,v 1.3 1997/01/15 23:42:11 millert Exp $	*/
/*	$NetBSD: ar.c,v 1.5 1995/03/26 03:27:44 glass Exp $	*/

/*-
 * Copyright (c) 1990, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Hugh Smith at The University of Guelph.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
#if 0
static char copyright[] =
"@(#) Copyright (c) 1990, 1993, 1994\n\
	The Regents of the University of California.  All rights reserved.\n";
static char sccsid[] = "@(#)ar.c	8.3 (Berkeley) 4/2/94";
static char rcsid[] = "$OpenBSD: ar.c,v 1.3 1997/01/15 23:42:11 millert Exp $";
#endif
#endif /* not lint */

#include <sys/param.h>

#include <ar.h>
#include <dirent.h>
#include <err.h>
#include <paths.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "archive.h"
#include "extern.h"
#include "stuff/execute.h"
#include "stuff/unix_standard_mode.h"

CHDR chdr;
u_int options;
char *archive, *envtmp, *posarg, *posname;
static void badoptions __P((char *));
static void usage __P((void));
char *progname;

/*
 * main --
 *	main basically uses getopt to parse options and calls the appropriate
 *	functions.  Some hacks that let us be backward compatible with 4.3 ar
 *	option parsing and sanity checking.
 */
int
main(argc, argv)
	int argc;
	char **argv;
{
	int c, retval, verbose, run_ranlib, toc64;
	char *p;
	int (*fcall) __P((char **));

	fcall = 0;
	verbose = 0;
	toc64 = 0;
	progname = argv[0];
	run_ranlib = 1;

	if (argc < 3)
		usage();

	/*
	 * Historic versions didn't require a '-' in front of the options.
	 * Fix it, if necessary.
	*/
	if (*argv[1] != '-') {
		if (!(p = malloc((u_int)(strlen(argv[1]) + 2))))
			err(1, NULL);
		*p = '-';
		(void)strcpy(p + 1, argv[1]);
		argv[1] = p;
	}

	/*
	 * For Rhapsody Premier the option to use long member names, -L, is the
	 * default.  The compiler tools for Rhapsody Premier do understand
	 * extended format #1.  The new option -L allows ar to use the extended 
	 * format and the old -T option causes the truncation of names.
	 */
	while ((c = getopt(argc, argv, "abcdilLmopqrSsTtuVvx6")) != -1) {
		switch(c) {
		case 'a':
			options |= AR_A;
			break;
		case 'b':
		case 'i':
			options |= AR_B;
			break;
		case 'c':
			options |= AR_C;
			break;
		case 'd':
			options |= AR_D;
			fcall = delete;
			break;
		case 'l':		/* not documented, compatibility only */
			envtmp = ".";
			break;
		case 'L':
			options &= ~AR_TR;
			break;
		case 'm':
			options |= AR_M;
			fcall = move;
			break;
		case 'o':
			options |= AR_O;
			break;
		case 'p':
			options |= AR_P;
			fcall = print;
			break;
		case 'q':
			options |= AR_Q;
			fcall = append;
			break;
		case 'r':
			options |= AR_R;
			fcall = replace;
			break;
		case 's':
			options |= AR_S;
			break;
		case 'S':
			options &= ~AR_S;
			run_ranlib = 0;
			break;
		case 'T':
			options |= AR_TR;
			break;
		case 't':
			options |= AR_T;
			fcall = contents;
			break;
		case 'u':
			options |= AR_U;
			break;
		case 'V':
			verbose = 1;
			break;
		case '6':
			toc64 = 1;
			break;
		case 'v':
			options |= AR_V;
			break;
		case 'x':
			options |= AR_X;
			fcall = extract;
			break;
		default:
			usage();
		}
	}

	argv += optind;
	argc -= optind;

	/* One of -dmpqrtsx required. */
	if (!(options & (AR_D|AR_M|AR_P|AR_Q|AR_R|AR_S|AR_T|AR_X))) {
		warnx("one of options -dmpqrtsx is required");
		usage();
	}
	/* Only one of -a and -bi allowed. */
	if (options & AR_A && options & AR_B) {
		warnx("only one of -a and -[bi] options allowed");
		usage();
	}
	/* -ab require a position argument. */
	if (options & (AR_A|AR_B)) {
		if (!(posarg = *argv++)) {
			warnx("no position operand specified");
			usage();
		}
		posname = rname(posarg);
	}
	/* -d only valid with -Tsv. */
	if (options & AR_D && options & ~(AR_D|AR_TR|AR_S|AR_V))
		badoptions("-d");
	/* -m only valid with -abiTsv. */
	if (options & AR_M && options & ~(AR_A|AR_B|AR_M|AR_TR|AR_S|AR_V))
		badoptions("-m");
	/* -p only valid with -Tsv. */
	if (options & AR_P && options & ~(AR_P|AR_TR|AR_S|AR_V))
		badoptions("-p");
	/* -q only valid with -cTsv. */
	if (options & AR_Q && options & ~(AR_C|AR_Q|AR_TR|AR_S|AR_V))
		badoptions("-q");
	/* -r only valid with -abcuTsv. */
	if (options & AR_R && options & ~(AR_A|AR_B|AR_C|AR_R|AR_U|AR_TR|AR_S|AR_V))
		badoptions("-r");
	/* -t only valid with -Tsv. */
	if (options & AR_T && options & ~(AR_T|AR_TR|AR_S|AR_V))
		badoptions("-t");
	/* -x only valid with -ouTsv. */
	if (options & AR_X && options & ~(AR_O|AR_U|AR_TR|AR_S|AR_V|AR_X))
		badoptions("-x");

	if (!(archive = *argv++)) {
		warnx("no archive specified");
		usage();
	}

	/* -dmqr require a list of archive elements. */
	if (options & (AR_D|AR_M|AR_Q|AR_R) && !*argv) {
		warnx("no archive members specified");
		usage();
	}

	if(fcall != 0){
	    retval = (*fcall)(argv);
	    if(retval != EXIT_SUCCESS ||
	       ((options & AR_S) != AR_S &&
	        (get_unix_standard_mode() == FALSE ||
		 archive_opened_for_writing == 0)))
		exit(retval);
	}

	/*
	 * The default is to run ranlib(1) for UNIX conformance.  But if the -S
	 * option is specified by the user we don't run it.
	 */
	if(run_ranlib){
	    /* run ranlib -f or -q on the archive */
	    reset_execute_list();
	    add_execute_list_with_prefix("ranlib");
	    if(options & AR_S)
		add_execute_list("-f");
	    else
		add_execute_list("-q");
	    if(toc64)
		add_execute_list("-toc64");
	    add_execute_list(archive);
	    if(execute_list(verbose) == 0){
		(void)fprintf(stderr, "%s: internal ranlib command failed\n",
			      progname);
		exit(EXIT_FAILURE);
	    }
	}
	exit(EXIT_SUCCESS);
}

static void
badoptions(arg)
	char *arg;
{

	warnx("illegal option combination for %s", arg);
	usage();
}

static void
usage()
{

	(void)fprintf(stderr, "usage:  ar -d [-TLsv] archive file ...\n");
	(void)fprintf(stderr, "\tar -m [-TLsv] archive file ...\n");
	(void)fprintf(stderr, "\tar -m [-abiTLsv] position archive file ...\n");
	(void)fprintf(stderr, "\tar -p [-TLsv] archive [file ...]\n");
	(void)fprintf(stderr, "\tar -q [-cTLsv] archive file ...\n");
	(void)fprintf(stderr, "\tar -r [-cuTLsv] archive file ...\n");
	(void)fprintf(stderr, "\tar -r [-abciuTLsv] position archive file ...\n");
	(void)fprintf(stderr, "\tar -t [-TLsv] archive [file ...]\n");
	(void)fprintf(stderr, "\tar -x [-ouTLsv] archive [file ...]\n");
	exit(1);
}	
