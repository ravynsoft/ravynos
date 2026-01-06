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
/*	$OpenBSD: printgprof.c,v 1.2 1996/06/26 05:33:59 deraadt Exp $	*/
/*	$NetBSD: printgprof.c,v 1.5 1995/04/19 07:16:21 cgd Exp $	*/

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
#include "gprof.h"
#include "stuff/errors.h"

static int timecmp(
    nltype **npp1,
    nltype **npp2);

static void flatprofheader(
    void);

static void flatprofline(
    nltype *np);

static void gprofheader(
    void);

static void gprofline(
    nltype *np);

static void printparents(
    nltype *childp);

static void printchildren(
    nltype *parentp);

static void sortchildren(
    nltype *parentp);

static void sortparents(
    nltype *childp);

static void printcycle(
    nltype *cyclep);

static void printmembers(
    nltype *cyclep);

static void sortmembers(
    nltype *cyclep);

static int membercmp(
    nltype *this,
    nltype *that);

static int arccmp(
    arctype *thisp,
    arctype *thatp);

static void printblurb(
    char *blurbname);

static int namecmp(
    nltype **npp1,
    nltype **npp2);

void
printprof(
void)
{
    nltype *np;
    nltype **sortednlp;
    uint32_t index;

	actime = 0.0;
	printf("\f\n");
	flatprofheader();
	/*
	 * Sort the symbol table in by time.
	 */
	sortednlp = (nltype **)calloc(nname, sizeof(nltype *));
	if(sortednlp == (nltype **)0)
	    fatal("[printprof] ran out of memory for time sorting");
	for(index = 0; index < nname; index += 1){
	    sortednlp[index] = &nl[index];
	}
	qsort(sortednlp, nname, sizeof(nltype *),
	      (int (*)(const void *, const void *))timecmp);
	for(index = 0; index < nname; index += 1){
	    np = sortednlp[index];
	    flatprofline(np);
	}
	actime = 0.0;
	free(sortednlp);
}

static
int
timecmp(
nltype **npp1,
nltype **npp2)
{
    double timediff;
    int32_t calldiff;

	timediff = (*npp2)->time - (*npp1)->time;
	if(timediff > 0.0)
	    return(1);
	if(timediff < 0.0)
	    return(-1);
	calldiff = (*npp2)->ncall - (*npp1)->ncall;
	if(calldiff > 0)
	    return(1);
	if(calldiff < 0)
	    return(-1);
	return(strcmp((*npp1)->name, (*npp2)->name));
}

/*
 * header for flatprofline
 */
static
void
flatprofheader(
void)
{
    
	if(bflag == TRUE){
	    printblurb(FLAT_BLURB);
	}
	printf("\ngranularity: each sample hit covers %lu byte(s)",
	       (int32_t)sample_sets->scale * sizeof(UNIT) );
	if(totime > 0.0){
	    printf(" for %.2f%% of %.2f seconds\n\n",
		   100.0/totime , totime / hz );
	}
	else{
	    printf(" no time accumulated\n\n");
	    /*
	     * this doesn't hurt since all the numerators will be zero.
	     */
	    totime = 1.0;
	}
	printf("%5.5s %10.10s %8.8s %8.8s %8.8s %8.8s  %-8.8s\n",
	       "%  ", "cumulative", "self  ", "", "self  ", "total ", "");
	printf("%5.5s %10.10s %8.8s %8.8s %8.8s %8.8s  %-8.8s\n",
	       "time", "seconds ", "seconds", "calls",
	       "ms/call", "ms/call", "name");
}

static
void
flatprofline(
nltype *np)
{
	if(zflag == FALSE && np->ncall == 0 && np->time == 0){
	    return;
	}
	actime += np->time;
	printf("%5.1f %10.2f %8.2f",
	       100 * np->time / totime, actime / hz, np->time / hz);
	if(np->ncall != 0){
	    printf(" %8d %8.2f %8.2f  ", np->ncall,
		   1000 * np->time / hz / np->ncall,
		   1000 * (np->time + np->childtime) / hz / np->ncall);
	}
	else{
	    printf(" %8.8s %8.8s %8.8s  ", "", "", "");
	}
	printname(np);
	printf("\n");
}

static
void
gprofheader(
void)
{
	if(bflag == TRUE){
	    printblurb(CALLG_BLURB);
	}
	printf("\ngranularity: each sample hit covers %lu byte(s)",
	       (int32_t)sample_sets->scale * sizeof(UNIT));
	if(printtime > 0.0){
	    printf(" for %.2f%% of %.2f seconds\n\n",
		   100.0 / printtime, printtime / hz);
	}
	else{
	    printf(" no time propagated\n\n");
	    /*
	     * this doesn't hurt, since all the numerators will be 0.0
	     */
	    printtime = 1.0;
	}
	printf("%6.6s %5.5s %7.7s %11.11s %7.7s/%-7.7s     %-8.8s\n",
	       "", "", "", "", "called", "total", "parents");
	printf("%-6.6s %5.5s %7.7s %11.11s %7.7s+%-7.7s %-8.8s\t%5.5s\n",
	       "index", "%time", "self", "descendents",
	       "called", "self", "name", "index");
	printf("%6.6s %5.5s %7.7s %11.11s %7.7s/%-7.7s     %-8.8s\n",
	       "", "", "", "", "called", "total", "children");
	printf("\n");
}

static
void
gprofline(
nltype *np)
{
    char kirkbuffer[BUFSIZ];

	sprintf(kirkbuffer, "[%d]", np->index);
	printf("%-6.6s %5.1f %7.2f %11.2f",
		kirkbuffer ,
		100 * ( np -> propself + np -> propchild ) / printtime ,
		np -> propself / hz ,
		np -> propchild / hz );
	if((np->ncall + np->selfcalls) != 0){
	    printf(" %7d", np->ncall);
	    if(np->selfcalls != 0)
		printf("+%-7d", np->selfcalls);
	    else
		printf(" %7.7s ", "");
	}
	else{
	    printf(" %7.7s %7.7s ", "", "");
	}
	printname(np);
	printf("\n");
}

void
printgprof(
nltype **timesortnlp)
{
    uint32_t index;
    nltype *parentp;

	/*
	 * Print out the structured profiling list
	 */
	gprofheader();
	for(index = 0; index < nname + ncycle; index ++){
	    parentp = timesortnlp[index];
	    if(zflag == FALSE &&
	       parentp->ncall == 0 &&
	       parentp->selfcalls == 0 &&
	       parentp->propself == 0 &&
	       parentp->propchild == 0 ){
		continue;
	    }
	    if(!parentp->printflag){
		continue;
	    }
	    if(parentp->name == 0 && parentp->cycleno != 0){
		/*
		 * cycle header
		 */
		printcycle(parentp);
		printmembers(parentp);
	    }
	    else{
		printparents(parentp);
		gprofline(parentp);
		printchildren(parentp);
	    }
	    printf("\n");
	    printf("-----------------------------------------------\n");
	    printf("\n");
	}
	free(timesortnlp);
}

/*
 * sort by decreasing propagated time
 * if times are equal, but one is a cycle header,
 * 	say that's first (e.g. less, i.e. -1).
 * if one's name doesn't have an underscore and the other does,
 * 	say the one is first.
 * all else being equal, sort by names.
 */
int
totalcmp(
nltype **npp1,
nltype **npp2)
{
    nltype *np1, *np2;
    double diff;

	np1 = *npp1;
	np2 = *npp2;
	diff =    (np1->propself + np1->propchild)
		- (np2->propself + np2->propchild);
	if(diff < 0.0)
	    return(1);
	if(diff > 0.0)
	    return(-1);
	if(np1->name == 0 && np1->cycleno != 0) 
	    return(-1);
	if(np2->name == 0 && np2->cycleno != 0)
	    return(1);
	if(np1->name == 0)
	    return(-1);
	if(np2->name == 0)
	    return(1);
	if(*(np1->name) != '_' && *(np2->name) == '_')
	    return(-1);
	if(*(np1->name) == '_' && *(np2->name) != '_')
	    return(1);
	if(np1->ncall > np2->ncall)
	    return(-1);
	if(np1->ncall < np2->ncall) 
	    return(1);
	return(strcmp(np1->name, np2->name));
}

static
void
printparents(
nltype *childp)
{
    nltype *parentp;
    arctype *arcp;
    nltype *cycleheadp;

	if(childp->cyclehead != 0){
	    cycleheadp = childp->cyclehead;
	}
	else{
	    cycleheadp = childp;
	}
	if(childp->parents == 0){
	    printf("%6.6s %5.5s %7.7s %11.11s %7.7s %7.7s     <spontaneous>\n",
		    "" , "" , "" , "" , "" , "" );
	    return;
	}
	sortparents(childp);
	for(arcp = childp->parents; arcp; arcp = arcp->arc_parentlist){
	    parentp = arcp->arc_parentp;
	    if(childp == parentp ||
	       (childp->cycleno != 0 && parentp->cycleno == childp->cycleno)){
		/*
		 * selfcall or call among siblings
		 */
		printf("%6.6s %5.5s %7.7s %11.11s %7u %7.7s     ",
		       "" , "" , "" , "", arcp->arc_count, "");
		printname(parentp);
		printf("\n");
	    }
	    else{
		/*
		 * regular parent of child
		 */
		printf("%6.6s %5.5s %7.2f %11.2f %7u/%-7d     ",
		       "" , "" ,
		       arcp->arc_time / hz, arcp->arc_childtime / hz,
		       arcp->arc_count, cycleheadp->ncall);
		printname(parentp);
		printf("\n");
	    }
	}
}

static
void
printchildren(
nltype *parentp)
{
    nltype *childp;
    arctype *arcp;

	sortchildren(parentp);
	arcp = parentp->children;
	for(arcp = parentp->children; arcp; arcp = arcp->arc_childlist){
	    childp = arcp->arc_childp;
	    if(childp == parentp ||
	       (childp->cycleno != 0 && childp->cycleno == parentp->cycleno)){
		/*
		 * self call or call to sibling
		 */
		printf("%6.6s %5.5s %7.7s %11.11s %7u %7.7s     ",
		       "", "", "", "", arcp->arc_count, "");
		printname(childp);
		printf("\n");
	    }
	    else{
		/*
		 * regular child of parent
		 */
		printf("%6.6s %5.5s %7.2f %11.2f %7u/%-7d     ",
		       "", "",
		       arcp->arc_time / hz, arcp->arc_childtime / hz,
		       arcp->arc_count, childp->cyclehead->ncall);
		printname(childp);
		printf("\n");
	    }
	}
}

void
printname(
nltype *selfp)
{

	if(selfp->name != NULL){
	    printf("%s", selfp->name);
#ifdef DEBUG
	    if(debug & DFNDEBUG){
		printf("{%d} ", selfp->toporder);
	    }
	    if(debug & PROPDEBUG){
		printf("%5.2f%% ", selfp->propfraction);
	    }
#endif
	}
	if(selfp->cycleno != 0){
	    printf(" <cycle %d>", selfp->cycleno);
	}
	if(selfp->index != 0){
	    if(selfp->printflag){
		printf(" [%d]", selfp->index );
	    }else{
		printf(" (%d)", selfp->index );
	    }
	}
}

static
void
sortchildren(
nltype *parentp)
{
    arctype *arcp;
    arctype *detachedp;
    arctype sorted;
    arctype *prevp;

	detachedp = NULL;
	/*
	 * unlink children from parent,
	 * then insertion sort back on to sorted's children.
	 *     *arcp	the arc you have detached and are inserting.
	 *     *detachedp	the rest of the arcs to be sorted.
	 *     sorted	arc list onto which you insertion sort.
	 *     *prevp	arc before the arc you are comparing.
	 */
	sorted.arc_childlist = 0;
#ifdef notdef
	for((arcp = parentp->children) && (detachedp = arcp->arc_childlist);
	    arcp;
	    (arcp = detachedp) && (detachedp = detachedp->arc_childlist)){
#endif
	if((arcp = parentp->children) != NULL)
	    detachedp = arcp->arc_childlist;
	while(arcp != NULL){
	    /*
	     * consider *arcp as disconnected
	     * insert it into sorted
	     */
	    for(prevp = &sorted;
		prevp->arc_childlist;
		prevp = prevp->arc_childlist){
		if(arccmp(arcp, prevp->arc_childlist) != LESSTHAN){
		    break;
		}
	    }
	    arcp->arc_childlist = prevp->arc_childlist;
	    prevp->arc_childlist = arcp;
	    if((arcp = detachedp) != NULL)
		detachedp = detachedp->arc_childlist;
	}
	/*
	 * reattach sorted children to parent
	 */
	parentp->children = sorted.arc_childlist;
}

static
void
sortparents(
nltype *childp)
{
    arctype *arcp;
    arctype *detachedp;
    arctype sorted;
    arctype *prevp;

	detachedp = NULL;
	/*
	 * unlink parents from child,
	 * then insertion sort back on to sorted's parents.
	 *     *arcp	the arc you have detached and are inserting.
	 *     *detachedp	the rest of the arcs to be sorted.
	 *     sorted	arc list onto which you insertion sort.
	 *     *prevp	arc before the arc you are comparing.
	 */
	sorted.arc_parentlist = 0;
#ifdef notdef
	for((arcp = childp->parents) && (detachedp = arcp->arc_parentlist);
	    arcp;
	    (arcp = detachedp) && (detachedp = detachedp->arc_parentlist)){
#endif
	if((arcp = childp->parents) != NULL)
	    detachedp = arcp->arc_parentlist;
	while(arcp != NULL){
	    /*
	     * consider *arcp as disconnected
	     * insert it into sorted
	     */
	    for(prevp = &sorted;
		prevp->arc_parentlist;
		prevp = prevp->arc_parentlist){
		if(arccmp(arcp, prevp->arc_parentlist) != GREATERTHAN){
		    break;
		}
	    }
	    arcp->arc_parentlist = prevp->arc_parentlist;
	    prevp->arc_parentlist = arcp;
	    if((arcp = detachedp) != NULL)
		detachedp = detachedp->arc_parentlist;
	}
	/*
	 * reattach sorted arcs to child
	 */
	childp->parents = sorted.arc_parentlist;
}

/*
 * print a cycle header
 */
static
void
printcycle(
nltype *cyclep)
{
    char kirkbuffer[BUFSIZ];

	sprintf(kirkbuffer, "[%d]", cyclep->index);
	printf("%-6.6s %5.1f %7.2f %11.2f %7d",
	       kirkbuffer,
	       100 * (cyclep->propself + cyclep->propchild) / printtime,
	       cyclep->propself / hz,
	       cyclep->propchild / hz,
	       cyclep->ncall);
	if(cyclep->selfcalls != 0){
	    printf("+%-7d", cyclep->selfcalls);
	}
	else{
	    printf(" %7.7s", "");
	}
	printf(" <cycle %d as a whole>\t[%d]\n",
	       cyclep->cycleno, cyclep->index);
}

/*
 * print the members of a cycle
 */
static
void
printmembers(
nltype *cyclep)
{
    nltype *memberp;

	sortmembers(cyclep);
	for(memberp = cyclep->cnext; memberp; memberp = memberp->cnext){
	    printf("%6.6s %5.5s %7.2f %11.2f %7d", 
		   "", "", memberp->propself / hz, memberp->propchild / hz,
		   memberp->ncall);
	    if(memberp->selfcalls != 0){
		printf("+%-7d", memberp->selfcalls);
	    }
	    else{
		printf(" %7.7s", "");
	    }
	    printf("     ");
	    printname(memberp);
	    printf("\n");
	}
}

/*
 * sort members of a cycle
 */
static
void
sortmembers(
nltype *cyclep)
{
    nltype *todo;
    nltype *doing;
    nltype *prev;

	/*
	 *	detach cycle members from cyclehead,
	 *	and insertion sort them back on.
	 */
	todo = cyclep->cnext;
	cyclep->cnext = 0;
#ifdef notdef
	for((doing = todo) && (todo = doing->cnext);
	    doing;
	    (doing = todo) && (todo = doing->cnext)){
#endif
	if((doing = todo) != NULL)
	    todo = doing->cnext;
	while(doing != NULL){
	    for(prev = cyclep; prev->cnext; prev = prev->cnext){
		if(membercmp(doing, prev->cnext) == GREATERTHAN){
		    break;
		}
	    }
	    doing->cnext = prev->cnext;
	    prev->cnext = doing;
	    if((doing = todo) != NULL)
		todo = doing->cnext;
	}
}

/*
 * major sort is on propself + propchild,
 * next is sort on ncalls + selfcalls.
 */
static
int
membercmp(
nltype *this,
nltype *that)
{
    double thistime, thattime;
    int32_t thiscalls, thatcalls;

    thistime = this->propself + this->propchild;
    thattime = that->propself + that->propchild;
    thiscalls = this->ncall + this->selfcalls;
    thatcalls = that->ncall + that->selfcalls;

    if(thistime > thattime){
	return(GREATERTHAN);
    }
    if(thistime < thattime){
	return(LESSTHAN);
    }
    if(thiscalls > thatcalls){
	return(GREATERTHAN);
    }
    if(thiscalls < thatcalls){
	return(LESSTHAN);
    }
    return(EQUALTO);
}
/*
 * compare two arcs to/from the same child/parent.
 * - if one arc is a self arc, it's least.
 * - if one arc is within a cycle, it's less than.
 * - if both arcs are within a cycle, compare arc counts.
 * - if neither arc is within a cycle, compare with
 * 	arc_time + arc_childtime as major key
 * 	arc count as minor key
 */
static
int
arccmp(
arctype *thisp,
arctype *thatp)
{
    nltype *thisparentp, *thischildp, *thatparentp, *thatchildp;
    double thistime, thattime;

	thisparentp = thisp->arc_parentp;
	thischildp  = thisp->arc_childp;
	thatparentp = thatp->arc_parentp;
	thatchildp  = thatp->arc_childp;

#ifdef DEBUG
	if(debug & TIMEDEBUG){
	    printf("[arccmp] ");
	    printname(thisparentp);
	    printf(" calls ");
	    printname(thischildp);
	    printf(" %f + %f %u/%d\n",
		   thisp->arc_time, thisp->arc_childtime,
		   thisp->arc_count, thischildp->ncall);
	    printf("[arccmp] ");
	    printname(thatparentp);
	    printf(" calls ");
	    printname(thatchildp);
	    printf(" %f + %f %u/%d\n",
		thatp->arc_time, thatp->arc_childtime,
		thatp->arc_count, thatchildp->ncall);
	    printf("\n");
	}
#endif /* DEBUG */
	if(thisparentp == thischildp){
	    /* this is a self call */
	    return(LESSTHAN);
	}
	if(thatparentp == thatchildp){
	    /* that is a self call */
	    return(GREATERTHAN);
	}
	if(thisparentp->cycleno != 0 && thischildp->cycleno != 0 &&
	   thisparentp->cycleno == thischildp->cycleno){
	    /* this is a call within a cycle */
	    if(thatparentp->cycleno != 0 && thatchildp->cycleno != 0 &&
	       thatparentp->cycleno == thatchildp->cycleno){
		/* that is a call within the cycle, too */
		if(thisp->arc_count < thatp->arc_count){
		    return(LESSTHAN);
		}
		if(thisp->arc_count > thatp->arc_count){
		    return(GREATERTHAN);
		}
		return(EQUALTO);
	    }
	    else{
		/* that isn't a call within the cycle */
		return(LESSTHAN);
	    }
	}
	else{
	    /* this isn't a call within a cycle */
	    if(thatparentp->cycleno != 0 && thatchildp->cycleno != 0 &&
	       thatparentp->cycleno == thatchildp->cycleno){
		/* that is a call within a cycle */
		return(GREATERTHAN);
	    }
	    else{
		/* neither is a call within a cycle */
		thistime = thisp->arc_time + thisp->arc_childtime;
		thattime = thatp->arc_time + thatp->arc_childtime;
		if(thistime < thattime)
		    return(LESSTHAN);
		if(thistime > thattime)
		    return(GREATERTHAN);
		if(thisp->arc_count < thatp->arc_count)
		    return(LESSTHAN);
		if(thisp->arc_count > thatp->arc_count)
		    return(GREATERTHAN);
		return(EQUALTO);
	    }
	}
}

static
void
printblurb(
char *blurbname)
{
    FILE *blurbfile;
    int input;

	blurbfile = fopen(blurbname, "r");
	if(blurbfile == NULL){
	    perror(blurbname);
	    return;
	}
	while((input = getc(blurbfile)) != EOF){
	    putchar(input);
	}
	fclose(blurbfile);
}

static
int
namecmp(
nltype **npp1,
nltype **npp2)
{
	return(strcmp((*npp1)->name, (*npp2)->name));
}

void
printindex(
void)
{
    nltype **namesortnlp;
    nltype *nlp;
    uint32_t index, nnames, todo, i, j;
    char peterbuffer[BUFSIZ];

	/*
	 * Now, sort regular function name alphbetically
	 * to create an index.
	 */
	namesortnlp = (nltype **)calloc(nname + ncycle, sizeof(nltype *));
	if(namesortnlp == NULL)
	    fatal("ran out of memory for sorting");

	for(index = 0, nnames = 0; index < nname; index++){
	    if(zflag == 0 && nl[index].ncall == 0 && nl[index].time == 0)
		continue;
	    namesortnlp[nnames++] = &nl[index];
	}
	qsort(namesortnlp, nnames, sizeof(nltype *),
	      (int (*)(const void *, const void *))namecmp);
	for(index = 1, todo = nnames; index <= (uint32_t)ncycle; index++){
	    namesortnlp[todo++] = &cyclenl[index];
	}
	printf("\f\nIndex by function name\n\n");
	index = ( todo + 2 ) / 3;
	for(i = 0; i < index ; i++){
	    for(j = i; j < todo; j += index){
		nlp = namesortnlp[j];
		if(nlp->printflag){
		    sprintf(peterbuffer, "[%d]", nlp->index);
		}
		else{
		    sprintf(peterbuffer, "(%d)", nlp->index);
		}
		if(j < nnames){
		    printf("%6.6s %-19.19s", peterbuffer, nlp->name);
		}
		else{
		    printf("%6.6s ", peterbuffer);
		    sprintf(peterbuffer, "<cycle %d>", nlp->cycleno);
		    printf("%-19.19s", peterbuffer);
		}
	    }
	    printf("\n");
	}
	free(namesortnlp);
}
