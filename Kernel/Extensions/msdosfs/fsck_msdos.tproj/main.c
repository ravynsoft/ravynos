/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
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
/*
 * Copyright (C) 1995 Wolfgang Solfrank
 * Copyright (c) 1995 Martin Husemann
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
 *	This product includes software developed by Martin Husemann
 *	and Wolfgang Solfrank.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <sys/cdefs.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#if __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#include "fsutil.h"
#include "ext.h"

int alwaysno;		/* assume "no" for all questions */
int alwaysyes;		/* assume "yes" for all questions */
int preen;		/* set when preening */
int quick;		/* set to quickly check if volume is dirty */
int quiet;		/* set to supress most messages */
int rdonly;		/* device is opened read only (supersedes above) */
size_t maxmem;	/* If non-zero, limit major allocations to this many bytes */

static void usage __P((void));
int main __P((int, char **));

static void
usage()
{
	errexit("Usage: fsck_msdos [-fnpqy] [-M <integer>[k|K|m|M]] filesystem ... \n");
}

int
main(argc, argv)
	int argc;
	char **argv;
{
	int ret = 0, erg;
	int ch;
	int offset;

	/*
	 * Allow default maximum memory to be specified at compile time.
	 */
	#ifdef FSCK_MAX_MEM
	maxmem = FSCK_MAX_MEM;
	#endif

	while ((ch = getopt(argc, argv, "pynfqM:")) != -1) {
		switch (ch) {
		case 'f':
			/*
			 * We are always forced, since we don't
			 * have a clean flag
			 */
			break;
		case 'n':
			alwaysno = 1;
			alwaysyes = preen = 0;
			break;
		case 'y':
			alwaysyes = 1;
			alwaysno = preen = 0;
			break;

		case 'p':
			preen = 1;
			alwaysyes = alwaysno = 0;
			break;

		case 'q':
			quick = 1;
			break;
		case 'M':
			if (sscanf(optarg, "%zi%n", &maxmem, &offset) == 0)
			{
				pfatal("Size argument '%s' not recognized\n", optarg);
				usage();
			}
			switch (optarg[offset])
			{
				case 'M':
				case 'm':
					maxmem *= 1024;
					/* Fall through */
				case 'K':
				case 'k':
					maxmem *= 1024;
					if (optarg[offset+1])
						goto bad_multiplier;
					break;
				case '\0':
					break;
				default:
bad_multiplier:
					pfatal("Size multiplier '%s' not recognized\n", optarg+offset);
					usage();
			}
			break;
		default:
			pfatal("Option '%c' not recognized\n", optopt);
			usage();
			break;
		}
	}
	argc -= optind;
	argv += optind;

	if (!argc)
		usage();

	/*
	 * Realistically, this does not work well with multiple filesystems.
	 */
	while (--argc >= 0) {
		setcdevname(*argv);
		erg = checkfilesys(*argv++);
		if (erg > ret)
			ret = erg;
	}

	return ret;
}


/*VARARGS*/
int
#if __STDC__
ask(int def, const char *fmt, ...)
#else
ask(def, fmt, va_alist)
	int def;
	char *fmt;
	va_dcl
#endif
{
	va_list ap;

	char prompt[256];
	int c;

	if (preen) {
		if (rdonly)
			def = 0;
		if (def)
			printf("FIXED\n");
		return def;
	}

#if __STDC__
	va_start(ap, fmt);
#else
	va_start(ap);
#endif
	vsnprintf(prompt, sizeof(prompt), fmt, ap);

	if (alwaysyes || rdonly) {
		if (!quiet)
			printf("%s? %s\n", prompt, rdonly ? "no" : "yes");
		return !rdonly;
	}
	do {
		printf("%s? [yn] ", prompt);
		fflush(stdout);
		c = getchar();
		while (c != '\n' && getchar() != '\n')
			if (feof(stdin))
				return 0;
	} while (c != 'y' && c != 'Y' && c != 'n' && c != 'N');
	return c == 'y' || c == 'Y';
}
