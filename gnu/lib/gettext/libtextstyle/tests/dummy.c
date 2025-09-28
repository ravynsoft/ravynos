/* A dummy file, to prevent empty libraries from breaking builds.
   Copyright (C) 2004, 2007, 2009-2023 Free Software Foundation, Inc.

   This file is in the public domain.  */

/* Some systems, reportedly OpenBSD and Mac OS X, refuse to create
   libraries without any object files.  You might get an error like:

   > ar cru .libs/libgl.a
   > ar: no archive members specified

   Compiling this file, and adding its object file to the library, will
   prevent the library from being empty.  */

/* Some systems, such as Solaris with cc 5.0, refuse to work with libraries
   that don't export any symbol.  You might get an error like:

   > cc ... libgnu.a
   > ild: (bad file) garbled symbol table in archive ../gllib/libgnu.a

   Compiling this file, and adding its object file to the library, will
   prevent the library from exporting no symbols.  */

#ifdef __sun
/* This declaration ensures that the library will export at least 1 symbol.  */
int gl_dummy_symbol;
#else
/* This declaration is solely to ensure that after preprocessing
   this file is never empty.  */
typedef int dummy;
#endif
