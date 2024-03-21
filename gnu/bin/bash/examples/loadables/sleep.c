/*
 * sleep -- sleep for fractions of a second
 *
 * usage: sleep seconds[.fraction]
 *
 * as an extension, we support the GNU time interval format (2m20s)
 */

/*
   Copyright (C) 1999-2021 Free Software Foundation, Inc.

   This file is part of GNU Bash.
   Bash is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Bash is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "config.h"

#include "bashtypes.h"

#if defined (TIME_WITH_SYS_TIME)
#  include <sys/time.h>
#  include <time.h>
#else
#  if defined (HAVE_SYS_TIME_H)
#    include <sys/time.h>
#  else
#    include <time.h>
#  endif
#endif

#if defined (HAVE_UNISTD_H)
#include <unistd.h>
#endif

#include <stdio.h>
#include "chartypes.h"

#include "loadables.h"

#define S_SEC	1
#define S_MIN	(60*S_SEC)
#define S_HOUR	(60*S_MIN)
#define S_DAY	(24*S_HOUR)

static int
parse_gnutimefmt (char *string, long *sp, long *up)
{
	int	c, r;
	char	*s, *ep;
	long	tsec, tusec, accumsec, accumusec, t;
	int	mult;

	tsec = tusec = 0;
	accumsec = accumusec = 0;
	mult = 1;

	for (s = string; s && *s; s++) {
	    	r = uconvert(s, &accumsec, &accumusec, &ep);
		if (r == 0 && *ep == 0)
			return r;
		c = *ep;
		mult = 1;
		switch (c) {
			case '\0':
			case 's':
				mult = S_SEC;
				break;
			case 'm':
				mult = S_MIN;
				break;
			case 'h':
				mult = S_HOUR;
				break;
			case 'd':
				mult = S_DAY;
				break;
			default:
				return 0;
		}

		/* multiply the accumulated value by the multiplier */
		t = accumusec * mult;
		accumsec = accumsec * mult + (t / 1000000);
		accumusec = t % 1000000;

		/* add to running total */
		tsec += accumsec;
		tusec += accumusec;
		if (tusec >= 1000000) {
			tsec++;
			tusec -= 1000000;
		}

		/* reset and continue */
		accumsec = accumusec = 0;
		mult = 1;
		if (c == 0)
			break;
		s = ep;
	}

	if (sp)
		*sp = tsec;
	if (up)
		*up = tusec;

	return 1;
}

int
sleep_builtin (WORD_LIST *list)
{
	long	sec, usec;
	char	*ep;
	int	r, mul;
	time_t	t;

	if (list == 0) {
		builtin_usage();
		return(EX_USAGE);
	}

	/* Skip over `--' */
	if (list->word && ISOPTION (list->word->word, '-'))
		list = list->next;

	if (*list->word->word == '-' || list->next) {
		builtin_usage ();
		return (EX_USAGE);
	}

    	r = uconvert(list->word->word, &sec, &usec, &ep);
	/*
	 * Maybe postprocess conversion failures here based on EP
	 *
	 * A heuristic: if the conversion failed, but the argument appears to
	 * contain a GNU-like interval specifier (e.g. "1m30s"), try to parse
	 * it. If we can't, return the right exit code to tell
	 * execute_builtin to try and execute a disk command instead.
	 */
	if (r == 0 && (strchr ("dhms", *ep) || strpbrk (list->word->word, "dhms")))
		r = parse_gnutimefmt (list->word->word, &sec, &usec);
    		
	if (r) {
		fsleep(sec, usec);
		QUIT;
		return(EXECUTION_SUCCESS);
	}
	builtin_error("%s: bad sleep interval", list->word->word);
	return (EXECUTION_FAILURE);
}

static char *sleep_doc[] = {
	"Suspend execution for specified period.",
	""
	"sleep suspends execution for a minimum of SECONDS[.FRACTION] seconds.",
	"As an extension, sleep accepts GNU-style time intervals (e.g., 2m30s).",
	(char *)NULL
};

struct builtin sleep_struct = {
	"sleep",
	sleep_builtin,
	BUILTIN_ENABLED,
	sleep_doc,
	"sleep seconds[.fraction]",
	0
};
