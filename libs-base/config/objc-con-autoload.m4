AC_DEFUN(OBJC_CON_AUTOLOAD,
# Copyright (C) 2005 Free Software Foundation
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.
#--------------------------------------------------------------------
# Guess if we are using a object file format that supports automatic
# loading of constructor functions.
#
# If this system supports autoloading of constructors, that means that gcc
# doesn't have to do it for us via collect2. This routine tests for this
# in a very roundabout way by compiling a program with a constructor and
# testing the file, via nm, for certain symbols that collect2 includes to
# handle loading of constructors.
#
# Makes the following substitutions:
#	Defines CON_AUTOLOAD (whether constructor functions are autoloaded)
#--------------------------------------------------------------------
[dnl
AC_MSG_CHECKING(loading of constructor functions)
AC_CACHE_VAL(objc_cv_con_autoload,
[dnl
AC_TRY_RUN([static int loaded = 0;
	void cons_functions() __attribute__ ((constructor));
	void cons_functions() { loaded = 1; }
	int main()
	{
  	  return ( (loaded == 1) ? 0 : 1);
	}],
	objc_cv_con_autoload=yes, objc_cv_con_autoload=no, 
	objc_cv_con_autoload=no)
case "$target_os" in
    cygwin*)	objc_cv_con_autoload=yes;;
    mingw*)	objc_cv_con_autoload=yes;;
esac
])
if test $objc_cv_con_autoload = yes; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(CON_AUTOLOAD,1,[Define if constructors are automatically loaded])
else
  AC_MSG_RESULT(no)
fi
])
