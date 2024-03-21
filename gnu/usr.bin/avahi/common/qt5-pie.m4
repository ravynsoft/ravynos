AC_DEFUN([AC_AVAHI_QT_ADD_PIC_IF_NEEDED],
[
    AC_LANG_PUSH([C++])
	save_CPPFLAGS="$CPPFLAGS"
	CPPFLAGS="$CPPFLAGS $QT5_CFLAGS"
	AC_MSG_CHECKING([whether Qt works without -fPIC])
	AC_PREPROC_IFELSE(
		[AC_LANG_SOURCE([[#include <QtCore>]])],
		[AC_MSG_RESULT(yes)],
		[
			AC_MSG_RESULT(no)
			AC_MSG_CHECKING([whether Qt works with -fPIC])
			CPPFLAGS="$CPPFLAGS -fPIC"
			AC_PREPROC_IFELSE(
				[AC_LANG_SOURCE([[#include <QtCore>]])],
				[
					AC_MSG_RESULT(yes)
					QT5_CFLAGS="$QT5_CFLAGS -fPIC"
				],
				[
					AC_MSG_RESULT(no)
					AC_MSG_ERROR(Couldn't compile Qt without -fPIC nor with -fPIC)
				])
		])
	CPPFLAGS="$save_CPPFLAGS"
    AC_LANG_POP([C++])
])
