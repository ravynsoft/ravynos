AC_DEFUN([DISPATCH_C_ATOMIC_BUILTINS], [
#
# This is a bit subtle: on i386 systems without at least -march=i486 defined,
# certain built-in atomics fall back to depending on undefined symbols if
# their return values are used.
#
AC_CACHE_CHECK([for gcc atomic builtins],[dispatch_cv_atomic],
[AC_LINK_IFELSE([AC_LANG_PROGRAM([],[[
int i, x =0;
i = __sync_add_and_fetch(&x,1);
return x;]])],[dispatch_cv_atomic=yes],
  [saveCFLAGS="$CFLAGS"
  CFLAGS="$CFLAGS -march=i486"
  AC_LINK_IFELSE([AC_LANG_PROGRAM([],[[
  int i, x =0;
  i = __sync_add_and_fetch(&x,1);
  return x;]])],[CFLAGS="$saveCFLAGS"
dispatch_cv_atomic="-march=i486"
])])])

])
