/* Memory allocation on the stack.
   Copyright (C) 1995, 1999, 2001-2020 Free Software Foundation, Inc.

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

/* When this file is included, it may be preceded only by preprocessor
   declarations.  Thanks to AIX.  Therefore we include it right after
   "config.h", not later.  */

/* Avoid using the symbol _ALLOCA_H here, as Bison assumes _ALLOCA_H
   means there is a real alloca function.  */
#ifndef _GL_ALLOCA_H
#define _GL_ALLOCA_H

/* alloca(N) returns a pointer (void* or char*) to N bytes of memory
   allocated on the stack, and which will last until the function returns.
   Use of alloca should be avoided:
     - inside arguments of function calls - undefined behaviour,
     - in inline functions - the allocation may actually last until the
       calling function returns,
     - for huge N (say, N >= 65536) - you never know how large (or small)
       the stack is, and when the stack cannot fulfill the memory allocation
       request, the program just crashes.
 */

#ifndef alloca
  /* Some version of mingw have an <alloca.h> that causes trouble when
     included after 'alloca' gets defined as a macro.  As a workaround,
     include this <alloca.h> first and define 'alloca' as a macro afterwards
     if needed.  */
# if defined __GNUC__ && (defined _WIN32 && ! defined __CYGWIN__) && @HAVE_ALLOCA_H@
#  include_next <alloca.h>
# endif
#endif
#ifndef alloca
# ifdef __GNUC__
#  define alloca __builtin_alloca
# else
#  if defined _AIX
 #pragma alloca
    /* Alternatively: #define alloca __alloca, works as well.  */
#  elif defined _MSC_VER
#   include <malloc.h>
#   define alloca _alloca
#  elif defined __DECC && defined __VMS
#   define alloca __ALLOCA
#  elif defined __TANDEM && defined _TNS_E_TARGET
#   ifdef  __cplusplus
extern "C"
#   endif
void *_alloca (unsigned short);
#   pragma intrinsic (_alloca)
#   define alloca _alloca
#  elif defined __MVS__
#   include <stdlib.h>
#  else
#   ifdef __hpux /* This section must match that of bison generated files. */
#    ifdef __cplusplus
extern "C" void *alloca (unsigned int);
#    else /* not __cplusplus */
extern void *alloca ();
#    endif /* not __cplusplus */
#   else /* not __hpux */
#    ifndef alloca
extern char *alloca ();
#    endif
#   endif /* __hpux */
#  endif
# endif
#endif

#endif /* _GL_ALLOCA_H */
