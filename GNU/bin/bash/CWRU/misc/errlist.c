/*
 * If necessary, link with lib/sh/libsh.a
 */

/* Copyright (C) 1998-2009 Free Software Foundation, Inc.

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

#include <stdio.h>
#include <errno.h>

extern char *strerror();

extern int sys_nerr;

int
main(c, v)
int	c;
char	**v;
{
	int	i, n;

	if (c == 1) {
		for (i = 1; i < sys_nerr; i++)
			printf("%d --> %s\n", i, strerror(i));
	} else {
		for (i = 1; i < c; i++) {
			n = atoi(v[i]);
			printf("%d --> %s\n", n, strerror(n));
		}
	}
	exit (0);
}

programming_error(a, b)
char	*a;
int	b;
{
}

fatal_error()
{
}
