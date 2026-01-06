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
/*	$OpenBSD: printlist.c,v 1.2 1996/06/26 05:33:59 deraadt Exp $	*/
/*	$NetBSD: printlist.c,v 1.5 1995/04/19 07:16:23 cgd Exp $	*/

/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
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
#include <stdlib.h>
#include "stuff/errors.h"
#include "gprof.h"

/*
 * these are the lists of names:
 * there is the list head and then the listname
 * is a pointer to the list head
 * (for ease of passing to stringlist functions).
 */
struct stringlist	fhead = { 0 , 0 };
struct stringlist	*flist = &fhead;
struct stringlist	Fhead = { 0 , 0 };
struct stringlist	*Flist = &Fhead;
struct stringlist	ehead = { 0 , 0 };
struct stringlist	*elist = &ehead;
struct stringlist	Ehead = { 0 , 0 };
struct stringlist	*Elist = &Ehead;

void
addlist(
struct stringlist *listp,
char *funcname)
{
    struct stringlist *slp;

	slp = (struct stringlist *)malloc(sizeof(struct stringlist));
	if(slp == NULL)
	    fatal("ran out room for printlist");
	slp->next = listp->next;
	slp->string = funcname;
	listp->next = slp;
}

enum bool
onlist(
struct stringlist *listp,
char *funcname)
{
    struct stringlist *slp;

	for(slp = listp->next; slp; slp = slp->next){
	    if(!strcmp(slp->string, funcname)){
		return(TRUE);
	    }
	    if(funcname[0] == '_' && !strcmp(slp->string, &funcname[1])){
		return(TRUE);
	    }
	}
	return(FALSE);
}
