/*
   recho -- really echo args, bracketed with <> and with invisible chars
	    made visible.

   Chet Ramey
   chet@po.cwru.edu
*/

/* Copyright (C) 2002-2005 Free Software Foundation, Inc.

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

#if defined (HAVE_CONFIG_H)
#  include  <config.h>
#endif

#include "bashansi.h"
#include <stdio.h>

void strprint();

int
main(argc, argv)
int	argc;
char	**argv;
{
	register int	i;

	for (i = 1; i < argc; i++) {
		printf("argv[%d] = <", i);
		strprint(argv[i]);
		printf(">\n");
	}
	exit(0);
}

void
strprint(str)
char	*str;
{
	register unsigned char *s;

	for (s = (unsigned char *)str; s && *s; s++) {
		if (*s < ' ') {
			putchar('^');
			putchar(*s+64);
		} else if (*s == 127) {
			putchar('^');
			putchar('?');
		} else
			putchar(*s);
	}
}
