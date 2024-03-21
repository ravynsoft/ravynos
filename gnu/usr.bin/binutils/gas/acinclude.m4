dnl GAS_CHECK_DECL_NEEDED(name, typedefname, typedef, headers)
AC_DEFUN([GAS_CHECK_DECL_NEEDED],[
AC_MSG_CHECKING(whether declaration is required for $1)
AC_CACHE_VAL(gas_cv_decl_needed_$1,
AC_TRY_LINK([$4],
[
typedef $3;
$2 x;
x = ($2) $1;
], gas_cv_decl_needed_$1=no, gas_cv_decl_needed_$1=yes))dnl
AC_MSG_RESULT($gas_cv_decl_needed_$1)
if test $gas_cv_decl_needed_$1 = yes; then
 AC_DEFINE([NEED_DECLARATION_]translit($1, [a-z], [A-Z]), 1,
	   [Define if $1 is not declared in system header files.])
fi
])dnl
dnl
dnl Some non-ANSI preprocessors botch requoting inside strings.  That's bad
dnl enough, but on some of those systems, the assert macro relies on requoting
dnl working properly!
dnl GAS_WORKING_ASSERT
AC_DEFUN([GAS_WORKING_ASSERT],
[AC_MSG_CHECKING([for working assert macro])
AC_CACHE_VAL(gas_cv_assert_ok,
AC_TRY_LINK([#include <assert.h>
#include <string.h>
#include <stdio.h>], [
/* check for requoting problems */
static int a, b, c, d;
static char *s;
assert (!strcmp(s, "foo bar baz quux"));
/* check for newline handling */
assert (a == b
        || c == d);
], gas_cv_assert_ok=yes, gas_cv_assert_ok=no))dnl
AC_MSG_RESULT($gas_cv_assert_ok)
test $gas_cv_assert_ok = yes || AC_DEFINE(BROKEN_ASSERT, 1, [assert broken?])
])dnl
dnl
dnl Since many Bourne shell implementations lack subroutines, use this
dnl hack to simplify the code in configure.ac.
dnl GAS_UNIQ(listvar)
AC_DEFUN([GAS_UNIQ],
[_gas_uniq_list="[$]$1"
_gas_uniq_newlist=""
dnl Protect against empty input list.
for _gas_uniq_i in _gas_uniq_dummy [$]_gas_uniq_list ; do
  case [$]_gas_uniq_i in
  _gas_uniq_dummy) ;;
  *) case " [$]_gas_uniq_newlist " in
       *" [$]_gas_uniq_i "*) ;;
       *) _gas_uniq_newlist="[$]_gas_uniq_newlist [$]_gas_uniq_i" ;;
     esac ;;
  esac
done
$1=[$]_gas_uniq_newlist
])dnl
dnl
dnl Check for existence of member $2 in type $1 in time.h
dnl
AC_DEFUN([GAS_HAVE_TIME_TYPE_MEMBER],
[AC_MSG_CHECKING([for $1.$2 in time.h])
 AC_CACHE_VAL(gas_cv_have_time_type_member_$2,
   [AC_TRY_COMPILE([
#define _BSD_SOURCE 1
#include <time.h>],
      [$1 avar; void* aref = (void*) &avar.$2],
      gas_cv_have_time_type_member_$2=yes,
      gas_cv_have_time_type_member_$2=no
   )])
 if test $gas_cv_have_time_type_member_$2 = yes; then
   AC_DEFINE([HAVE_]translit($2, [a-z], [A-Z]), 1,
	     [Define if <time.h> has $1.$2.])
 fi
 AC_MSG_RESULT($gas_cv_have_time_type_member_$2)
])dnl
dnl
dnl Check for existence of member $2.$3 in type $1 in sys/stat.h
dnl
AC_DEFUN([GAS_HAVE_SYS_STAT_TYPE_MEMBER],
[AC_MSG_CHECKING([for $1.$2.$3 in sys/stat.h])
 AC_CACHE_VAL(gas_cv_have_sys_stat_type_member_$2_$3,
   [AC_TRY_COMPILE([
#define _BSD_SOURCE 1
#include <sys/stat.h>],
      [$1 avar; void* aref = (void*) &avar.$2.$3],
      gas_cv_have_sys_stat_type_member_$2_$3=yes,
      gas_cv_have_sys_stat_type_member_$2_$3=no
   )])
 if test $gas_cv_have_sys_stat_type_member_$2_$3 = yes; then
   AC_DEFINE([HAVE_]translit($2, [a-z], [A-Z])[_]translit($3, [a-z], [A-Z]), 1,
	     [Define if <sys/stat.h> has $1.$2.$3])
 fi
 AC_MSG_RESULT($gas_cv_have_sys_stat_type_member_$2_$3)
])dnl
