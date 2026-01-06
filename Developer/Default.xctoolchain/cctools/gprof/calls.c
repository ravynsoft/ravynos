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
#ifdef vax
#include "vax.h"
#endif

#ifdef m68k
#include "m68k.h"
#endif

/*
 * A namelist entry to be the child of indirect calls.
 */
static nltype indirectchild = {
	"(*)" ,				/* the name */
	(uint64_t) 0 ,			/* the pc entry point */
	(uint64_t) 0 ,			/* entry point aligned to histogram */
	(double) 0.0 ,			/* ticks in this routine */
	(double) 0.0 ,			/* cumulative ticks in children */
	(uint32_t) 0 ,			/* order called */
	(int32_t) 0 ,			/* how many times called */
	(int32_t) 0 ,			/* how many calls to self */
	(double) 1.0 ,			/* propagation fraction */
	(double) 0.0 ,			/* self propagation time */
	(double) 0.0 ,			/* child propagation time */
	(enum bool) 0 ,			/* print flag */
	(int) 0 ,			/* index in the graph list */
	(int) 0 , 			/* graph call chain top-sort order */
	(int) 0 ,			/* internal number of cycle on */
	(struct nl *) &indirectchild ,	/* pointer to head of cycle */
	(struct nl *) 0 ,		/* pointer to next member of cycle */
	(arctype *) 0 ,			/* list of caller arcs */
	(arctype *) 0 			/* list of callee arcs */
};

#ifdef vax
operandenum
operandmode( modep )
    struct modebyte	*modep;
{
    uint32_t	usesreg = modep -> regfield;
    
    switch ( modep -> modefield ) {
	case 0:
	case 1:
	case 2:
	case 3:
	    return literal;
	case 4:
	    return indexed;
	case 5:
	    return reg;
	case 6:
	    return regdef;
	case 7:
	    return autodec;
	case 8:
	    return ( usesreg != PC ? autoinc : immediate );
	case 9:
	    return ( usesreg != PC ? autoincdef : absolute );
	case 10:
	    return ( usesreg != PC ? bytedisp : byterel );
	case 11:
	    return ( usesreg != PC ? bytedispdef : bytereldef );
	case 12:
	    return ( usesreg != PC ? worddisp : wordrel );
	case 13:
	    return ( usesreg != PC ? worddispdef : wordreldef );
	case 14:
	    return ( usesreg != PC ? longdisp : longrel );
	case 15:
	    return ( usesreg != PC ? longdispdef : longreldef );
    }
    /* NOTREACHED */
}

static
char *
operandname( mode )
    operandenum	mode;
{
    
    switch ( mode ) {
	case literal:
	    return "literal";
	case indexed:
	    return "indexed";
	case reg:
	    return "register";
	case regdef:
	    return "register deferred";
	case autodec:
	    return "autodecrement";
	case autoinc:
	    return "autoincrement";
	case autoincdef:
	    return "autoincrement deferred";
	case bytedisp:
	    return "byte displacement";
	case bytedispdef:
	    return "byte displacement deferred";
	case byterel:
	    return "byte relative";
	case bytereldef:
	    return "byte relative deferred";
	case worddisp:
	    return "word displacement";
	case worddispdef:
	    return "word displacement deferred";
	case wordrel:
	    return "word relative";
	case wordreldef:
	    return "word relative deferred";
	case immediate:
	    return "immediate";
	case absolute:
	    return "absolute";
	case longdisp:
	    return "long displacement";
	case longdispdef:
	    return "long displacement deferred";
	case longrel:
	    return "long relative";
	case longreldef:
	    return "long relative deferred";
    }
    /* NOTREACHED */
}

static
uint32_t
operandlength( modep )
    struct modebyte	*modep;
{
    
    switch ( operandmode( modep ) ) {
	case literal:
	case reg:
	case regdef:
	case autodec:
	case autoinc:
	case autoincdef:
	    return 1;
	case bytedisp:
	case bytedispdef:
	case byterel:
	case bytereldef:
	    return 2;
	case worddisp:
	case worddispdef:
	case wordrel:
	case wordreldef:
	    return 3;
	case immediate:
	case absolute:
	case longdisp:
	case longdispdef:
	case longrel:
	case longreldef:
	    return 5;
	case indexed:
	    return 1+operandlength( (struct modebyte *) ((char *) modep) + 1 );
    }
    /* NOTREACHED */
}

static
uint32_t
reladdr( modep )
    struct modebyte	*modep;
{
    operandenum	mode = operandmode( modep );
    char	*cp;
    short	*sp;
    int32_t	*lp;

    cp = (char *) modep;
    cp += 1;			/* skip over the mode */
    switch ( mode ) {
	default:
	    fprintf( stderr , "[reladdr] not relative address\n" );
	    return (uint32_t) modep;
	case byterel:
	    return (uint32_t) ( cp + sizeof *cp + *cp );
	case wordrel:
	    sp = (short *) cp;
	    return (uint32_t) ( cp + sizeof *sp + *sp );
	case longrel:
	    lp = (int32_t *) cp;
	    return (uint32_t) ( cp + sizeof *lp + *lp );
    }
}

findcalls( parentp , p_lowpc , p_highpc )
    nltype		*parentp;
    uint32_t		p_lowpc;
    uint32_t		p_highpc;
{
    unsigned char	*instructp;
    int32_t		length;
    nltype		*childp;
    operandenum		mode;
    operandenum		firstmode;
    uint32_t	destpc;

    if ( textspace == 0 ) {
	return;
    }
    if ( p_lowpc < s_lowpc ) {
	p_lowpc = s_lowpc;
    }
    if ( p_highpc > s_highpc ) {
	p_highpc = s_highpc;
    }
#   ifdef DEBUG
	if ( debug & CALLSDEBUG ) {
	    printf( "[findcalls] %s: 0x%x to 0x%x\n" ,
		    parentp -> name , p_lowpc , p_highpc );
	}
#   endif /* DEBUG */
    for (   instructp = textspace + p_lowpc ;
	    instructp < textspace + p_highpc ;
	    instructp += length ) {
	length = 1;
	if ( *instructp == CALLS ) {
		/*
		 *	maybe a calls, better check it out.
		 *	skip the count of the number of arguments.
		 */
#	    ifdef DEBUG
		if ( debug & CALLSDEBUG ) {
		    printf( "[findcalls]\t0x%x:calls" , instructp - textspace );
		}
#	    endif /* DEBUG */
	    firstmode = operandmode( (struct modebyte *) (instructp+length) );
	    switch ( firstmode ) {
		case literal:
		case immediate:
		    break;
		default:
		    goto botched;
	    }
	    length += operandlength( (struct modebyte *) (instructp+length) );
	    mode = operandmode( (struct modebyte *) ( instructp + length ) );
#	    ifdef DEBUG
		if ( debug & CALLSDEBUG ) {
		    printf( "\tfirst operand is %s", operandname( firstmode ) );
		    printf( "\tsecond operand is %s\n" , operandname( mode ) );
		}
#	    endif /* DEBUG */
	    switch ( mode ) {
		case regdef:
		case bytedispdef:
		case worddispdef:
		case longdispdef:
		case bytereldef:
		case wordreldef:
		case longreldef:
			/*
			 *	indirect call: call through pointer
			 *	either	*d(r)	as a parameter or local
			 *		(r)	as a return value
			 *		*f	as a global pointer
			 *	[are there others that we miss?,
			 *	 e.g. arrays of pointers to functions???]
			 */
		    addarc( parentp , &indirectchild , (uint32_t) 0 , 0);
		    length += operandlength(
				(struct modebyte *) ( instructp + length ) );
		    continue;
		case byterel:
		case wordrel:
		case longrel:
			/*
			 *	regular pc relative addressing
			 *	check that this is the address of 
			 *	a function.
			 */
		    destpc = reladdr( (struct modebyte *) (instructp+length) )
				- (uint32_t) textspace;
		    if ( destpc >= s_lowpc && destpc <= s_highpc ) {
			childp = nllookup( destpc );
#			ifdef DEBUG
			    if ( debug & CALLSDEBUG ) {
				printf( "[findcalls]\tdestpc 0x%x" , destpc );
				printf( " childp->name %s" , childp -> name );
				printf( " childp->value 0x%x\n" ,
					childp -> value );
			    }
#			endif /* DEBUG */
			if ( childp -> value == destpc ) {
				/*
				 *	a hit
				 */
			    addarc( parentp , childp , (uint32_t) 0 , 0);
			    length += operandlength( (struct modebyte *)
					    ( instructp + length ) );
			    continue;
			}
			goto botched;
		    }
			/*
			 *	else:
			 *	it looked like a calls,
			 *	but it wasn't to anywhere.
			 */
		    goto botched;
		default:
		botched:
			/*
			 *	something funny going on.
			 */
#		    ifdef DEBUG
			if ( debug & CALLSDEBUG ) {
			    printf( "[findcalls]\tbut it's a botch\n" );
			}
#		    endif /* DEBUG */
		    length = 1;
		    continue;
	    }
	}
    }
}
#endif /* vax */

#ifdef m68k
void
findcalls(
nltype *parentp,
uint32_t p_lowpc,
uint32_t p_highpc)
{
    unsigned short	*instructp;
    int32_t		length;
    nltype		*childp;
    uint32_t		destpc;

	if(textspace == NULL){
	    return;
	}
	if(p_lowpc < sample_sets->s_lowpc){
	    p_lowpc = sample_sets->s_lowpc;
	}
	if(p_highpc > sample_sets->s_highpc){
	    p_highpc = sample_sets->s_highpc;
	}
#ifdef DEBUG
	if(debug & CALLSDEBUG){
	    printf("[findcalls] %s: 0x%x to 0x%x\n", parentp->name,
		   (unsigned int)p_lowpc, (unsigned int)p_highpc);
	}
#endif
	for(instructp = (unsigned short *)(textspace + p_lowpc);
	    instructp < (unsigned short *)(textspace + p_highpc);
	    instructp += length){

	    length = 1;			/* 1 word */

	    if((*instructp & BSR_MASK) == BSR_OP){
		/* bsr instruction */
		short disp;

		if(*instructp & 0xff)
		    disp = (char)(*instructp & 0xff);
		else{
		    length = 2;	/* 2 words */
		    disp = instructp[1];
		}
		destpc = (uint32_t)instructp + disp + 2 - 
			 (uint32_t)textspace;
gotdestpc:
		if(destpc >= sample_sets->s_lowpc &&
		   destpc <= sample_sets->s_highpc){
		    childp = nllookup(destpc);
#ifdef DEBUG
		    if(debug & CALLSDEBUG){
			printf("[findcalls]\tdestpc 0x%x",
			       (unsigned int)destpc);
			printf("childp->name %s", childp->name);
			printf("childp->value 0x%x\n",
			       (unsigned int)childp->value );
		    }
#endif
		    if(childp->value == destpc){
			/*
			 * a hit
			 */
			addarc(parentp, childp, 0, 0);
			continue;
		    }
		    goto botched;
		}
	    }
	    else if((*instructp & JBSR_MASK) == JBSR_OP){
		/* jbsr instruction */
		if((*instructp & 0x38) != 0x38) {
indirect:
		    addarc(parentp, &indirectchild, 0, 0);
		    continue;
		}
		switch(*instructp & 7) {
		case 0:			/* Abs.W */
		    destpc = instructp[1];
		    length = 2;
		    goto gotdestpc;
		case 1:			/* Abs.L */
		    destpc = (instructp[1] << 16) + (instructp[2] & 0xffff);
		    length = 3;
		    goto gotdestpc;
		default:
		    goto indirect;
		}
	    }
	    else
		continue;
botched:
	    /*
	     *	something funny going on.
	     */
#ifdef DEBUG
	    if(debug & CALLSDEBUG){
		printf("[findcalls]\tbut it's a botch\n");
	    }
#endif
	    length = 1;
	}
}
#endif /* m68k */
