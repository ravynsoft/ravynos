dnl librarypath macros
dnl  Copyright (C) 2005 Free Software Foundation
dnl  Copying and distribution of this file, with or without modification,
dnl  are permitted in any medium without royalty provided the copyright
dnl  notice and this notice are preserved.
dnl
dnl  Written by Andrew Ruder
dnl GS_ADD_LIBRARY_PATH
dnl Adds -L$1 -Wl,-R$1 on netbsd and -Wl,-rpath,$1 -L$1 elsewhere
dnl to LDFLAGS and LDIR_FLAGS
AC_DEFUN([GS_ADD_LIBRARY_PATH], [
case "$target_os" in
	netbsd*)	
		LDFLAGS="$LDFLAGS -L$1 -Wl,-R$1"
		LDIR_FLAGS="$LDIR_FLAGS -Wl,-R$1 -L$1";;
	*)	
		LDFLAGS="$LDFLAGS -Wl,-rpath,$1 -L$1"
		LDIR_FLAGS="$LDIR_FLAGS -Wl,-rpath,$1 -L$1";;
esac
])

