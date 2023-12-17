#!/bin/sh
# Convenience script for regenerating all autogeneratable files that are
# omitted from the version control repository. In particular, this script
# also regenerates all aclocal.m4, config.h.in, Makefile.in, configure files
# with new versions of autoconf or automake.
#
# This script requires autoconf-2.64..2.71 and automake-1.13..1.16 in the PATH.

# Copyright (C) 2003-2023 Free Software Foundation, Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# Prerequisite (if not used from a released tarball): either
#   - the GNULIB_SRCDIR environment variable pointing to a gnulib checkout, or
#   - a preceding invocation of './autopull.sh'.
#
# Usage: ./autogen.sh [--skip-gnulib]
#
# Options:
#   --skip-gnulib       Avoid fetching files from Gnulib.
#                       This option is useful
#                       - when you are working from a released tarball (possibly
#                         with modifications), or
#                       - as a speedup, if the set of gnulib modules did not
#                         change since the last time you ran this script.

# Nuisances.
(unset CDPATH) >/dev/null 2>&1 && unset CDPATH

skip_gnulib=false
skip_gnulib_option=

while :; do
  case "$1" in
    --skip-gnulib) skip_gnulib=true; skip_gnulib_option='--skip-gnulib'; shift;;
    *) break ;;
  esac
done

# The tests in gettext-tools/tests are not meant to be executable, because
# they have a TESTS_ENVIRONMENT that specifies the shell explicitly.

if ! $skip_gnulib; then
  if test -n "$GNULIB_SRCDIR"; then
    test -d "$GNULIB_SRCDIR" || {
      echo "*** GNULIB_SRCDIR is set but does not point to an existing directory." 1>&2
      exit 1
    }
  else
    GNULIB_SRCDIR=`pwd`/gnulib
    test -d "$GNULIB_SRCDIR" || {
      echo "*** Subdirectory 'gnulib' does not yet exist. Use './gitsub.sh pull' to create it, or set the environment variable GNULIB_SRCDIR." 1>&2
      exit 1
    }
  fi
  # Now it should contain a gnulib-tool.
  GNULIB_TOOL="$GNULIB_SRCDIR/gnulib-tool"
  test -f "$GNULIB_TOOL" || {
    echo "*** gnulib-tool not found." 1>&2
    exit 1
  }
  # In gettext-runtime:
  GNULIB_MODULES_RUNTIME_FOR_SRC='
    atexit
    attribute
    basename-lgpl
    binary-io
    closeout
    error
    getopt-gnu
    gettext-h
    havelib
    memmove
    noreturn
    progname
    propername
    relocatable-prog
    setlocale
    sigpipe
    stdbool
    stdio
    stdlib
    strtoul
    unistd
    unlocked-io
    xalloc
  '
  GNULIB_MODULES_RUNTIME_OTHER='
    gettext-runtime-misc
    ansi-c++-opt
    csharpcomp-script
    java
    javacomp-script
    manywarnings
  '
  $GNULIB_TOOL --dir=gettext-runtime --lib=libgrt --source-base=gnulib-lib --m4-base=gnulib-m4 --no-libtool --local-dir=gnulib-local --local-symlink \
    --import $GNULIB_MODULES_RUNTIME_FOR_SRC $GNULIB_MODULES_RUNTIME_OTHER || exit $?
  $GNULIB_TOOL --copy-file m4/build-to-host.m4 gettext-runtime/m4/build-to-host.m4 || exit $?
  # In gettext-runtime/intl:
  GNULIB_MODULES_LIBINTL='
    gettext-runtime-intl-misc
    attribute
    bison
    filename
    flexmember
    havelib
    lib-symbol-visibility
    localcharset
    localename
    lock
    manywarnings
    relocatable-lib-lgpl
    tsearch
    vasnprintf-posix
    vasnwprintf-posix
  '
  GNULIB_SETLOCALE_DEPENDENCIES=`$GNULIB_TOOL --extract-dependencies setlocale | sed -e 's/ .*//'`
  $GNULIB_TOOL --dir=gettext-runtime/intl --source-base=gnulib-lib --m4-base=gnulib-m4 --lgpl=2 --libtool --local-dir=gnulib-local --local-symlink \
    --import $GNULIB_MODULES_LIBINTL $GNULIB_SETLOCALE_DEPENDENCIES || exit $?
  # In gettext-runtime/libasprintf:
  GNULIB_MODULES_LIBASPRINTF='
    alloca
    manywarnings
    vasnprintf
  '
  $GNULIB_TOOL --dir=gettext-runtime/libasprintf --source-base=gnulib-lib --m4-base=gnulib-m4 --lgpl=2 --libtool --local-dir=gnulib-local --local-symlink \
    --import $GNULIB_MODULES_LIBASPRINTF || exit $?
  # In gettext-tools:
  GNULIB_MODULES_TOOLS_FOR_SRC='
    alloca-opt
    atexit
    attribute
    backupfile
    basename-lgpl
    binary-io
    bison
    bison-i18n
    byteswap
    c-ctype
    c-strcase
    c-strcasestr
    c-strstr
    clean-temp
    closedir
    closeout
    configmake
    copy-file
    csharpcomp
    csharpexec
    error
    error-progname
    execute
    filename
    findprog
    flexmember
    fnmatch
    fopen
    free-posix
    fstrcmp
    full-write
    fwriteerror
    gcd
    getaddrinfo
    getline
    getopt-gnu
    gettext
    gettext-h
    iconv
    javacomp
    javaexec
    libunistring-optional
    libxml
    localcharset
    locale
    localename
    localtime
    lock
    mem-hash-map
    memchr
    memmove
    memset
    minmax
    mkdir
    noreturn
    obstack
    open
    opendir
    openmp-init
    pipe-filter-ii
    progname
    propername
    read-file
    readdir
    relocatable-prog
    relocatable-script
    setlocale
    sh-filename
    sh-quote
    sigpipe
    sigprocmask
    spawn-pipe
    stdbool
    stdio
    stdlib
    stpcpy
    stpncpy
    strchrnul
    strcspn
    strerror
    string-desc
    strpbrk
    strtol
    strtoul
    supersede
    sys_select
    sys_stat
    sys_time
    trim
    unictype/ctype-space
    unictype/syntax-java-whitespace
    unilbrk/ulc-width-linebreaks
    uniname/uniname
    unistd
    unistr/u8-check
    unistr/u8-mbtouc
    unistr/u8-mbtoucr
    unistr/u8-uctomb
    unistr/u16-mbtouc
    uniwidth/width
    unlocked-io
    unsetenv
    vasprintf
    wait-process
    write
    xalloc
    xconcat-filename
    xerror
    xmalloca
    xmemdup0
    xsetenv
    xstriconv
    xstriconveh
    xvasprintf
  '
  # Common dependencies of GNULIB_MODULES_TOOLS_FOR_SRC and GNULIB_MODULES_TOOLS_FOR_LIBGREP.
  GNULIB_MODULES_TOOLS_FOR_SRC_COMMON_DEPENDENCIES='
    alloca-opt
    extensions
    gettext-h
    include_next
    locale
    localcharset
    malloc-posix
    mbrtowc
    mbsinit
    multiarch
    setlocale-null
    snippet/arg-nonnull
    snippet/c++defs
    snippet/warn-on-use
    ssize_t
    stdbool
    stddef
    stdint
    stdlib
    streq
    unistd
    verify
    wchar
    wctype-h
    windows-mutex
    windows-once
    windows-recmutex
    windows-rwlock
  '
  GNULIB_MODULES_TOOLS_OTHER='
    gettext-tools-misc
    ansi-c++-opt
    csharpcomp-script
    csharpexec-script
    java
    javacomp-script
    javaexec-script
    manywarnings
    stdint
  '
  GNULIB_MODULES_TOOLS_LIBUNISTRING_TESTS='
    unilbrk/u8-possible-linebreaks-tests
    unilbrk/ulc-width-linebreaks-tests
    unistr/u8-mbtouc-tests
    unistr/u8-mbtouc-unsafe-tests
    uniwidth/width-tests
  '
  $GNULIB_TOOL --dir=gettext-tools --lib=libgettextlib --source-base=gnulib-lib --m4-base=gnulib-m4 --tests-base=gnulib-tests --makefile-name=Makefile.gnulib --libtool --with-tests --local-dir=gnulib-local --local-symlink \
    --import \
    --avoid=fdutimensat-tests --avoid=futimens-tests --avoid=utime-tests --avoid=utimens-tests --avoid=utimensat-tests \
    --avoid=array-list-tests --avoid=linked-list-tests --avoid=linkedhash-list-tests \
    `for m in $GNULIB_MODULES_TOOLS_LIBUNISTRING_TESTS; do echo --avoid=$m; done` \
    $GNULIB_MODULES_TOOLS_FOR_SRC $GNULIB_MODULES_TOOLS_FOR_SRC_COMMON_DEPENDENCIES $GNULIB_MODULES_TOOLS_OTHER || exit $?
  $GNULIB_TOOL --copy-file m4/libtextstyle.m4 gettext-tools/gnulib-m4/libtextstyle.m4 || exit $?
  # In gettext-tools/libgrep:
  GNULIB_MODULES_TOOLS_FOR_LIBGREP='
    mbrlen
    regex
  '
  $GNULIB_TOOL --dir=gettext-tools --macro-prefix=grgl --lib=libgrep --source-base=libgrep --m4-base=libgrep/gnulib-m4 --witness-c-macro=IN_GETTEXT_TOOLS_LIBGREP --makefile-name=Makefile.gnulib --local-dir=gnulib-local --local-symlink \
    --import \
    `for m in $GNULIB_MODULES_TOOLS_FOR_SRC_COMMON_DEPENDENCIES; do \
       if test \`$GNULIB_TOOL --extract-applicability $m\` != all; then \
         case $m in \
           locale | stdbool | stddef | stdint | stdlib | unistd | wchar | wctype-h) ;; \
           *) echo --avoid=$m ;; \
         esac; \
       fi; \
     done` \
    $GNULIB_MODULES_TOOLS_FOR_LIBGREP || exit $?
  # In gettext-tools/libgettextpo:
  # This is a subset of the GNULIB_MODULES_TOOLS_FOR_SRC.
  GNULIB_MODULES_LIBGETTEXTPO='
    attribute
    basename-lgpl
    close
    c-ctype
    c-strcase
    c-strstr
    error
    error-progname
    filename
    fopen
    free-posix
    fstrcmp
    fwriteerror
    gcd
    getline
    gettext-h
    iconv
    libtextstyle-dummy
    libunistring-optional
    markup
    mem-hash-map
    minmax
    open
    relocatable-lib
    sigpipe
    stdbool
    stdio
    stdlib
    stpcpy
    stpncpy
    strchrnul
    strerror
    string-desc
    unictype/ctype-space
    unilbrk/ulc-width-linebreaks
    unistr/u8-mbtouc
    unistr/u8-mbtoucr
    unistr/u8-uctomb
    unistr/u16-mbtouc
    uniwidth/width
    unlocked-io
    vasprintf
    xalloc
    xconcat-filename
    xmalloca
    xerror
    xstriconv
    xvasprintf
  '
  # Module 'fdopen' is enabled in gettext-tools/config.status, because
  # it occurs as dependency of some module ('supersede') in
  # GNULIB_MODULES_TOOLS_FOR_SRC. Therefore on mingw, libgettextpo/stdio.h
  # contains '#define fdopen rpl_fdopen'. Therefore we need to include
  # fdopen.lo in libgettextpo.la.
  # Module 'realloc-posix' is enabled in gettext-tools/config.status, because
  # it occurs as dependency of some module ('read-file') in
  # GNULIB_MODULES_TOOLS_FOR_SRC. Therefore on mingw, libgettextpo/stdlib.h
  # contains '#define realloc rpl_realloc'. Therefore we need to include
  # realloc.lo in libgettextpo.la.
  GNULIB_MODULES_LIBGETTEXTPO_OTHER='
    fdopen
    realloc-posix
  '
  $GNULIB_TOOL --dir=gettext-tools --source-base=libgettextpo --m4-base=libgettextpo/gnulib-m4 --macro-prefix=gtpo --makefile-name=Makefile.gnulib --libtool --local-dir=gnulib-local --local-symlink \
    --import --avoid=progname $GNULIB_MODULES_LIBGETTEXTPO $GNULIB_MODULES_LIBGETTEXTPO_OTHER || exit $?
  # Overwrite older versions of .m4 files with the up-to-date version.
  cp gettext-runtime/m4/gettext.m4 gettext-tools/gnulib-m4/gettext.m4
  # Import build tools.  We use --copy-file to avoid directory creation.
  $GNULIB_TOOL --copy-file tests/init.sh gettext-tools || exit $?
  $GNULIB_TOOL --copy-file build-aux/x-to-1.in gettext-runtime/man/x-to-1.in || exit $?
  $GNULIB_TOOL --copy-file build-aux/x-to-1.in gettext-tools/man/x-to-1.in || exit $?
  $GNULIB_TOOL --copy-file build-aux/git-version-gen || exit $?
  $GNULIB_TOOL --copy-file build-aux/gitlog-to-changelog || exit $?
  $GNULIB_TOOL --copy-file build-aux/update-copyright || exit $?
  $GNULIB_TOOL --copy-file build-aux/useless-if-before-free || exit $?
  $GNULIB_TOOL --copy-file build-aux/vc-list-files || exit $?
  $GNULIB_TOOL --copy-file top/GNUmakefile . || exit $?
  $GNULIB_TOOL --copy-file top/maint.mk . || exit $?

  # Fetch config.guess, config.sub.
  for file in config.guess config.sub; do
    $GNULIB_TOOL --copy-file build-aux/$file && chmod a+x build-aux/$file || exit $?
  done
fi

# Make sure we get new versions of files brought in by automake.
(cd build-aux && rm -f ar-lib compile depcomp install-sh mdate-sh missing test-driver ylwrap)

# Generate configure script in each subdirectories.
# The aclocal and autoconf invocations need to be done bottom-up
# (subdirs first), so that 'configure --help' shows also the options
# that matter for the subdirs.
dir0=`pwd`

echo "$0: generating configure in gettext-runtime/intl..."
cd gettext-runtime/intl
aclocal -I ../../m4 -I ../m4 -I gnulib-m4 \
  && autoconf \
  && autoheader && touch config.h.in \
  && touch ChangeLog \
  && automake --add-missing --copy \
  && rm -rf autom4te.cache \
  || exit $?
cd "$dir0"

echo "$0: generating configure in gettext-runtime/libasprintf..."
cd gettext-runtime/libasprintf
aclocal -I ../../m4 -I ../m4 -I gnulib-m4 \
  && autoconf \
  && autoheader && touch config.h.in \
  && touch ChangeLog \
  && automake --add-missing --copy \
  && rm -rf autom4te.cache \
  || exit $?
cd "$dir0"

echo "$0: generating configure in gettext-runtime..."
cd gettext-runtime
aclocal -I m4 -I ../m4 -I gnulib-m4 \
  && autoconf \
  && autoheader && touch config.h.in \
  && touch ChangeLog \
  && automake --add-missing --copy \
  && rm -rf autom4te.cache \
  || exit $?
cd "$dir0"

echo "$0: generating files in libtextstyle..."
cd libtextstyle
(if ! $skip_gnulib; then export GNULIB_SRCDIR; fi
 ./autogen.sh $skip_gnulib_option
) || exit $?
cd "$dir0"

echo "$0: generating configure in gettext-tools/examples..."
cd gettext-tools/examples
aclocal -I ../../gettext-runtime/m4 -I ../../m4 \
  && autoconf \
  && touch ChangeLog \
  && automake --add-missing --copy \
  && rm -rf autom4te.cache \
  || exit $?
cd "$dir0"

echo "$0: copying common files from gettext-runtime to gettext-tools..."
cp -p gettext-runtime/ABOUT-NLS gettext-tools/ABOUT-NLS
cp -p gettext-runtime/po/Makefile.in.in gettext-tools/po/Makefile.in.in
cp -p gettext-runtime/po/Rules-quot gettext-tools/po/Rules-quot
cp -p gettext-runtime/po/boldquot.sed gettext-tools/po/boldquot.sed
cp -p gettext-runtime/po/quot.sed gettext-tools/po/quot.sed
cp -p gettext-runtime/po/en@quot.header gettext-tools/po/en@quot.header
cp -p gettext-runtime/po/en@boldquot.header gettext-tools/po/en@boldquot.header
cp -p gettext-runtime/po/insert-header.sin gettext-tools/po/insert-header.sin
cp -p gettext-runtime/po/remove-potcdate.sin gettext-tools/po/remove-potcdate.sin
# This file might be newer than Gnulib's.
sed_extract_serial='s/^#.* serial \([^ ]*\).*/\1/p
1q'
for file in po.m4; do
  existing_serial=`sed -n -e "$sed_extract_serial" < "gettext-tools/gnulib-m4/$file"`
  gettext_serial=`sed -n -e "$sed_extract_serial" < "gettext-runtime/m4/$file"`
  if test -n "$existing_serial" && test -n "$gettext_serial" \
        && test "$existing_serial" -ge "$gettext_serial" 2> /dev/null; then
    :
  else
    cp -p "gettext-runtime/m4/$file" "gettext-tools/gnulib-m4/$file"
  fi
done

echo "$0: generating configure in gettext-tools..."
cd gettext-tools
aclocal -I m4 -I ../gettext-runtime/m4 -I ../m4 -I gnulib-m4 -I libgrep/gnulib-m4 -I libgettextpo/gnulib-m4 \
  && autoconf \
  && autoheader && touch config.h.in \
  && touch ChangeLog \
  && automake --add-missing --copy \
  && rm -rf autom4te.cache \
  || exit $?
cd "$dir0"

echo "$0: generating configure at the top-level..."
aclocal -I m4 \
  && autoconf \
  && touch ChangeLog \
  && automake --add-missing --copy \
  && rm -rf autom4te.cache \
            gettext-runtime/autom4te.cache \
            gettext-runtime/intl/autom4te.cache \
            gettext-runtime/libasprintf/autom4te.cache \
            libtextstyle/autom4te.cache \
            gettext-tools/autom4te.cache \
            gettext-tools/examples/autom4te.cache \
  || exit $?

echo "$0: done.  Now you can run './configure'."
