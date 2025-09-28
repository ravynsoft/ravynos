/* sigs - print signal dispositions for a process */

/* Copyright (C) 1990-2010 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

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

#include <signal.h>
#include <stdio.h>

extern const char	* const sys_siglist[];

typedef void	sighandler();

main(argc, argv)
int	argc;
char	**argv;
{
	register int	i;
	sighandler	*h;

	for (i = 1; i < NSIG; i++) {
		h = signal(i, SIG_DFL);
		if (h != SIG_DFL) {
			if (h == SIG_IGN)
				fprintf(stderr, "%d: ignored (%s)\n", i, sys_siglist[i]);
			else
				fprintf(stderr, "%d: caught (%s)\n", i, sys_siglist[i]);
		}
	}
	exit(0);
}

		
