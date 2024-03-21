# manywarnings.m4 serial 24
dnl Copyright (C) 2008-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl From Simon Josefsson

AC_PREREQ([2.64])

# gl_MANYWARN_COMPLEMENT(OUTVAR, LISTVAR, REMOVEVAR)
# --------------------------------------------------
# Copy LISTVAR to OUTVAR except for the entries in REMOVEVAR.
# Elements separated by whitespace.  In set logic terms, the function
# does OUTVAR = LISTVAR \ REMOVEVAR.
AC_DEFUN([gl_MANYWARN_COMPLEMENT],
[
  gl_warn_set=
  set x $2; shift
  for gl_warn_item
  do
    case " $3 " in
      *" $gl_warn_item "*)
        ;;
      *)
        AS_VAR_APPEND([gl_warn_set], [" $gl_warn_item"])
        ;;
    esac
  done
  $1=$gl_warn_set
])

# gl_MANYWARN_ALL_GCC(VARIABLE)
# -----------------------------
# Add all documented GCC warning parameters to variable VARIABLE.
# Note that you need to test them using gl_WARN_ADD if you want to
# make sure your gcc understands it.
#
# The effects of this macro depend on the current language (_AC_LANG).
AC_DEFUN([gl_MANYWARN_ALL_GCC],
[_AC_LANG_DISPATCH([$0], _AC_LANG, $@)])

# Specialization for _AC_LANG = C.
AC_DEFUN([gl_MANYWARN_ALL_GCC(C)],
[
  AC_LANG_PUSH([C])

  dnl First, check for some issues that only occur when combining multiple
  dnl gcc warning categories.
  AC_REQUIRE([AC_PROG_CC])
  AS_IF([test -n "$GCC"], [
    AC_CACHE_CHECK([whether -Wno-missing-field-initializers is needed],
      [gl_cv_cc_nomfi_needed],
      [gl_cv_cc_nomfi_needed=no
       gl_save_CFLAGS="$CFLAGS"
       CFLAGS="$CFLAGS -Wextra -Werror"
       AC_COMPILE_IFELSE(
         [AC_LANG_PROGRAM(
            [[struct file_data { int desc, name; };
              struct cmp { struct file_data file[1]; };
              void f (struct cmp *r)
              {
                typedef struct { int a; int b; } s_t;
                s_t s1 = { 0, };
                struct cmp cmp = { .file[0].desc = r->file[0].desc + s1.a };
                *r = cmp;
              }
            ]],
            [[]])],
         [],
         [CFLAGS="$CFLAGS -Wno-missing-field-initializers"
          AC_COMPILE_IFELSE([],
            [gl_cv_cc_nomfi_needed=yes])])
       CFLAGS="$gl_save_CFLAGS"
    ])

    dnl Next, check if -Werror -Wuninitialized is useful with the
    dnl user's choice of $CFLAGS; some versions of gcc warn that it
    dnl has no effect if -O is not also used
    AC_CACHE_CHECK([whether -Wuninitialized is supported],
      [gl_cv_cc_uninitialized_supported],
      [gl_save_CFLAGS="$CFLAGS"
       CFLAGS="$CFLAGS -Werror -Wuninitialized"
       AC_COMPILE_IFELSE(
         [AC_LANG_PROGRAM([[]], [[]])],
         [gl_cv_cc_uninitialized_supported=yes],
         [gl_cv_cc_uninitialized_supported=no])
       CFLAGS="$gl_save_CFLAGS"
      ])
  ])

  # List all gcc warning categories.
  # To compare this list to your installed GCC's, run this Bash command:
  #
  # comm -3 \
  #  <((sed -n 's/^  *\(-[^ 0-9][^ ]*\).*/\1/p' manywarnings.m4; \
  #     awk '/^[^#]/ {print $1}' ../build-aux/gcc-warning.spec) | sort) \
  #  <(LC_ALL=C gcc --help=warnings | sed -n 's/^  \(-[^ ]*\) .*/\1/p' | sort)

  $1=
  for gl_manywarn_item in -fanalyzer -fstrict-flex-arrays \
    -Wall \
    -Warith-conversion \
    -Wbad-function-cast \
    -Wcast-align=strict \
    -Wdate-time \
    -Wdisabled-optimization \
    -Wdouble-promotion \
    -Wduplicated-branches \
    -Wduplicated-cond \
    -Wextra \
    -Wformat-signedness \
    -Winit-self \
    -Winline \
    -Winvalid-pch \
    -Wlogical-op \
    -Wmissing-declarations \
    -Wmissing-include-dirs \
    -Wmissing-prototypes \
    -Wnested-externs \
    -Wnull-dereference \
    -Wold-style-definition \
    -Wopenmp-simd \
    -Woverlength-strings \
    -Wpacked \
    -Wpointer-arith \
    -Wshadow \
    -Wstack-protector \
    -Wstrict-flex-arrays \
    -Wstrict-overflow \
    -Wstrict-prototypes \
    -Wsuggest-attribute=cold \
    -Wsuggest-attribute=const \
    -Wsuggest-attribute=format \
    -Wsuggest-attribute=malloc \
    -Wsuggest-attribute=noreturn \
    -Wsuggest-attribute=pure \
    -Wsuggest-final-methods \
    -Wsuggest-final-types \
    -Wsync-nand \
    -Wsystem-headers \
    -Wtrampolines \
    -Wuninitialized \
    -Wunknown-pragmas \
    -Wunsafe-loop-optimizations \
    -Wunused-macros \
    -Wvariadic-macros \
    -Wvector-operation-performance \
    -Wvla \
    -Wwrite-strings \
    \
    ; do
    AS_VAR_APPEND([$1], [" $gl_manywarn_item"])
  done

  # gcc --help=warnings outputs an unusual form for these options; list
  # them here so that the above 'comm' command doesn't report a false match.
  AS_VAR_APPEND([$1], [' -Warray-bounds=2'])
  AS_VAR_APPEND([$1], [' -Wattribute-alias=2'])
  AS_VAR_APPEND([$1], [' -Wbidi-chars=any,ucn'])
  AS_VAR_APPEND([$1], [' -Wformat-overflow=2'])
  AS_VAR_APPEND([$1], [' -Wformat=2'])
  AS_VAR_APPEND([$1], [' -Wformat-truncation=2'])
  AS_VAR_APPEND([$1], [' -Wimplicit-fallthrough=5'])
  AS_VAR_APPEND([$1], [' -Wshift-overflow=2'])
  AS_VAR_APPEND([$1], [' -Wuse-after-free=3'])
  AS_VAR_APPEND([$1], [' -Wunused-const-variable=2'])
  AS_VAR_APPEND([$1], [' -Wvla-larger-than=4031'])

  # These are needed for older GCC versions.
  if test -n "$GCC" && gl_gcc_version=`($CC --version) 2>/dev/null`; then
    case $gl_gcc_version in
      'gcc (GCC) '[[0-3]].* | \
      'gcc (GCC) '4.[[0-7]].*)
        AS_VAR_APPEND([$1], [' -fdiagnostics-show-option'])
        AS_VAR_APPEND([$1], [' -funit-at-a-time'])
          ;;
    esac
    case $gl_gcc_version in
      'gcc (GCC) '[[0-9]].*)
        AS_VAR_APPEND([$1], [' -fno-common'])
          ;;
    esac
  fi

  # Disable specific options as needed.
  if test "$gl_cv_cc_nomfi_needed" = yes; then
    AS_VAR_APPEND([$1], [' -Wno-missing-field-initializers'])
  fi

  if test "$gl_cv_cc_uninitialized_supported" = no; then
    AS_VAR_APPEND([$1], [' -Wno-uninitialized'])
  fi

  # This warning have too many false alarms in GCC 11.2.1.
  # https://gcc.gnu.org/bugzilla/show_bug.cgi?id=101713
  AS_VAR_APPEND([$1], [' -Wno-analyzer-malloc-leak'])

  AC_LANG_POP([C])
])

# Specialization for _AC_LANG = C++.
AC_DEFUN([gl_MANYWARN_ALL_GCC(C++)],
[
  gl_MANYWARN_ALL_GCC_CXX_IMPL([$1])
])
