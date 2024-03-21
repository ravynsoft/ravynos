/* zecho - bare-bones echo */

/* Copyright (C) 1996-2002 Free Software Foundation, Inc.

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

int
main(argc, argv)
int	argc;
char	**argv;
{
	argv++;

	while (*argv) {
		(void)printf("%s", *argv);
		if (*++argv)
			putchar(' ');
	}

	putchar('\n');
	exit(0);
}
