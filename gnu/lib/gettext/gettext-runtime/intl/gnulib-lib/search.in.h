/* A GNU-like <search.h>.

   Copyright (C) 2007-2023 Free Software Foundation, Inc.

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

#ifndef _@GUARD_PREFIX@_SEARCH_H

#if __GNUC__ >= 3
@PRAGMA_SYSTEM_HEADER@
#endif
@PRAGMA_COLUMNS@

/* The include_next requires a split double-inclusion guard.  */
#if @HAVE_SEARCH_H@
# @INCLUDE_NEXT@ @NEXT_SEARCH_H@
#endif

#ifndef _@GUARD_PREFIX@_SEARCH_H
#define _@GUARD_PREFIX@_SEARCH_H

/* This file uses GNULIB_POSIXCHECK, HAVE_RAW_DECL_*.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif


/* The definitions of _GL_FUNCDECL_RPL etc. are copied here.  */

/* The definition of _GL_ARG_NONNULL is copied here.  */

/* The definition of _GL_WARN_ON_USE is copied here.  */


#ifdef __cplusplus
extern "C" {
#endif
#if !GNULIB_defined_lsearch_fn_types
typedef int (*_gl_lsearch_compar_fn) (const void *, const void *);
# define GNULIB_defined_lsearch_fn_types 1
#endif
#ifdef __cplusplus
}
#endif

#if @GNULIB_MDA_LFIND@
/* On native Windows, map 'lfind' to '_lfind', so that -loldnames is not
   required.  In C++ with GNULIB_NAMESPACE, avoid differences between
   platforms by defining GNULIB_NAMESPACE::lfind always.  */
# if defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef lfind
#   define lfind _lfind
#  endif
_GL_CXXALIAS_MDA (lfind, void *,
                  (const void *key, const void *base, unsigned int *nmemb,
                   unsigned int size,
                   _gl_lsearch_compar_fn compar));
# else
_GL_CXXALIAS_SYS (lfind, void *,
                  (const void *key, const void *base, size_t *nmemb,
                   size_t size,
                   _gl_lsearch_compar_fn compar));
# endif
_GL_CXXALIASWARN (lfind);
#endif

#if @GNULIB_MDA_LSEARCH@
/* On native Windows, map 'lsearch' to '_lsearch', so that -loldnames is not
   required.  In C++ with GNULIB_NAMESPACE, avoid differences between
   platforms by defining GNULIB_NAMESPACE::lsearch always.  */
# if defined _WIN32 && !defined __CYGWIN__
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef lsearch
#   define lsearch _lsearch
#  endif
_GL_CXXALIAS_MDA (lsearch, void *,
                  (const void *key, void *base, unsigned int *nmemb,
                   unsigned int size,
                   _gl_lsearch_compar_fn compar));
# else
_GL_CXXALIAS_SYS (lsearch, void *,
                  (const void *key, void *base, size_t *nmemb,
                   size_t size,
                   _gl_lsearch_compar_fn compar));
# endif
_GL_CXXALIASWARN (lsearch);
#endif


#if @GNULIB_TSEARCH@
# if @REPLACE_TSEARCH@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   define tsearch rpl_tsearch
#   define tfind rpl_tfind
#   define tdelete rpl_tdelete
#  endif
# endif
# if @REPLACE_TWALK@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   define twalk rpl_twalk
#  endif
# endif

/* See <https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/search.h.html>
       <https://pubs.opengroup.org/onlinepubs/9699919799/functions/tsearch.html>
   for details.  */

# if !@HAVE_TYPE_VISIT@
#  if !GNULIB_defined_search_types
typedef enum
{
  preorder,
  postorder,
  endorder,
  leaf
}
VISIT;
#   define GNULIB_defined_search_types 1
#  endif
# endif

# ifdef __cplusplus
extern "C" {
# endif
# if !GNULIB_defined_search_fn_types
typedef int (*_gl_search_compar_fn) (const void *, const void *);
typedef void (*_gl_search_action_fn) (const void *, VISIT, int);
#  define GNULIB_defined_search_fn_types 1
# endif
# ifdef __cplusplus
}
# endif

/* Searches an element in the tree *VROOTP that compares equal to KEY.
   If one is found, it is returned.  Otherwise, a new element equal to KEY
   is inserted in the tree and is returned.  */
# if @REPLACE_TSEARCH@
_GL_FUNCDECL_RPL (tsearch, void *,
                  (const void *key, void **vrootp,
                   _gl_search_compar_fn compar)
                  _GL_ARG_NONNULL ((1, 2, 3)));
_GL_CXXALIAS_RPL (tsearch, void *,
                  (const void *key, void **vrootp,
                   _gl_search_compar_fn compar));
# else
#  if !@HAVE_TSEARCH@
_GL_FUNCDECL_SYS (tsearch, void *,
                  (const void *key, void **vrootp,
                   _gl_search_compar_fn compar)
                  _GL_ARG_NONNULL ((1, 2, 3)));
#  endif
_GL_CXXALIAS_SYS (tsearch, void *,
                  (const void *key, void **vrootp,
                   _gl_search_compar_fn compar));
# endif
# if __GLIBC__ >= 2
_GL_CXXALIASWARN (tsearch);
# endif

/* Searches an element in the tree *VROOTP that compares equal to KEY.
   If one is found, it is returned.  Otherwise, NULL is returned.  */
# if @REPLACE_TSEARCH@
_GL_FUNCDECL_RPL (tfind, void *,
                  (const void *key, void *const *vrootp,
                   _gl_search_compar_fn compar)
                  _GL_ARG_NONNULL ((1, 2, 3)));
_GL_CXXALIAS_RPL (tfind, void *,
                  (const void *key, void *const *vrootp,
                   _gl_search_compar_fn compar));
# else
#  if !@HAVE_TSEARCH@
_GL_FUNCDECL_SYS (tfind, void *,
                  (const void *key, void *const *vrootp,
                   _gl_search_compar_fn compar)
                  _GL_ARG_NONNULL ((1, 2, 3)));
#  endif
/* Need to cast, because on Cygwin 1.5.x systems, the second parameter is
                                         void **vrootp.  */
_GL_CXXALIAS_SYS_CAST (tfind, void *,
                       (const void *key, void *const *vrootp,
                        _gl_search_compar_fn compar));
# endif
# if __GLIBC__ >= 2
_GL_CXXALIASWARN (tfind);
# endif

/* Searches an element in the tree *VROOTP that compares equal to KEY.
   If one is found, it is removed from the tree, and its parent node is
   returned.  Otherwise, NULL is returned.  */
# if @REPLACE_TSEARCH@
_GL_FUNCDECL_RPL (tdelete, void *,
                  (const void *restrict key, void **restrict vrootp,
                   _gl_search_compar_fn compar)
                  _GL_ARG_NONNULL ((1, 2, 3)));
_GL_CXXALIAS_RPL (tdelete, void *,
                  (const void *restrict key, void **restrict vrootp,
                   _gl_search_compar_fn compar));
# else
#  if !@HAVE_TSEARCH@
_GL_FUNCDECL_SYS (tdelete, void *,
                  (const void *restrict key, void **restrict vrootp,
                   _gl_search_compar_fn compar)
                  _GL_ARG_NONNULL ((1, 2, 3)));
#  endif
_GL_CXXALIAS_SYS (tdelete, void *,
                  (const void *restrict key, void **restrict vrootp,
                   _gl_search_compar_fn compar));
# endif
# if __GLIBC__ >= 2
_GL_CXXALIASWARN (tdelete);
# endif

/* Perform a depth-first, left-to-right traversal of the tree VROOT.
   The ACTION function is called:
     - for non-leaf nodes: 3 times, before the left subtree traversal,
       after the left subtree traversal but before the right subtree traversal,
       and after the right subtree traversal,
     - for leaf nodes: once.
   The arguments passed to ACTION are:
     1. the node; it can be cast to a 'const void * const *', i.e. into a
        pointer to the key,
     2. an indicator which visit of the node this is,
     3. the level of the node in the tree (0 for the root).  */
# if @REPLACE_TWALK@
_GL_FUNCDECL_RPL (twalk, void,
                  (const void *vroot, _gl_search_action_fn action)
                  _GL_ARG_NONNULL ((2)));
_GL_CXXALIAS_RPL (twalk, void,
                  (const void *vroot, _gl_search_action_fn action));
# else
#  if !@HAVE_TWALK@
_GL_FUNCDECL_SYS (twalk, void,
                  (const void *vroot, _gl_search_action_fn action)
                  _GL_ARG_NONNULL ((2)));
#  endif
_GL_CXXALIAS_SYS (twalk, void,
                  (const void *vroot, _gl_search_action_fn action));
# endif
# if __GLIBC__ >= 2
_GL_CXXALIASWARN (twalk);
# endif

/* Flags used by tsearch.c.  */
# define GNULIB_defined_tsearch  (@REPLACE_TSEARCH@ || !@HAVE_TSEARCH@)
# define GNULIB_defined_twalk    (@REPLACE_TWALK@ || !@HAVE_TWALK@)

#elif defined GNULIB_POSIXCHECK
# undef tsearch
# if HAVE_RAW_DECL_TSEARCH
_GL_WARN_ON_USE (tsearch, "tsearch is unportable - "
                 "use gnulib module tsearch for portability");
# endif
# undef tfind
# if HAVE_RAW_DECL_TFIND
_GL_WARN_ON_USE (tfind, "tfind is unportable - "
                 "use gnulib module tsearch for portability");
# endif
# undef tdelete
# if HAVE_RAW_DECL_TDELETE
_GL_WARN_ON_USE (tdelete, "tdelete is unportable - "
                 "use gnulib module tsearch for portability");
# endif
# undef twalk
# if HAVE_RAW_DECL_TWALK
_GL_WARN_ON_USE (twalk, "twalk is unportable - "
                 "use gnulib module tsearch for portability");
# endif
#endif


#endif /* _@GUARD_PREFIX@_SEARCH_H */
#endif /* _@GUARD_PREFIX@_SEARCH_H */
