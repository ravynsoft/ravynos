/*    perlstatic.h
 *
 *    'I don't know half of you half as well as I should like; and I like less
 *    than half of you half as well as you deserve.'
 *
 *    Copyright (C) 2020 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 * This file is a home for static functions that we don't consider suitable for
 * inlining, but for which giving the compiler full knowledge of may be
 * advantageous.  Functions that have potential tail call optimizations are a
 * likely component.

 */

/* saves machine code for a common noreturn idiom typically used in Newx*() */
GCC_DIAG_IGNORE_DECL(-Wunused-function);

STATIC void
Perl_croak_memory_wrap(void)
{
    Perl_croak_nocontext("%s",PL_memory_wrap);
}

GCC_DIAG_RESTORE_DECL;


/*
 * ex: set ts=8 sts=4 sw=4 et:
 */

