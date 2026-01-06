/* xmalloc.c - get memory or bust
   Copyright (C) 1987 Free Software Foundation, Inc.

This file is part of GAS, the GNU Assembler.

GAS is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

GAS is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GAS; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include <stdlib.h>
#include "xmalloc.h"
#include "messages.h"

void *
xmalloc(
size_t n)
{
    void *retval;

	if(!(retval = malloc((unsigned)n))){
	    as_fatal("virtual memory exceeded");
	}
	return(retval);
}

void *
xrealloc(
void *ptr,
size_t n)
{
    if((ptr = realloc(ptr, (unsigned)n)) == 0)
	as_fatal("virtual memory exceeded");
    return(ptr);
}
