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
/*	$OpenBSD: dfn.c,v 1.2 1996/06/26 05:33:49 deraadt Exp $	*/
/*	$NetBSD: dfn.c,v 1.5 1995/04/19 07:15:56 cgd Exp $	*/

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
#include <stdio.h>
#include <stdlib.h>
#include "stuff/errors.h"
#include "gprof.h"

#define	DFN_DEPTH	1000
struct dfnstruct {
    nltype	*nlentryp;
    int		cycletop;
};
typedef struct dfnstruct dfntype;

static dfntype	dfn_stack[DFN_DEPTH] = { {0} };
static int dfn_depth = 0;

static int dfn_counter = DFN_NAN;

static void dfn_pre_visit(
    nltype *parentp);
static enum bool dfn_numbered(
    nltype *childp);
static enum bool dfn_busy(
    nltype *childp);
static void dfn_findcycle(
    nltype *childp);
static void dfn_self_cycle(
    nltype *parentp);
static void dfn_self_cycle(
    nltype *parentp);
static void dfn_post_visit(
    nltype *parentp);

/*
 * given this parent, depth first number its children.
 */
void
dfn(
nltype *parentp)
{
    arctype *arcp;

#ifdef DEBUG
	if(debug & DFNDEBUG){
	    printf("[dfn] dfn(");
	    printname(parentp);
	    printf(")\n");
	}
#endif
	/*
	 * if we're already numbered, no need to look any furthur.
	 */
	if(dfn_numbered(parentp)){
	    return;
	}
	/*
	 * if we're already busy, must be a cycle
	 */
	if(dfn_busy(parentp)){
	    dfn_findcycle(parentp);
	    return;
	}
	/*
	 * visit yourself before your children
	 */
	dfn_pre_visit(parentp);
	/*
	 * visit children
	 */
	for(arcp = parentp->children; arcp ; arcp = arcp->arc_childlist){
	    dfn(arcp->arc_childp);
	}
	/*
	 * visit yourself after your children
	 */
	dfn_post_visit(parentp);
}

/*
 * push a parent onto the stack and mark it busy
 */
static
void
dfn_pre_visit(
nltype *parentp)
{
	dfn_depth += 1;
	if(dfn_depth >= DFN_DEPTH){
	    fprintf(stderr, "[dfn] out of my depth (dfn_stack overflow)\n");
	    exit(1);
	}
	dfn_stack[dfn_depth].nlentryp = parentp;
	dfn_stack[dfn_depth].cycletop = dfn_depth;
	parentp->toporder = DFN_BUSY;
#ifdef DEBUG
	if(debug & DFNDEBUG){
	    printf("[dfn_pre_visit]\t\t%d:", dfn_depth);
	    printname(parentp);
	    printf("\n");
	}
#endif
}

/*
 * are we already numbered?
 */
static
enum bool
dfn_numbered(
nltype *childp)
{
    
	return(childp->toporder != DFN_NAN && childp->toporder != DFN_BUSY);
}

/*
 * are we already busy?
 */
static
enum bool
dfn_busy(
nltype *childp)
{

    if(childp->toporder == DFN_NAN){
	return(FALSE);
    }
    return(TRUE);
}

/*
 * MISSING: an explanation
 */
static
void
dfn_findcycle(
nltype *childp)
{
    int cycletop;
    nltype *cycleheadp;
    nltype *tailp;
    int index;

	cycleheadp = NULL;
	for(cycletop = dfn_depth; cycletop > 0; cycletop -= 1){
	    cycleheadp = dfn_stack[cycletop].nlentryp;
	    if(childp == cycleheadp){
		break;
	    }
	    if(childp->cyclehead != childp && childp->cyclehead == cycleheadp){
		break;
	    }
	}
	if(cycletop <= 0){
	    fatal("[dfn_findcycle] couldn't find head of cycle");
	}
#ifdef DEBUG
	if(debug & DFNDEBUG){
	    printf("[dfn_findcycle] dfn_depth %d cycletop %d ",
		   dfn_depth, cycletop);
	    printname(cycleheadp);
	    printf("\n");
	}
#endif
	if(cycletop == dfn_depth){
	    /*
	     * this is previous function, e.g. this calls itself
	     * sort of boring
	     */
	    dfn_self_cycle(childp);
	}
	else{
	    /*
	     * glom intervening functions that aren't already
	     * glommed into this cycle.
	     * things have been glommed when their cyclehead field
	     * points to the head of the cycle they are glommed into.
	     */
	    for(tailp = cycleheadp; tailp->cnext; tailp = tailp->cnext){
		/* void: chase down to tail of things already glommed */
#ifdef DEBUG
		if(debug & DFNDEBUG){
		    printf("[dfn_findcycle] tail ");
		    printname(tailp);
		    printf("\n");
		}
#endif /* DEBUG */
	    }
	    /*
	     * if what we think is the top of the cycle
	     * has a cyclehead field, then it's not really the
	     * head of the cycle, which is really what we want
	     */	
	    if(cycleheadp->cyclehead != cycleheadp){
		cycleheadp = cycleheadp->cyclehead;
#ifdef DEBUG
		if(debug & DFNDEBUG){
		    printf("[dfn_findcycle] new cyclehead ");
		    printname(cycleheadp);
		    printf("\n");
		}
#endif
	    }
	    for(index = cycletop + 1; index <= dfn_depth; index += 1){
		childp = dfn_stack[index].nlentryp;
		if(childp->cyclehead == childp){
		    /*
		     * not yet glommed anywhere, glom it
		     * and fix any children it has glommed
		     */
		    tailp -> cnext = childp;
		    childp -> cyclehead = cycleheadp;
#ifdef DEBUG
		    if(debug & DFNDEBUG){
			printf("[dfn_findcycle] glomming ");
			printname(childp);
			printf(" onto ");
			printname(cycleheadp);
			printf("\n");
		    }
#endif
		    for(tailp = childp; tailp->cnext; tailp = tailp->cnext){
			tailp->cnext->cyclehead = cycleheadp;
#ifdef DEBUG
			if(debug & DFNDEBUG){
			    printf("[dfn_findcycle] and its tail ");
			    printname(tailp->cnext);
			    printf(" onto ");
			    printname(cycleheadp);
			    printf("\n");
			}
#endif
		    }
		}
		else if(childp->cyclehead != cycleheadp /* firewall */ ){
		    fprintf(stderr,
			    "[dfn_busy] glommed, but not to cyclehead\n");
		}
	    }
	}
}

/*
 * deal with self-cycles
 * for lint: ARGSUSED
 */
static
void
dfn_self_cycle(
nltype *parentp)
{
	/*
	 * since we are taking out self-cycles elsewhere
	 * no need for the special case, here.
	 */
#ifdef DEBUG
	if(debug & DFNDEBUG){
	    printf("[dfn_self_cycle] ");
	    printname(parentp);
	    printf("\n");
	}
#endif
}

/*
 * visit a node after all its children
 * [MISSING: an explanation]
 * and pop it off the stack
 */
static
void
dfn_post_visit(
nltype *parentp)
{
    nltype *memberp;

#ifdef DEBUG
	if(debug & DFNDEBUG){
	    printf("[dfn_post_visit]\t%d: ", dfn_depth);
	    printname(parentp);
	    printf("\n");
	}
#endif
	/*
	 * number functions and things in their cycles
	 * unless the function is itself part of a cycle
	 */
	if(parentp->cyclehead == parentp){
	    dfn_counter += 1;
	    for(memberp = parentp; memberp; memberp = memberp->cnext){
		memberp->toporder = dfn_counter;
#ifdef DEBUG
		if(debug & DFNDEBUG){
		    printf("[dfn_post_visit]\t\tmember ");
		    printname(memberp);
		    printf(" -> toporder = %d\n", dfn_counter);
		}
#endif
	    }
	}
	else{
#ifdef DEBUG
	    if(debug & DFNDEBUG){
		printf("[dfn_post_visit]\t\tis part of a cycle\n");
	    }
#endif
	}
	dfn_depth -= 1;
}
