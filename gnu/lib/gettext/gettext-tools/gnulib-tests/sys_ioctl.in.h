/* Substitute for and wrapper around <sys/ioctl.h>.
   Copyright (C) 2008-2023 Free Software Foundation, Inc.

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

#ifndef _@GUARD_PREFIX@_SYS_IOCTL_H

#if __GNUC__ >= 3
@PRAGMA_SYSTEM_HEADER@
#endif
@PRAGMA_COLUMNS@

/* The include_next requires a split double-inclusion guard.  */
#if @HAVE_SYS_IOCTL_H@
# @INCLUDE_NEXT@ @NEXT_SYS_IOCTL_H@
#endif

#ifndef _@GUARD_PREFIX@_SYS_IOCTL_H
#define _@GUARD_PREFIX@_SYS_IOCTL_H

/* This file uses GNULIB_POSIXCHECK, HAVE_RAW_DECL_*.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

/* AIX 5.1 and Solaris 10 declare ioctl() in <unistd.h> and in <stropts.h>,
   but not in <sys/ioctl.h>.
   Haiku declares ioctl() in <unistd.h>, but not in <sys/ioctl.h>.
   But avoid namespace pollution on glibc systems.  */
#ifndef __GLIBC__
# include <unistd.h>
#endif

/* The definitions of _GL_FUNCDECL_RPL etc. are copied here.  */

/* The definition of _GL_WARN_ON_USE is copied here.  */


/* Declare overridden functions.  */

#if @GNULIB_IOCTL@
# if @REPLACE_IOCTL@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef ioctl
#   define ioctl rpl_ioctl
#  endif
_GL_FUNCDECL_RPL (ioctl, int,
                  (int fd, int request, ... /* {void *,char *} arg */));
_GL_CXXALIAS_RPL (ioctl, int,
                  (int fd, int request, ... /* {void *,char *} arg */));
# else
#  if @SYS_IOCTL_H_HAVE_WINSOCK2_H@ || 1
_GL_FUNCDECL_SYS (ioctl, int,
                  (int fd, int request, ... /* {void *,char *} arg */));
#  endif
_GL_CXXALIAS_SYS (ioctl, int,
                  (int fd, int request, ... /* {void *,char *} arg */));
# endif
# if __GLIBC__ >= 2
_GL_CXXALIASWARN (ioctl);
# endif
#elif @SYS_IOCTL_H_HAVE_WINSOCK2_H_AND_USE_SOCKETS@
# undef ioctl
# define ioctl ioctl_used_without_requesting_gnulib_module_ioctl
#elif defined GNULIB_POSIXCHECK
# undef ioctl
# if HAVE_RAW_DECL_IOCTL
_GL_WARN_ON_USE (ioctl, "ioctl does not portably work on sockets - "
                 "use gnulib module ioctl for portability");
# endif
#endif


#endif /* _@GUARD_PREFIX@_SYS_IOCTL_H */
#endif /* _@GUARD_PREFIX@_SYS_IOCTL_H */
