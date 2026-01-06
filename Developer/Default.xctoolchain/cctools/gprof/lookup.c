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
/*	$OpenBSD: lookup.c,v 1.2 1996/06/26 05:33:53 deraadt Exp $	*/
/*	$NetBSD: lookup.c,v 1.5 1995/04/19 07:16:06 cgd Exp $	*/

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
#include "gprof.h"

/*
 * look up an address in a sorted-by-address namelist
 *    this deals with misses by mapping them to the next lower 
 *    entry point.
 */
nltype *
nllookup(
uint64_t address)
{
    int32_t low;
    int32_t middle;
    int32_t high;

#ifdef DEBUG
    int probes;

	probes = 0;
#endif
	for(low = 0, high = nname; low != high ; ){
#ifdef DEBUG
	    probes += 1;
#endif
	    middle = (high + low) >> 1;
	    if(nl[middle].value <= address && nl[middle+1].value > address){
#ifdef DEBUG
		if(debug & LOOKUPDEBUG){
		    printf("[nllookup] %d (%u) probes\n", probes, nname-1);
		}
#endif
		return(&nl[middle]);
	    }
	    if(nl[middle].value > address){
		high = middle;
	    }
	    else{
		low = middle + 1;
	    }
	}
	fprintf(stderr, "[nllookup] binary search fails for address 0x%x\n",
		(unsigned int)address );
	return(NULL);
}

arctype *
arclookup(
nltype *parentp,
nltype *childp)
{
    arctype *arcp;

	if(parentp == 0 || childp == 0){
	    printf("[arclookup] parentp == 0 || childp == 0\n");
	    return(NULL);
	}
#ifdef DEBUG
	if(debug & LOOKUPDEBUG){
	    printf("[arclookup] parent %s child %s\n",
		   parentp->name, childp->name);
	}
#endif
	for(arcp = parentp->children; arcp ; arcp = arcp->arc_childlist){
#ifdef DEBUG
	    if(debug & LOOKUPDEBUG){
		printf("[arclookup]\t arc_parent %s arc_child %s\n",
		       arcp->arc_parentp->name,
		       arcp->arc_childp->name);
	    }
#endif
	    if(arcp->arc_childp == childp){
		return(arcp);
	    }
	}
	return(NULL);
}
