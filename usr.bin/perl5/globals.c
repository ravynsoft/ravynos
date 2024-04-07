/*    globals.c
 *
 *    Copyright (C) 1995, 1999, 2000, 2001, 2008 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 * 'For the rest, they shall represent the other Free Peoples of the World:
 *  Elves, Dwarves, and Men.'                                --Elrond
 *
 *     [p.275 of _The Lord of the Rings_, II/iii: "The Ring Goes South"]
 */

/* This file exists to #include "perl.h" _ONCE_ with
 * PERL_IN_GLOBALS_C defined. That causes various global variables
 * in perl.h and other files it includes to be _defined_ (and initialized)
 * rather than just declared.
*/

#include "INTERN.h"
#define PERL_IN_GLOBALS_C
#include "perl.h"

/* regcomp.h * isn't #included in perl.h, as its only included within a
 * few specific files such as regcomp.c, regexec.c.  So include it
 * explicitly to process any data declarations within it.
 */
#include "regcomp.h"

/* We need somewhere to declare this. This file seems a good place.
 * This is not a regular "global" in that we don't know whether it needs to
 * exist until we include threads.h, and we don't want it as part of any
 * global struct (if that or something similar is re-introduced. */

#if defined(USE_ITHREADS) && defined(PERL_THREAD_LOCAL)
PERL_THREAD_LOCAL void *PL_current_context;
#endif

/*
 * ex: set ts=8 sts=4 sw=4 et:
 */
