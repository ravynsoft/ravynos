/* open-files -- report files a process has open */

/* Copyright (C) 1989-2009 Free Software Foundation, Inc.

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

#include <sys/types.h>
#include <fcntl.h>
#include <sys/file.h>

#include <unistd.h>
#include <stdlib.h>

#include <stdio.h>

main()
{
	register int	i;

	for (i = 0; i < getdtablesize(); i++) {
		if (fcntl(i, F_GETFD, 0) != -1)
			fprintf(stderr, "fd %d: open\n", i);
	}
	exit(0);
}
