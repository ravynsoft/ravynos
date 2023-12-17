/* Expression evaluation for plural form selection.
   Copyright (C) 2005-2006, 2019 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2005.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#ifndef _PLURAL_EVAL_H
#define _PLURAL_EVAL_H


/* Definition of 'struct expression', and
   declaration of extract_plural_expression() and plural_eval().  */
#include "plural-exp.h"


/* Protection against signals during plural evaluation.  */

#include <setjmp.h>

/* Some platforms don't have the sigjmp_buf type in <setjmp.h>.  */
#if defined _MSC_VER || defined __MINGW32__
/* Native Woe32 API.  */
# define sigjmp_buf jmp_buf
# define sigsetjmp(env,savesigs) setjmp (env)
# define siglongjmp longjmp
#endif

/* We use siginfo to get precise information about the signal.
   But siginfo doesn't work on Irix 6.5 and on Cygwin 2005.  */
#if HAVE_SIGINFO && !defined (__sgi) && !defined (__CYGWIN__)
# define USE_SIGINFO 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Exit point.  Must be set before calling install_sigfpe_handler().  */
extern sigjmp_buf sigfpe_exit;

#if USE_SIGINFO
/* Additional information that is set before sigfpe_exit is invoked.  */
extern int volatile sigfpe_code;
#endif

/* Protect against signals during plural evaluation.  Must be called around
   calls to plural_eval().  Must be called in pairs.  */
extern void install_sigfpe_handler (void);
extern void uninstall_sigfpe_handler (void);

#ifdef __cplusplus
}
#endif


#endif /* _PLURAL_EVAL_H */
