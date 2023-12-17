# javacomp.m4 serial 26
dnl Copyright (C) 2001-2003, 2006-2007, 2009-2023 Free Software Foundation,
dnl Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

# Prerequisites of javacomp.sh.
# gt_JAVACOMP([source-version], [target-version])
# Sets HAVE_JAVACOMP to nonempty if javacomp.sh will allow Java source code
# according to source-version to be compiled to Java bytecode classes in the
# target-version format.
#
# source-version can be:    support for
#           1.6             assert keyword (1.4), generic classes and methods (1.5)
#           1.7             switch(string)
#           1.8             lambdas
#           9               private interface methods
#          10               type inference for local variables
#          11               'var' in parameters of lambda expressions
#          ...
# (For reference, see <https://en.wikipedia.org/wiki/Java_version_history>.)
# If source-version 1.3 or 1.4 or 1.5 is requested, it gets mapped to 1.6, for
# backward compatibility. (Currently the minimum Java and javac version we need
# to support is Java 1.6, since that's the default Java version on Solaris 10.)
#
# target-version can be:  classfile version:
#           1.6                 50.0
#           1.7                 51.0
#           1.8                 52.0
#           9                   53.0
#          10                   54.0
#          11                   55.0
#          ...                  ...
# The classfile version of a .class file can be determined through the "file"
# command. More portably, the classfile major version can be determined through
# "od -A n -t d1 -j 7 -N 1 classfile".
# If a target-version below 1.6 is requested, it gets mapped to 1.6, for
# backward compatibility. (Currently the minimum Java and javac version we need
# to support is Java 1.6, since that's the default Java version on Solaris 10.)
#
# target-version can also be omitted. In this case, the required target-version
# is determined from the found JVM (see macro gt_JAVAEXEC):
#      target-version   for JVM
#           1.6         JDK/JRE 6
#           1.7         JDK/JRE 7
#           1.8         JDK/JRE 8
#           9           JDK/JRE 9
#          10           JDK/JRE 10
#          11           JDK/JRE 11
#          ...          ...
#
# Specifying target-version is useful when building a library (.jar) that is
# useful outside the given package. Omitting target-version is useful when
# building an application.
#
# It is unreasonable to ask for a target-version < source-version, such as
#   - target-version < 1.4 with source-version >= 1.4, or
#   - target-version < 1.5 with source-version >= 1.5, or
#   - target_version < 1.6 with source_version >= 1.6, or
#   - target_version < 1.7 with source_version >= 1.7, or
#   - target_version < 1.8 with source_version >= 1.8, or
#   - target_version < 9 with source_version >= 9, or
#   - target_version < 10 with source_version >= 10, or
#   - target_version < 11 with source_version >= 11, or
#   - ...
# because even Sun's/Oracle's javac doesn't support these combinations.
#
# It is redundant to ask for a target-version > source-version, since the
# smaller target-version = source-version will also always work and newer JVMs
# support the older target-versions too.

AC_DEFUN([gt_JAVACOMP],
[
  m4_if([$2], [], [AC_REQUIRE([gt_JAVAEXEC])], [])
  AC_EGREP_CPP([yes], [
#if defined _WIN32 || defined __CYGWIN__ || defined __EMX__ || defined __DJGPP__
  yes
#endif
], CLASSPATH_SEPARATOR=';', CLASSPATH_SEPARATOR=':')
  source_version=$1
  test -n "$source_version" || {
    AC_MSG_ERROR([missing source-version argument to gt_@&t@JAVACOMP])
  }
  case "$source_version" in
    1.1 | 1.2 | 1.3 | 1.4 | 1.5) source_version='1.6' ;;
  esac
  m4_if([$2], [],
    [if test -n "$HAVE_JAVAEXEC"; then
       dnl Use $CONF_JAVA to determine the JVM's version.
changequote(,)dnl
       cat > conftestver.java <<EOF
public class conftestver {
  public static void main (String[] args) {
    System.out.println(System.getProperty("java.specification.version"));
  }
}
EOF
changequote([,])dnl
       dnl A precompiled version of conftestver.java, compiled with
       dnl "javac -target 1.1". This avoids having to compile conftestver.java
       dnl during each test for a suitable Java compiler.
       dnl For the conversion from binary to this ASCII encapsulation, avoiding
       dnl to assume the presence of uudecode, use the command
       dnl   $ od -A n -t o1 < conftestver.class | tr ' ' '\012'| sort | uniq | sed -e '/^$/d' -e 's,^,\\,' | tr -d '\012'
       dnl and the long tr command in opposite direction.
       dnl Finally move the position corresponding to \055 to the last position,
       dnl to work around a coreutils-5.x bug.
       echo 'yzwx!$!I!D,!)!3+!4!5*!6,!4!7,!8!9)!:)!;"!(MeienN"!$FGW"!%Ojab"!2QeibRohZblVYZgb"!%hYei"!9FXQfYpYKgYidKUnleidLGW"!,Ujol_bPegb"!3_jicnbmnpblJfYpY/!*!+)!</!=!>"!=fYpYJmkb_ece_YnejiJpblmeji/!?!@)!A/!B!C"!._jicnbmnpbl"!3fYpYKgYidKSZfb_n"!3fYpYKgYidKUqmnbh"!$jon"!8QfYpYKejKTleinUnlbYhL"!.dbnTljkblnq"!EFQfYpYKgYidKUnleidLGQfYpYKgYidKUnleidL"!6fYpYKejKTleinUnlbYh"!)kleingi"!8FQfYpYKgYidKUnleidLGW!D!(!)!!!!!#!"!*!+!"!,!!!@!"!"!!!&Hu!"r!!!"!.!!!(!"!!!"!+!/!0!"!,!!!F!#!"!!!/s!#5$v!%t!&r!!!"!.!!!,!#!!!$!.!%!"!1!!!#!2' \
         | tr -d '\012\015' \
         | tr '!"#$%&()*+,./0123456789:;<=>?@ABCDEFGHJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyzI' '\000\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017\020\021\022\023\024\025\026\027\030\031\032\033\034\035\036\037\040\041\046\050\051\052\056\057\073\074\076\103\106\114\116\117\120\123\124\126\133\141\142\143\144\145\146\147\151\152\154\155\156\157\160\162\163\164\165\166\171\261\262\266\267\270\272\276\312\376\055' \
         > conftestver.class
       java_exec_version=`{
         unset JAVA_HOME
         echo "$as_me:__oline__: CLASSPATH=.${CLASSPATH:+$CLASSPATH_SEPARATOR$CLASSPATH} $CONF_JAVA conftestver" >&AS_MESSAGE_LOG_FD
         CLASSPATH=.${CLASSPATH:+$CLASSPATH_SEPARATOR$CLASSPATH} $CONF_JAVA conftestver 2>&AS_MESSAGE_LOG_FD
       }`
       case "$java_exec_version" in
         null)
           dnl JDK 1.1.X returns null.
           java_exec_version=1.1 ;;
       esac
       case "$java_exec_version" in
         1.1 | 1.2 | 1.3 | 1.4 | 1.5)
           AC_MSG_WARN([$CONF_JAVA is too old, cannot compile Java code for this old version any more])
           target_version=1.6 ;;
changequote(,)dnl
         1.6 | 1.7 | 1.8 | 9 | [1-9][0-9])
changequote([,])dnl
           dnl Here we could choose any target_version between $source_version
           dnl and the $java_exec_version. (If it is too small, it will be
           dnl incremented below until it works.) Since we documented above that
           dnl it is determined from the JVM, we do that:
           target_version="$java_exec_version" ;;
         *) AC_MSG_WARN([unknown target-version $target_version, please update gt_@&t@JAVACOMP macro])
            target_version=1.6 ;;
       esac
     else
       target_version="1.6"
     fi
    ],
    [target_version=$2
     case "$target_version" in
       1.1 | 1.2 | 1.3 | 1.4 | 1.5) target_version='1.6' ;;
     esac
    ])
  case "$source_version" in
changequote(,)dnl
    1.6 | 1.7 | 1.8 | 9 | [1-9][0-9]) ;;
changequote([,])dnl
    *) AC_MSG_ERROR([invalid source-version argument to gt_@&t@JAVACOMP: $source_version]) ;;
  esac
  case "$target_version" in
changequote(,)dnl
    1.6 | 1.7 | 1.8 | 9 | [1-9][0-9]) ;;
changequote([,])dnl
    *) AC_MSG_ERROR([invalid target-version argument to gt_@&t@JAVACOMP: $target_version]) ;;
  esac
  # Function to output the classfile version of a file (8th byte) in decimal.
  if od -A x < /dev/null >/dev/null 2>/dev/null; then
    # Use POSIX od.
    func_classfile_version ()
    {
      od -A n -t d1 -j 7 -N 1 "[$]1"
    }
  else
    # Use BSD hexdump.
    func_classfile_version ()
    {
      dd if="[$]1" bs=1 count=1 skip=7 2>/dev/null | hexdump -e '1/1 "%3d "'
      echo
    }
  fi
  AC_MSG_CHECKING([for Java compiler])
  dnl
  dnl The support of Sun/Oracle javac for target-version and source-version:
  dnl
  dnl   javac 1.6:   -target 1.1 1.2 1.3 1.4 1.5 1.6   default: 1.6
  dnl                -source 1.3 1.4 1.5 1.6           default: 1.5
  dnl                -target 1.1/1.2/1.3 only possible with -source 1.3
  dnl                -target 1.4 only possible with -source 1.3/1.4
  dnl                -target 1.5 only possible with -source 1.3/1.4/1.5 or no -source
  dnl
  dnl   javac 1.7:   -target 1.1 1.2 1.3 1.4 1.5 1.6 1.7  default: 1.7
  dnl                -source 1.3 1.4 1.5 1.6 1.7          default: 1.7
  dnl                -target 1.1/1.2/1.3 only possible with -source 1.3
  dnl                -target 1.4 only possible with -source 1.3/1.4
  dnl                -target 1.5 only possible with -source 1.3/1.4/1.5
  dnl                -target 1.6 only possible with -source 1.3/1.4/1.5/1.6
  dnl
  dnl   javac 1.8:   -target 1.1 1.2 1.3 1.4 1.5 1.6 1.7 1.8  default: 1.8
  dnl                -source 1.3 1.4 1.5 1.6 1.7 1.8          default: 1.8
  dnl                -target 1.1/1.2/1.3 only possible with -source 1.3
  dnl                -target 1.4 only possible with -source 1.3/1.4
  dnl                -target 1.5 only possible with -source 1.3/1.4/1.5
  dnl                -target 1.6 only possible with -source 1.3/1.4/1.5/1.6
  dnl                -target 1.7 only possible with -source 1.3/1.4/1.5/1.6/1.7
  dnl
  dnl   javac 9:     -target 1.6 1.7 1.8 9  default: 9
  dnl                -source 1.6 1.7 1.8 9  default: 9
  dnl                -target 1.6 only possible with -source 1.6
  dnl                -target 1.7 only possible with -source 1.6/1.7
  dnl                -target 1.8 only possible with -source 1.6/1.7/1.8
  dnl
  dnl   javac 10:    -target 1.6 1.7 1.8 9 10  default: 10
  dnl                -source 1.6 1.7 1.8 9 10  default: 10
  dnl                -target 1.6 only possible with -source 1.6
  dnl                -target 1.7 only possible with -source 1.6/1.7
  dnl                -target 1.8 only possible with -source 1.6/1.7/1.8
  dnl                -target 9 only possible with -source 1.6/1.7/1.8/9
  dnl
  dnl   and so on.
  dnl   This can be summarized in this table:
  dnl
  dnl     javac     classfile         valid -source and   obsolete -source
  dnl     version   default version   -target values      and -target values
  dnl     -------   ---------------   -----------------   ------------------
  dnl     1.6       50.0              1.2 .. 1.6
  dnl     1.7       51.0              1.2 .. 1.7
  dnl     1.8       52.0              1.3 .. 1.8          1.3 .. 1.5
  dnl     9         53.0              1.6 .. 9            1.6
  dnl     10        54.0              1.6 .. 10           1.6
  dnl     11        55.0              1.6 .. 11           1.6
  dnl     12        56.0              1.7 .. 12           1.7
  dnl     13        57.0              1.7 .. 13           1.7
  dnl     14        58.0              1.7 .. 14           1.7
  dnl     15        59.0              1.7 .. 15           1.7
  dnl     16        60.0              1.7 .. 16           1.7
  dnl     17        61.0              1.7 .. 17           1.7
  dnl     18        62.0              1.7 .. 18           1.7
  dnl     19        63.0              1.7 .. 19           1.7
  dnl     20        64.0              1.8 .. 20           1.8
  dnl
  dnl   The -source option value must be <= the -target option value.
  dnl   The minimal -source and -target option value produces an "is obsolete"
  dnl   warning (in javac 1.8 or newer). Additionally, if the -source option
  dnl   value is not the maximal possible one, i.e. not redundant, it produces a
  dnl   "bootstrap class path not set in conjunction with -source ..." warning
  dnl   (in javac 1.7 or newer).
  dnl
  dnl   To get rid of these warnings, two options are available:
  dnl     * -nowarn. This option is supported since javac 1.6 at least. But
  dnl       it is overkill, because it would also silence warnings about the
  dnl       code being compiled.
  dnl     * -Xlint:-options. This option is supported since javac 1.6 at least.
  dnl       In javac 1.6 it is an undocumented no-op.
  dnl   We use -Xlint:-options and omit it only if we find that the compiler
  dnl   does not support it (which is unlikely).
  dnl
  dnl Canonicalize source_version and target_version, for easier arithmetic.
  case "$source_version" in
    1.*) source_version=`echo "$source_version" | sed -e 's/^1\.//'` ;;
  esac
  case "$target_version" in
    1.*) target_version=`echo "$target_version" | sed -e 's/^1\.//'` ;;
  esac
  CONF_JAVAC=
  HAVE_JAVAC_ENVVAR=
  HAVE_JAVAC=
  HAVE_JAVACOMP=
  dnl Sanity check.
  if expr $source_version '<=' $target_version >/dev/null; then
    echo 'class conftest {}' > conftest.java
    dnl If the user has set the JAVAC environment variable, use that, if it
    dnl satisfies the constraints (possibly after adding -target and -source
    dnl options).
    if test -n "$JAVAC"; then
      dnl Test whether $JAVAC is usable.
      dnl At the same time, determine which option to use to inhibit warnings;
      dnl see the discussion above.
      nowarn_option=' -Xlint:-options'
      if { rm -f conftest.class \
           && { echo "$as_me:__oline__: $JAVAC$nowarn_option -d . conftest.java" >&AS_MESSAGE_LOG_FD
                $JAVAC$nowarn_option -d . conftest.java >&AS_MESSAGE_LOG_FD 2>&1
              } \
           && test -f conftest.class
         } || { \
           nowarn_option=
           rm -f conftest.class \
           && { echo "$as_me:__oline__: $JAVAC$nowarn_option -d . conftest.java" >&AS_MESSAGE_LOG_FD
                $JAVAC$nowarn_option -d . conftest.java >&AS_MESSAGE_LOG_FD 2>&1
              } \
           && test -f conftest.class
         }; then
        compiler_cfversion=`func_classfile_version conftest.class`
        compiler_target_version=`expr $compiler_cfversion - 44`
        dnl It is hard to determine the compiler_source_version. This would
        dnl require a list of code snippets that can be compiled only with a
        dnl specific '-source' option and up, and this list would need to grow
        dnl every 6 months.
        dnl Also, $JAVAC may already include a '-source' option.
        dnl Therefore, pass a '-source' option always.
        source_option=' -source '`case "$source_version" in 6|7|8) echo 1. ;; esac`"$source_version"
        dnl And pass a '-target' option as well, if needed.
        dnl (All supported javac versions support both, see the table above.)
        if expr $target_version = $compiler_target_version >/dev/null; then
          target_option=
        else
          target_option=' -target '`case "$target_version" in 6|7|8) echo 1. ;; esac`"$target_version"
        fi
        if { echo "$as_me:__oline__: $JAVAC$nowarn_option$source_option$target_option -d . conftest.java" >&AS_MESSAGE_LOG_FD
             $JAVAC$nowarn_option$source_option$target_option -d . conftest.java >&AS_MESSAGE_LOG_FD 2>&1
           } \
           && test -f conftest.class; then
          dnl The compiler directly supports the desired source_version and
          dnl target_version. Perfect.
          CONF_JAVAC="$JAVAC$nowarn_option$source_option$target_option"
          HAVE_JAVAC_ENVVAR=1
          HAVE_JAVACOMP=1
        else
          dnl If the desired source_version or target_version were too large
          dnl for the compiler, there's nothing else we can do.
          compiler_version=`echo "$as_me:__oline__: $JAVAC -version | sed -e 1q" >&AS_MESSAGE_LOG_FD
                            $JAVAC -version | sed -e 1q`
changequote(,)dnl
          compiler_version=`echo "$compiler_version" | sed -e 's/^[^0-9]*\([0-9][0-9.]*\).*/\1/'`
changequote([,])dnl
          case "$compiler_version" in
            1.*) dnl Map 1.6.0_85 to 6, 1.8.0_151 to 8.
                 compiler_version=`echo "$compiler_version" | sed -e 's/^1\.//' -e 's/\..*//'`
                 ;;
            *) dnl Map 9.0.4 to 9, 10.0.2 to 10, etc.
               compiler_version=`echo "$compiler_version" | sed -e 's/\..*//'`
               ;;
          esac
          if expr $source_version '<=' "$compiler_version" >/dev/null \
             && expr $target_version '<=' "$compiler_version" >/dev/null; then
            dnl Increase $source_version and $compiler_version until the
            dnl compiler accepts these values. This is necessary to make
            dnl e.g. $source_version = 6 work with Java 12 or newer, or
            dnl $source_version = 7 work with Java 20 or newer.
            try_source_version="$source_version"
            try_target_version="$target_version"
            while true; do
              dnl Invariant: $try_source_version <= $try_target_version.
              if expr $try_source_version = $try_target_version >/dev/null; then
                try_target_version=`expr $try_target_version + 1`
              fi
              try_source_version=`expr $try_source_version + 1`
              expr $try_source_version '<=' $compiler_version >/dev/null || break
              source_option=' -source '`case "$try_source_version" in 6|7|8) echo 1. ;; esac`"$try_source_version"
              if expr $try_target_version = $compiler_target_version >/dev/null; then
                target_option=
              else
                target_option=' -target '`case "$try_target_version" in 6|7|8) echo 1. ;; esac`"$try_target_version"
              fi
              if { echo "$as_me:__oline__: $JAVAC$nowarn_option$source_option$target_option -d . conftest.java" >&AS_MESSAGE_LOG_FD
                   $JAVAC$nowarn_option$source_option$target_option -d . conftest.java >&AS_MESSAGE_LOG_FD 2>&1
                 } \
                 && test -f conftest.class; then
                dnl The compiler supports the try_source_version and
                dnl try_target_version. It's better than nothing.
                CONF_JAVAC="$JAVAC$nowarn_option$source_option$target_option"
                HAVE_JAVAC_ENVVAR=1
                HAVE_JAVACOMP=1
                break
              fi
            done
          fi
        fi
      fi
    fi
    if test -z "$HAVE_JAVACOMP"; then
      pushdef([AC_MSG_CHECKING],[:])dnl
      pushdef([AC_CHECKING],[:])dnl
      pushdef([AC_MSG_RESULT],[:])dnl
      AC_CHECK_PROG([HAVE_JAVAC_IN_PATH], [javac], [yes])
      popdef([AC_MSG_RESULT])dnl
      popdef([AC_CHECKING])dnl
      popdef([AC_MSG_CHECKING])dnl
      if test -z "$HAVE_JAVACOMP" && test -n "$HAVE_JAVAC_IN_PATH"; then
        dnl Test whether javac is usable.
        dnl At the same time, determine which option to use to inhibit warnings;
        dnl see the discussion above.
        nowarn_option=' -Xlint:-options'
        if { rm -f conftest.class \
             && { echo "$as_me:__oline__: javac$nowarn_option -d . conftest.java" >&AS_MESSAGE_LOG_FD
                  javac$nowarn_option -d . conftest.java >&AS_MESSAGE_LOG_FD 2>&1
                } \
             && test -f conftest.class
           } || { \
             nowarn_option=
             rm -f conftest.class \
             && { echo "$as_me:__oline__: javac$nowarn_option -d . conftest.java" >&AS_MESSAGE_LOG_FD
                  javac$nowarn_option -d . conftest.java >&AS_MESSAGE_LOG_FD 2>&1
                } \
             && test -f conftest.class
           }; then
          compiler_cfversion=`func_classfile_version conftest.class`
          compiler_target_version=`expr $compiler_cfversion - 44`
          dnl It is hard to determine the compiler_source_version. This would
          dnl require a list of code snippets that can be compiled only with a
          dnl specific '-source' option and up, and this list would need to grow
          dnl every 6 months.
          dnl Also, javac may point to a shell script that already includes a
          dnl '-source' option.
          dnl Therefore, pass a '-source' option always.
          source_option=' -source '`case "$source_version" in 6|7|8) echo 1. ;; esac`"$source_version"
          dnl And pass a '-target' option as well, if needed.
          dnl (All supported javac versions support both, see the table above.)
          if expr $target_version = $compiler_target_version >/dev/null; then
            target_option=
          else
            target_option=' -target '`case "$target_version" in 6|7|8) echo 1. ;; esac`"$target_version"
          fi
          if { echo "$as_me:__oline__: javac$nowarn_option$source_option$target_option -d . conftest.java" >&AS_MESSAGE_LOG_FD
               javac$nowarn_option$source_option$target_option -d . conftest.java >&AS_MESSAGE_LOG_FD 2>&1
             } \
             && test -f conftest.class; then
            dnl The compiler directly supports the desired source_version and
            dnl target_version. Perfect.
            CONF_JAVAC="javac$nowarn_option$source_option$target_option"
            HAVE_JAVAC=1
            HAVE_JAVACOMP=1
          else
            dnl If the desired source_version or target_version were too large
            dnl for the compiler, there's nothing else we can do.
            compiler_version=`echo "$as_me:__oline__: javac -version | sed -e 1q" >&AS_MESSAGE_LOG_FD
                              javac -version | sed -e 1q`
changequote(,)dnl
            compiler_version=`echo "$compiler_version" | sed -e 's/^[^0-9]*\([0-9][0-9.]*\).*/\1/'`
changequote([,])dnl
            case "$compiler_version" in
              1.*) dnl Map 1.6.0_85 to 6, 1.8.0_151 to 8.
                   compiler_version=`echo "$compiler_version" | sed -e 's/^1\.//' -e 's/\..*//'`
                   ;;
              *) dnl Map 9.0.4 to 9, 10.0.2 to 10, etc.
                 compiler_version=`echo "$compiler_version" | sed -e 's/\..*//'`
                 ;;
            esac
            if expr $source_version '<=' "$compiler_version" >/dev/null \
               && expr $target_version '<=' "$compiler_version" >/dev/null; then
              dnl Increase $source_version and $compiler_version until the
              dnl compiler accepts these values. This is necessary to make
              dnl e.g. $source_version = 6 work with Java 12 or newer, or
              dnl $source_version = 7 work with Java 20 or newer.
              try_source_version="$source_version"
              try_target_version="$target_version"
              while true; do
                dnl Invariant: $try_source_version <= $try_target_version.
                if expr $try_source_version = $try_target_version >/dev/null; then
                  try_target_version=`expr $try_target_version + 1`
                fi
                try_source_version=`expr $try_source_version + 1`
                expr $try_source_version '<=' $compiler_version >/dev/null || break
                source_option=' -source '`case "$try_source_version" in 6|7|8) echo 1. ;; esac`"$try_source_version"
                if expr $try_target_version = $compiler_target_version >/dev/null; then
                  target_option=
                else
                  target_option=' -target '`case "$try_target_version" in 6|7|8) echo 1. ;; esac`"$try_target_version"
                fi
                if { echo "$as_me:__oline__: javac$nowarn_option$source_option$target_option -d . conftest.java" >&AS_MESSAGE_LOG_FD
                     javac$nowarn_option$source_option$target_option -d . conftest.java >&AS_MESSAGE_LOG_FD 2>&1
                   } \
                   && test -f conftest.class; then
                  dnl The compiler supports the try_source_version and
                  dnl try_target_version. It's better than nothing.
                  CONF_JAVAC="javac$nowarn_option$source_option$target_option"
                  HAVE_JAVAC=1
                  HAVE_JAVACOMP=1
                  break
                fi
              done
            fi
          fi
        fi
      fi
    fi
    rm -f conftest*.java conftest*.class
  fi
  if test -n "$HAVE_JAVACOMP"; then
    ac_result="$CONF_JAVAC"
  else
    ac_result="no"
  fi
  AC_MSG_RESULT([$ac_result])
  AC_SUBST([CONF_JAVAC])
  AC_SUBST([CLASSPATH])
  AC_SUBST([CLASSPATH_SEPARATOR])
  AC_SUBST([HAVE_JAVAC_ENVVAR])
  AC_SUBST([HAVE_JAVAC])
])

# Simulates gt_JAVACOMP when no Java support is desired.
AC_DEFUN([gt_JAVACOMP_DISABLED],
[
  CONF_JAVAC=
  HAVE_JAVAC_ENVVAR=
  HAVE_JAVAC=
  AC_SUBST([CONF_JAVAC])
  AC_SUBST([HAVE_JAVAC_ENVVAR])
  AC_SUBST([HAVE_JAVAC])
])
