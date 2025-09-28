/* Provide relocatable packages.
   Copyright (C) 2003, 2005, 2008-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2003.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#ifndef _RELOCATABLE_H
#define _RELOCATABLE_H

/* This file uses _GL_ATTRIBUTE_MALLOC, HAVE_VISIBILITY.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif


/* This can be enabled through the configure --enable-relocatable option.  */
#if ENABLE_RELOCATABLE

/* When building a shared library, we must export some functions.
   Note that because this is a private .h file, we don't need to use
   __declspec(dllimport) in any case.  */
#if HAVE_VISIBILITY && BUILDING_DLL
# define RELOCATABLE_SHLIB_EXPORTED __attribute__((__visibility__("default")))
#elif defined _MSC_VER && BUILDING_DLL
/* When building with MSVC, exporting a symbol means that the object file
   contains a "linker directive" of the form /EXPORT:symbol.  This can be
   inspected through the "objdump -s --section=.drectve FILE" or
   "dumpbin /directives FILE" commands.
   The symbols from this file should be exported if and only if the object
   file gets included in a DLL.  Libtool, on Windows platforms, defines
   the C macro DLL_EXPORT (together with PIC) when compiling for a shared
   library (called DLL under Windows) and does not define it when compiling
   an object file meant to be linked statically into some executable.  */
# if defined DLL_EXPORT
#  define RELOCATABLE_SHLIB_EXPORTED __declspec(dllexport)
# else
#  define RELOCATABLE_SHLIB_EXPORTED
# endif
#else
# define RELOCATABLE_SHLIB_EXPORTED
#endif

/* Sets the original and the current installation prefix of the package.
   Relocation simply replaces a pathname starting with the original prefix
   by the corresponding pathname with the current prefix instead.  Both
   prefixes should be directory names without trailing slash (i.e. use ""
   instead of "/").  */
extern RELOCATABLE_SHLIB_EXPORTED void
       set_relocation_prefix (const char *orig_prefix,
                              const char *curr_prefix);

/* Returns the pathname, relocated according to the current installation
   directory.
   The returned string is either PATHNAME unmodified or a freshly allocated
   string that you can free with free() after casting it to 'char *'.  */
extern const char * relocate (const char *pathname);

/* Returns the pathname, relocated according to the current installation
   directory.
   This function sets *ALLOCATEDP to the allocated memory, or to NULL if
   no memory allocation occurs.  So that, after you're done with the return
   value, to reclaim allocated memory, you can do: free (*ALLOCATEDP).  */
extern const char * relocate2 (const char *pathname, char **allocatedp);

/* Memory management: relocate() potentially allocates memory, because it has
   to construct a fresh pathname.  If this is a problem because your program
   calls relocate() frequently or because you want to fix all potential memory
   leaks anyway, you have three options:
   1) Use this idiom:
        const char *pathname = ...;
        const char *rel_pathname = relocate (pathname);
        ...
        if (rel_pathname != pathname)
          free ((char *) rel_pathname);
   2) Use this idiom:
        char *allocated;
        const char *rel_pathname = relocate2 (..., &allocated);
        ...
        free (allocated);
   3) Think about caching the result.  */

/* Convenience function:
   Computes the current installation prefix, based on the original
   installation prefix, the original installation directory of a particular
   file, and the current pathname of this file.
   Returns it, freshly allocated.  Returns NULL upon failure.  */
extern char * compute_curr_prefix (const char *orig_installprefix,
                                   const char *orig_installdir,
                                   const char *curr_pathname)
  _GL_ATTRIBUTE_MALLOC _GL_ATTRIBUTE_DEALLOC_FREE;

#else

/* By default, we use the hardwired pathnames.  */
#define relocate(pathname) (pathname)
#define relocate2(pathname,allocatedp) (*(allocatedp) = NULL, (pathname))

#endif


#ifdef __cplusplus
}
#endif

#endif /* _RELOCATABLE_H */
