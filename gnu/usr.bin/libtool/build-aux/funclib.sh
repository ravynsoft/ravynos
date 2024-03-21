# Set a version string for this script.
scriptversion=2019-02-19.15; # UTC

# General shell script boiler plate, and helper functions.
# Written by Gary V. Vaughan, 2004

# This is free software.  There is NO warranty; not even for
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# Copyright (C) 2004-2019, 2021 Bootstrap Authors
#
# This file is dual licensed under the terms of the MIT license
# <https://opensource.org/license/MIT>, and GPL version 2 or later
# <http://www.gnu.org/licenses/gpl-2.0.html>.  You must apply one of
# these licenses when using or redistributing this software or any of
# the files within it.  See the URLs above, or the file `LICENSE`
# included in the Bootstrap distribution for the full license texts.

# Please report bugs or propose patches to:
# <https://github.com/gnulib-modules/bootstrap/issues>


## ------ ##
## Usage. ##
## ------ ##

# Evaluate this file near the top of your script to gain access to
# the functions and variables defined here:
#
#   . `echo "$0" | ${SED-sed} 's|[^/]*$||'`/build-aux/funclib.sh
#
# If you need to override any of the default environment variable
# settings, do that before evaluating this file.


## -------------------- ##
## Shell normalisation. ##
## -------------------- ##

# Some shells need a little help to be as Bourne compatible as possible.
# Before doing anything else, make sure all that help has been provided!

DUALCASE=1; export DUALCASE # for MKS sh
if test -n "${ZSH_VERSION+set}" && (emulate sh) >/dev/null 2>&1; then :
  emulate sh
  NULLCMD=:
  # Pre-4.2 versions of Zsh do word splitting on ${1+"$@"}, which
  # is contrary to our usage.  Disable this feature.
  alias -g '${1+"$@"}'='"$@"'
  setopt NO_GLOB_SUBST
else
  case `(set -o) 2>/dev/null` in *posix*) set -o posix ;; esac
fi

# NLS nuisances: We save the old values in case they are required later.
_G_user_locale=
_G_safe_locale=
for _G_var in LANG LANGUAGE LC_ALL LC_CTYPE LC_COLLATE LC_MESSAGES
do
  eval "if test set = \"\${$_G_var+set}\"; then
          save_$_G_var=\$$_G_var
          $_G_var=C
	  export $_G_var
	  _G_user_locale=\"$_G_var=\\\$save_\$_G_var; \$_G_user_locale\"
	  _G_safe_locale=\"$_G_var=C; \$_G_safe_locale\"
	fi"
done
# These NLS vars are set unconditionally (bootstrap issue #24).  Unset those
# in case the environment reset is needed later and the $save_* variant is not
# defined (see the code above).
LC_ALL=C
LANGUAGE=C
export LANGUAGE LC_ALL

# Make sure IFS has a sensible default
sp=' '
nl='
'
IFS="$sp	$nl"

# There are apparently some retarded systems that use ';' as a PATH separator!
if test "${PATH_SEPARATOR+set}" != set; then
  PATH_SEPARATOR=:
  (PATH='/bin;/bin'; FPATH=$PATH; sh -c :) >/dev/null 2>&1 && {
    (PATH='/bin:/bin'; FPATH=$PATH; sh -c :) >/dev/null 2>&1 ||
      PATH_SEPARATOR=';'
  }
fi


# func_unset VAR
# --------------
# Portably unset VAR.
# In some shells, an 'unset VAR' statement leaves a non-zero return
# status if VAR is already unset, which might be problematic if the
# statement is used at the end of a function (thus poisoning its return
# value) or when 'set -e' is active (causing even a spurious abort of
# the script in this case).
func_unset ()
{
    { eval $1=; (eval unset $1) >/dev/null 2>&1 && eval unset $1 || : ; }
}


# Make sure CDPATH doesn't cause `cd` commands to output the target dir.
func_unset CDPATH

# Make sure ${,E,F}GREP behave sanely.
func_unset GREP_OPTIONS


## ------------------------- ##
## Locate command utilities. ##
## ------------------------- ##


# func_executable_p FILE
# ----------------------
# Check that FILE is an executable regular file.
func_executable_p ()
{
    test -f "$1" && test -x "$1"
}


# func_path_progs PROGS_LIST CHECK_FUNC [PATH]
# --------------------------------------------
# Search for either a program that responds to --version with output
# containing "GNU", or else returned by CHECK_FUNC otherwise, by
# trying all the directories in PATH with each of the elements of
# PROGS_LIST.
#
# CHECK_FUNC should accept the path to a candidate program, and
# set $func_check_prog_result if it truncates its output less than
# $_G_path_prog_max characters.
func_path_progs ()
{
    _G_progs_list=$1
    _G_check_func=$2
    _G_PATH=${3-"$PATH"}

    _G_path_prog_max=0
    _G_path_prog_found=false
    _G_save_IFS=$IFS; IFS=${PATH_SEPARATOR-:}
    for _G_dir in $_G_PATH; do
      IFS=$_G_save_IFS
      test -z "$_G_dir" && _G_dir=.
      for _G_prog_name in $_G_progs_list; do
        for _exeext in '' .EXE; do
          _G_path_prog=$_G_dir/$_G_prog_name$_exeext
          func_executable_p "$_G_path_prog" || continue
          case `"$_G_path_prog" --version 2>&1` in
            *GNU*) func_path_progs_result=$_G_path_prog _G_path_prog_found=: ;;
            *)     $_G_check_func $_G_path_prog
		   func_path_progs_result=$func_check_prog_result
		   ;;
          esac
          $_G_path_prog_found && break 3
        done
      done
    done
    IFS=$_G_save_IFS
    test -z "$func_path_progs_result" && {
      echo "no acceptable sed could be found in \$PATH" >&2
      exit 1
    }
}


# We want to be able to use the functions in this file before configure
# has figured out where the best binaries are kept, which means we have
# to search for them ourselves - except when the results are already set
# where we skip the searches.

# Unless the user overrides by setting SED, search the path for either GNU
# sed, or the sed that truncates its output the least.
test -z "$SED" && {
  _G_sed_script=s/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa/bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb/
  for _G_i in 1 2 3 4 5 6 7; do
    _G_sed_script=$_G_sed_script$nl$_G_sed_script
  done
  echo "$_G_sed_script" 2>/dev/null | sed 99q >conftest.sed
  _G_sed_script=

  func_check_prog_sed ()
  {
    _G_path_prog=$1

    _G_count=0
    printf 0123456789 >conftest.in
    while :
    do
      cat conftest.in conftest.in >conftest.tmp
      mv conftest.tmp conftest.in
      cp conftest.in conftest.nl
      echo '' >> conftest.nl
      "$_G_path_prog" -f conftest.sed <conftest.nl >conftest.out 2>/dev/null || break
      diff conftest.out conftest.nl >/dev/null 2>&1 || break
      _G_count=`expr $_G_count + 1`
      if test "$_G_count" -gt "$_G_path_prog_max"; then
        # Best one so far, save it but keep looking for a better one
        func_check_prog_result=$_G_path_prog
        _G_path_prog_max=$_G_count
      fi
      # 10*(2^10) chars as input seems more than enough
      test 10 -lt "$_G_count" && break
    done
    rm -f conftest.in conftest.tmp conftest.nl conftest.out
  }

  func_path_progs "sed gsed" func_check_prog_sed "$PATH:/usr/xpg4/bin"
  rm -f conftest.sed
  SED=$func_path_progs_result
}


# Unless the user overrides by setting GREP, search the path for either GNU
# grep, or the grep that truncates its output the least.
test -z "$GREP" && {
  func_check_prog_grep ()
  {
    _G_path_prog=$1

    _G_count=0
    _G_path_prog_max=0
    printf 0123456789 >conftest.in
    while :
    do
      cat conftest.in conftest.in >conftest.tmp
      mv conftest.tmp conftest.in
      cp conftest.in conftest.nl
      echo 'GREP' >> conftest.nl
      "$_G_path_prog" -e 'GREP$' -e '-(cannot match)-' <conftest.nl >conftest.out 2>/dev/null || break
      diff conftest.out conftest.nl >/dev/null 2>&1 || break
      _G_count=`expr $_G_count + 1`
      if test "$_G_count" -gt "$_G_path_prog_max"; then
        # Best one so far, save it but keep looking for a better one
        func_check_prog_result=$_G_path_prog
        _G_path_prog_max=$_G_count
      fi
      # 10*(2^10) chars as input seems more than enough
      test 10 -lt "$_G_count" && break
    done
    rm -f conftest.in conftest.tmp conftest.nl conftest.out
  }

  func_path_progs "grep ggrep" func_check_prog_grep "$PATH:/usr/xpg4/bin"
  GREP=$func_path_progs_result
}


## ------------------------------- ##
## User overridable command paths. ##
## ------------------------------- ##

# All uppercase variable names are used for environment variables.  These
# variables can be overridden by the user before calling a script that
# uses them if a suitable command of that name is not already available
# in the command search PATH.

: ${CP="cp -f"}
: ${ECHO="printf %s\n"}
: ${EGREP="$GREP -E"}
: ${FGREP="$GREP -F"}
: ${LN_S="ln -s"}
: ${MAKE="make"}
: ${MKDIR="mkdir"}
: ${MV="mv -f"}
: ${RM="rm -f"}
: ${SHELL="${CONFIG_SHELL-/bin/sh}"}


## -------------------- ##
## Useful sed snippets. ##
## -------------------- ##

sed_dirname='s|/[^/]*$||'
sed_basename='s|^.*/||'

# Sed substitution that helps us do robust quoting.  It backslashifies
# metacharacters that are still active within double-quoted strings.
sed_quote_subst='s|\([`"$\\]\)|\\\1|g'

# Same as above, but do not quote variable references.
sed_double_quote_subst='s/\(["`\\]\)/\\\1/g'

# Sed substitution that turns a string into a regex matching for the
# string literally.
sed_make_literal_regex='s|[].[^$\\*\/]|\\&|g'

# Sed substitution that converts a w32 file name or path
# that contains forward slashes, into one that contains
# (escaped) backslashes.  A very naive implementation.
sed_naive_backslashify='s|\\\\*|\\|g;s|/|\\|g;s|\\|\\\\|g'

# Re-'\' parameter expansions in output of sed_double_quote_subst that
# were '\'-ed in input to the same.  If an odd number of '\' preceded a
# '$' in input to sed_double_quote_subst, that '$' was protected from
# expansion.  Since each input '\' is now two '\'s, look for any number
# of runs of four '\'s followed by two '\'s and then a '$'.  '\' that '$'.
_G_bs='\\'
_G_bs2='\\\\'
_G_bs4='\\\\\\\\'
_G_dollar='\$'
sed_double_backslash="\
  s/$_G_bs4/&\\
/g
  s/^$_G_bs2$_G_dollar/$_G_bs&/
  s/\\([^$_G_bs]\\)$_G_bs2$_G_dollar/\\1$_G_bs2$_G_bs$_G_dollar/g
  s/\n//g"

# require_check_ifs_backslash
# ---------------------------
# Check if we can use backslash as IFS='\' separator, and set
# $check_ifs_backshlash_broken to ':' or 'false'.
require_check_ifs_backslash=func_require_check_ifs_backslash
func_require_check_ifs_backslash ()
{
  _G_save_IFS=$IFS
  IFS='\'
  _G_check_ifs_backshlash='a\\b'
  for _G_i in $_G_check_ifs_backshlash
  do
  case $_G_i in
  a)
    check_ifs_backshlash_broken=false
    ;;
  '')
    break
    ;;
  *)
    check_ifs_backshlash_broken=:
    break
    ;;
  esac
  done
  IFS=$_G_save_IFS
  require_check_ifs_backslash=:
}


## ----------------- ##
## Global variables. ##
## ----------------- ##

# Except for the global variables explicitly listed below, the following
# functions in the '^func_' namespace, and the '^require_' namespace
# variables initialised in the 'Resource management' section, sourcing
# this file will not pollute your global namespace with anything
# else. There's no portable way to scope variables in Bourne shell
# though, so actually running these functions will sometimes place
# results into a variable named after the function, and often use
# temporary variables in the '^_G_' namespace. If you are careful to
# avoid using those namespaces casually in your sourcing script, things
# should continue to work as you expect. And, of course, you can freely
# overwrite any of the functions or variables defined here before
# calling anything to customize them.

EXIT_SUCCESS=0
EXIT_FAILURE=1
EXIT_MISMATCH=63  # $? = 63 is used to indicate version mismatch to missing.
EXIT_SKIP=77	  # $? = 77 is used to indicate a skipped test to automake.

# Allow overriding, eg assuming that you follow the convention of
# putting '$debug_cmd' at the start of all your functions, you can get
# bash to show function call trace with:
#
#    debug_cmd='eval echo "${FUNCNAME[0]} $*" >&2' bash your-script-name
debug_cmd=${debug_cmd-":"}
exit_cmd=:

# By convention, finish your script with:
#
#    exit $exit_status
#
# so that you can set exit_status to non-zero if you want to indicate
# something went wrong during execution without actually bailing out at
# the point of failure.
exit_status=$EXIT_SUCCESS

# Work around backward compatibility issue on IRIX 6.5. On IRIX 6.4+, sh
# is ksh but when the shell is invoked as "sh" and the current value of
# the _XPG environment variable is not equal to 1 (one), the special
# positional parameter $0, within a function call, is the name of the
# function.
progpath=$0

# The name of this program.
progname=`$ECHO "$progpath" |$SED "$sed_basename"`

# Make sure we have an absolute progpath for reexecution:
case $progpath in
  [\\/]*|[A-Za-z]:\\*) ;;
  *[\\/]*)
     progdir=`$ECHO "$progpath" |$SED "$sed_dirname"`
     progdir=`cd "$progdir" && pwd`
     progpath=$progdir/$progname
     ;;
  *)
     _G_IFS=$IFS
     IFS=${PATH_SEPARATOR-:}
     for progdir in $PATH; do
       IFS=$_G_IFS
       test -x "$progdir/$progname" && break
     done
     IFS=$_G_IFS
     test -n "$progdir" || progdir=`pwd`
     progpath=$progdir/$progname
     ;;
esac


## ----------------- ##
## Standard options. ##
## ----------------- ##

# The following options affect the operation of the functions defined
# below, and should be set appropriately depending on run-time para-
# meters passed on the command line.

opt_dry_run=false
opt_quiet=false
opt_verbose=false

# Categories 'all' and 'none' are always available.  Append any others
# you will pass as the first argument to func_warning from your own
# code.
warning_categories=

# By default, display warnings according to 'opt_warning_types'.  Set
# 'warning_func'  to ':' to elide all warnings, or func_fatal_error to
# treat the next displayed warning as a fatal error.
warning_func=func_warn_and_continue

# Set to 'all' to display all warnings, 'none' to suppress all
# warnings, or a space delimited list of some subset of
# 'warning_categories' to display only the listed warnings.
opt_warning_types=all


## -------------------- ##
## Resource management. ##
## -------------------- ##

# This section contains definitions for functions that each ensure a
# particular resource (a file, or a non-empty configuration variable for
# example) is available, and if appropriate to extract default values
# from pertinent package files. Call them using their associated
# 'require_*' variable to ensure that they are executed, at most, once.
#
# It's entirely deliberate that calling these functions can set
# variables that don't obey the namespace limitations obeyed by the rest
# of this file, in order that that they be as useful as possible to
# callers.


# require_term_colors
# -------------------
# Allow display of bold text on terminals that support it.
require_term_colors=func_require_term_colors
func_require_term_colors ()
{
    $debug_cmd

    test -t 1 && {
      # COLORTERM and USE_ANSI_COLORS environment variables take
      # precedence, because most terminfo databases neglect to describe
      # whether color sequences are supported.
      test -n "${COLORTERM+set}" && : ${USE_ANSI_COLORS="1"}

      if test 1 = "$USE_ANSI_COLORS"; then
        # Standard ANSI escape sequences
        tc_reset='[0m'
        tc_bold='[1m';   tc_standout='[7m'
        tc_red='[31m';   tc_green='[32m'
        tc_blue='[34m';  tc_cyan='[36m'
      else
        # Otherwise trust the terminfo database after all.
        test -n "`tput sgr0 2>/dev/null`" && {
          tc_reset=`tput sgr0`
          test -n "`tput bold 2>/dev/null`" && tc_bold=`tput bold`
          tc_standout=$tc_bold
          test -n "`tput smso 2>/dev/null`" && tc_standout=`tput smso`
          test -n "`tput setaf 1 2>/dev/null`" && tc_red=`tput setaf 1`
          test -n "`tput setaf 2 2>/dev/null`" && tc_green=`tput setaf 2`
          test -n "`tput setaf 4 2>/dev/null`" && tc_blue=`tput setaf 4`
          test -n "`tput setaf 5 2>/dev/null`" && tc_cyan=`tput setaf 5`
        }
      fi
    }

    require_term_colors=:
}


## ----------------- ##
## Function library. ##
## ----------------- ##

# This section contains a variety of useful functions to call in your
# scripts. Take note of the portable wrappers for features provided by
# some modern shells, which will fall back to slower equivalents on
# less featureful shells.


# func_append VAR VALUE
# ---------------------
# Append VALUE onto the existing contents of VAR.

  # We should try to minimise forks, especially on Windows where they are
  # unreasonably slow, so skip the feature probes when bash or zsh are
  # being used:
  if test set = "${BASH_VERSION+set}${ZSH_VERSION+set}"; then
    : ${_G_HAVE_ARITH_OP="yes"}
    : ${_G_HAVE_XSI_OPS="yes"}
    # The += operator was introduced in bash 3.1
    case $BASH_VERSION in
      [12].* | 3.0 | 3.0*) ;;
      *)
        : ${_G_HAVE_PLUSEQ_OP="yes"}
        ;;
    esac
  fi

  # _G_HAVE_PLUSEQ_OP
  # Can be empty, in which case the shell is probed, "yes" if += is
  # useable or anything else if it does not work.
  test -z "$_G_HAVE_PLUSEQ_OP" \
    && (eval 'x=a; x+=" b"; test "a b" = "$x"') 2>/dev/null \
    && _G_HAVE_PLUSEQ_OP=yes

if test yes = "$_G_HAVE_PLUSEQ_OP"
then
  # This is an XSI compatible shell, allowing a faster implementation...
  eval 'func_append ()
  {
    $debug_cmd

    eval "$1+=\$2"
  }'
else
  # ...otherwise fall back to using expr, which is often a shell builtin.
  func_append ()
  {
    $debug_cmd

    eval "$1=\$$1\$2"
  }
fi


# func_append_quoted VAR VALUE
# ----------------------------
# Quote VALUE and append to the end of shell variable VAR, separated
# by a space.
if test yes = "$_G_HAVE_PLUSEQ_OP"; then
  eval 'func_append_quoted ()
  {
    $debug_cmd

    func_quote_arg pretty "$2"
    eval "$1+=\\ \$func_quote_arg_result"
  }'
else
  func_append_quoted ()
  {
    $debug_cmd

    func_quote_arg pretty "$2"
    eval "$1=\$$1\\ \$func_quote_arg_result"
  }
fi


# func_append_uniq VAR VALUE
# --------------------------
# Append unique VALUE onto the existing contents of VAR, assuming
# entries are delimited by the first character of VALUE.  For example:
#
#   func_append_uniq options " --another-option option-argument"
#
# will only append to $options if " --another-option option-argument "
# is not already present somewhere in $options already (note spaces at
# each end implied by leading space in second argument).
func_append_uniq ()
{
    $debug_cmd

    eval _G_current_value='`$ECHO $'$1'`'
    _G_delim=`expr "$2" : '\(.\)'`

    case $_G_delim$_G_current_value$_G_delim in
      *"$2$_G_delim"*) ;;
      *) func_append "$@" ;;
    esac
}


# func_arith TERM...
# ------------------
# Set func_arith_result to the result of evaluating TERMs.
  test -z "$_G_HAVE_ARITH_OP" \
    && (eval 'test 2 = $(( 1 + 1 ))') 2>/dev/null \
    && _G_HAVE_ARITH_OP=yes

if test yes = "$_G_HAVE_ARITH_OP"; then
  eval 'func_arith ()
  {
    $debug_cmd

    func_arith_result=$(( $* ))
  }'
else
  func_arith ()
  {
    $debug_cmd

    func_arith_result=`expr "$@"`
  }
fi


# func_basename FILE
# ------------------
# Set func_basename_result to FILE with everything up to and including
# the last / stripped.
if test yes = "$_G_HAVE_XSI_OPS"; then
  # If this shell supports suffix pattern removal, then use it to avoid
  # forking. Hide the definitions single quotes in case the shell chokes
  # on unsupported syntax...
  _b='func_basename_result=${1##*/}'
  _d='case $1 in
        */*) func_dirname_result=${1%/*}$2 ;;
        *  ) func_dirname_result=$3        ;;
      esac'

else
  # ...otherwise fall back to using sed.
  _b='func_basename_result=`$ECHO "$1" |$SED "$sed_basename"`'
  _d='func_dirname_result=`$ECHO "$1"  |$SED "$sed_dirname"`
      if test "X$func_dirname_result" = "X$1"; then
        func_dirname_result=$3
      else
        func_append func_dirname_result "$2"
      fi'
fi

eval 'func_basename ()
{
    $debug_cmd

    '"$_b"'
}'


# func_dirname FILE APPEND NONDIR_REPLACEMENT
# -------------------------------------------
# Compute the dirname of FILE.  If nonempty, add APPEND to the result,
# otherwise set result to NONDIR_REPLACEMENT.
eval 'func_dirname ()
{
    $debug_cmd

    '"$_d"'
}'


# func_dirname_and_basename FILE APPEND NONDIR_REPLACEMENT
# --------------------------------------------------------
# Perform func_basename and func_dirname in a single function
# call:
#   dirname:  Compute the dirname of FILE.  If nonempty,
#             add APPEND to the result, otherwise set result
#             to NONDIR_REPLACEMENT.
#             value returned in "$func_dirname_result"
#   basename: Compute filename of FILE.
#             value retuned in "$func_basename_result"
# For efficiency, we do not delegate to the functions above but instead
# duplicate the functionality here.
eval 'func_dirname_and_basename ()
{
    $debug_cmd

    '"$_b"'
    '"$_d"'
}'


# func_echo ARG...
# ----------------
# Echo program name prefixed message.
func_echo ()
{
    $debug_cmd

    _G_message=$*

    func_echo_IFS=$IFS
    IFS=$nl
    for _G_line in $_G_message; do
      IFS=$func_echo_IFS
      $ECHO "$progname: $_G_line"
    done
    IFS=$func_echo_IFS
}


# func_echo_all ARG...
# --------------------
# Invoke $ECHO with all args, space-separated.
func_echo_all ()
{
    $ECHO "$*"
}


# func_echo_infix_1 INFIX ARG...
# ------------------------------
# Echo program name, followed by INFIX on the first line, with any
# additional lines not showing INFIX.
func_echo_infix_1 ()
{
    $debug_cmd

    $require_term_colors

    _G_infix=$1; shift
    _G_indent=$_G_infix
    _G_prefix="$progname: $_G_infix: "
    _G_message=$*

    # Strip color escape sequences before counting printable length
    for _G_tc in "$tc_reset" "$tc_bold" "$tc_standout" "$tc_red" "$tc_green" "$tc_blue" "$tc_cyan"
    do
      test -n "$_G_tc" && {
        _G_esc_tc=`$ECHO "$_G_tc" | $SED "$sed_make_literal_regex"`
        _G_indent=`$ECHO "$_G_indent" | $SED "s|$_G_esc_tc||g"`
      }
    done
    _G_indent="$progname: "`echo "$_G_indent" | $SED 's|.| |g'`"  " ## exclude from sc_prohibit_nested_quotes

    func_echo_infix_1_IFS=$IFS
    IFS=$nl
    for _G_line in $_G_message; do
      IFS=$func_echo_infix_1_IFS
      $ECHO "$_G_prefix$tc_bold$_G_line$tc_reset" >&2
      _G_prefix=$_G_indent
    done
    IFS=$func_echo_infix_1_IFS
}


# func_error ARG...
# -----------------
# Echo program name prefixed message to standard error.
func_error ()
{
    $debug_cmd

    $require_term_colors

    func_echo_infix_1 "  $tc_standout${tc_red}error$tc_reset" "$*" >&2
}


# func_fatal_error ARG...
# -----------------------
# Echo program name prefixed message to standard error, and exit.
func_fatal_error ()
{
    $debug_cmd

    func_error "$*"
    exit $EXIT_FAILURE
}


# func_grep EXPRESSION FILENAME
# -----------------------------
# Check whether EXPRESSION matches any line of FILENAME, without output.
func_grep ()
{
    $debug_cmd

    $GREP "$1" "$2" >/dev/null 2>&1
}


# func_len STRING
# ---------------
# Set func_len_result to the length of STRING. STRING may not
# start with a hyphen.
  test -z "$_G_HAVE_XSI_OPS" \
    && (eval 'x=a/b/c;
      test 5aa/bb/cc = "${#x}${x%%/*}${x%/*}${x#*/}${x##*/}"') 2>/dev/null \
    && _G_HAVE_XSI_OPS=yes

if test yes = "$_G_HAVE_XSI_OPS"; then
  eval 'func_len ()
  {
    $debug_cmd

    func_len_result=${#1}
  }'
else
  func_len ()
  {
    $debug_cmd

    func_len_result=`expr "$1" : ".*" 2>/dev/null || echo $max_cmd_len`
  }
fi


# func_mkdir_p DIRECTORY-PATH
# ---------------------------
# Make sure the entire path to DIRECTORY-PATH is available.
func_mkdir_p ()
{
    $debug_cmd

    _G_directory_path=$1
    _G_dir_list=

    if test -n "$_G_directory_path" && test : != "$opt_dry_run"; then

      # Protect directory names starting with '-'
      case $_G_directory_path in
        -*) _G_directory_path=./$_G_directory_path ;;
      esac

      # While some portion of DIR does not yet exist...
      while test ! -d "$_G_directory_path"; do
        # ...make a list in topmost first order.  Use a colon delimited
	# list incase some portion of path contains whitespace.
        _G_dir_list=$_G_directory_path:$_G_dir_list

        # If the last portion added has no slash in it, the list is done
        case $_G_directory_path in */*) ;; *) break ;; esac

        # ...otherwise throw away the child directory and loop
        _G_directory_path=`$ECHO "$_G_directory_path" | $SED -e "$sed_dirname"`
      done
      _G_dir_list=`$ECHO "$_G_dir_list" | $SED 's|:*$||'`

      func_mkdir_p_IFS=$IFS; IFS=:
      for _G_dir in $_G_dir_list; do
	IFS=$func_mkdir_p_IFS
        # mkdir can fail with a 'File exist' error if two processes
        # try to create one of the directories concurrently.  Don't
        # stop in that case!
        $MKDIR "$_G_dir" 2>/dev/null || :
      done
      IFS=$func_mkdir_p_IFS

      # Bail out if we (or some other process) failed to create a directory.
      test -d "$_G_directory_path" || \
        func_fatal_error "Failed to create '$1'"
    fi
}


# func_mktempdir [BASENAME]
# -------------------------
# Make a temporary directory that won't clash with other running
# libtool processes, and avoids race conditions if possible.  If
# given, BASENAME is the basename for that directory.
func_mktempdir ()
{
    $debug_cmd

    _G_template=${TMPDIR-/tmp}/${1-$progname}

    if test : = "$opt_dry_run"; then
      # Return a directory name, but don't create it in dry-run mode
      _G_tmpdir=$_G_template-$$
    else

      # If mktemp works, use that first and foremost
      _G_tmpdir=`mktemp -d "$_G_template-XXXXXXXX" 2>/dev/null`

      if test ! -d "$_G_tmpdir"; then
        # Failing that, at least try and use $RANDOM to avoid a race
        _G_tmpdir=$_G_template-${RANDOM-0}$$

        func_mktempdir_umask=`umask`
        umask 0077
        $MKDIR "$_G_tmpdir"
        umask $func_mktempdir_umask
      fi

      # If we're not in dry-run mode, bomb out on failure
      test -d "$_G_tmpdir" || \
        func_fatal_error "cannot create temporary directory '$_G_tmpdir'"
    fi

    $ECHO "$_G_tmpdir"
}


# func_normal_abspath PATH
# ------------------------
# Remove doubled-up and trailing slashes, "." path components,
# and cancel out any ".." path components in PATH after making
# it an absolute path.
func_normal_abspath ()
{
    $debug_cmd

    # These SED scripts presuppose an absolute path with a trailing slash.
    _G_pathcar='s|^/\([^/]*\).*$|\1|'
    _G_pathcdr='s|^/[^/]*||'
    _G_removedotparts=':dotsl
		s|/\./|/|g
		t dotsl
		s|/\.$|/|'
    _G_collapseslashes='s|/\{1,\}|/|g'
    _G_finalslash='s|/*$|/|'

    # Start from root dir and reassemble the path.
    func_normal_abspath_result=
    func_normal_abspath_tpath=$1
    func_normal_abspath_altnamespace=
    case $func_normal_abspath_tpath in
      "")
        # Empty path, that just means $cwd.
        func_stripname '' '/' "`pwd`"
        func_normal_abspath_result=$func_stripname_result
        return
        ;;
      # The next three entries are used to spot a run of precisely
      # two leading slashes without using negated character classes;
      # we take advantage of case's first-match behaviour.
      ///*)
        # Unusual form of absolute path, do nothing.
        ;;
      //*)
        # Not necessarily an ordinary path; POSIX reserves leading '//'
        # and for example Cygwin uses it to access remote file shares
        # over CIFS/SMB, so we conserve a leading double slash if found.
        func_normal_abspath_altnamespace=/
        ;;
      /*)
        # Absolute path, do nothing.
        ;;
      *)
        # Relative path, prepend $cwd.
        func_normal_abspath_tpath=`pwd`/$func_normal_abspath_tpath
        ;;
    esac

    # Cancel out all the simple stuff to save iterations.  We also want
    # the path to end with a slash for ease of parsing, so make sure
    # there is one (and only one) here.
    func_normal_abspath_tpath=`$ECHO "$func_normal_abspath_tpath" | $SED \
          -e "$_G_removedotparts" -e "$_G_collapseslashes" -e "$_G_finalslash"`
    while :; do
      # Processed it all yet?
      if test / = "$func_normal_abspath_tpath"; then
        # If we ascended to the root using ".." the result may be empty now.
        if test -z "$func_normal_abspath_result"; then
          func_normal_abspath_result=/
        fi
        break
      fi
      func_normal_abspath_tcomponent=`$ECHO "$func_normal_abspath_tpath" | $SED \
          -e "$_G_pathcar"`
      func_normal_abspath_tpath=`$ECHO "$func_normal_abspath_tpath" | $SED \
          -e "$_G_pathcdr"`
      # Figure out what to do with it
      case $func_normal_abspath_tcomponent in
        "")
          # Trailing empty path component, ignore it.
          ;;
        ..)
          # Parent dir; strip last assembled component from result.
          func_dirname "$func_normal_abspath_result"
          func_normal_abspath_result=$func_dirname_result
          ;;
        *)
          # Actual path component, append it.
          func_append func_normal_abspath_result "/$func_normal_abspath_tcomponent"
          ;;
      esac
    done
    # Restore leading double-slash if one was found on entry.
    func_normal_abspath_result=$func_normal_abspath_altnamespace$func_normal_abspath_result
}


# func_notquiet ARG...
# --------------------
# Echo program name prefixed message only when not in quiet mode.
func_notquiet ()
{
    $debug_cmd

    $opt_quiet || func_echo ${1+"$@"}

    # A bug in bash halts the script if the last line of a function
    # fails when set -e is in force, so we need another command to
    # work around that:
    :
}


# func_relative_path SRCDIR DSTDIR
# --------------------------------
# Set func_relative_path_result to the relative path from SRCDIR to DSTDIR.
func_relative_path ()
{
    $debug_cmd

    func_relative_path_result=
    func_normal_abspath "$1"
    func_relative_path_tlibdir=$func_normal_abspath_result
    func_normal_abspath "$2"
    func_relative_path_tbindir=$func_normal_abspath_result

    # Ascend the tree starting from libdir
    while :; do
      # check if we have found a prefix of bindir
      case $func_relative_path_tbindir in
        $func_relative_path_tlibdir)
          # found an exact match
          func_relative_path_tcancelled=
          break
          ;;
        $func_relative_path_tlibdir*)
          # found a matching prefix
          func_stripname "$func_relative_path_tlibdir" '' "$func_relative_path_tbindir"
          func_relative_path_tcancelled=$func_stripname_result
          if test -z "$func_relative_path_result"; then
            func_relative_path_result=.
          fi
          break
          ;;
        *)
          func_dirname $func_relative_path_tlibdir
          func_relative_path_tlibdir=$func_dirname_result
          if test -z "$func_relative_path_tlibdir"; then
            # Have to descend all the way to the root!
            func_relative_path_result=../$func_relative_path_result
            func_relative_path_tcancelled=$func_relative_path_tbindir
            break
          fi
          func_relative_path_result=../$func_relative_path_result
          ;;
      esac
    done

    # Now calculate path; take care to avoid doubling-up slashes.
    func_stripname '' '/' "$func_relative_path_result"
    func_relative_path_result=$func_stripname_result
    func_stripname '/' '/' "$func_relative_path_tcancelled"
    if test -n "$func_stripname_result"; then
      func_append func_relative_path_result "/$func_stripname_result"
    fi

    # Normalisation. If bindir is libdir, return '.' else relative path.
    if test -n "$func_relative_path_result"; then
      func_stripname './' '' "$func_relative_path_result"
      func_relative_path_result=$func_stripname_result
    fi

    test -n "$func_relative_path_result" || func_relative_path_result=.

    :
}


# func_quote_portable EVAL ARG
# ----------------------------
# Internal function to portably implement func_quote_arg.  Note that we still
# keep attention to performance here so we as much as possible try to avoid
# calling sed binary (so far O(N) complexity as long as func_append is O(1)).
func_quote_portable ()
{
    $debug_cmd

    $require_check_ifs_backslash

    func_quote_portable_result=$2

    # one-time-loop (easy break)
    while true
    do
      if $1; then
        func_quote_portable_result=`$ECHO "$2" | $SED \
          -e "$sed_double_quote_subst" -e "$sed_double_backslash"`
        break
      fi

      # Quote for eval.
      case $func_quote_portable_result in
        *[\\\`\"\$]*)
          # Fallback to sed for $func_check_bs_ifs_broken=:, or when the string
          # contains the shell wildcard characters.
          case $check_ifs_backshlash_broken$func_quote_portable_result in
            :*|*[\[\*\?]*)
              func_quote_portable_result=`$ECHO "$func_quote_portable_result" \
                  | $SED "$sed_quote_subst"`
              break
              ;;
          esac

          func_quote_portable_old_IFS=$IFS
          for _G_char in '\' '`' '"' '$'
          do
            # STATE($1) PREV($2) SEPARATOR($3)
            set start "" ""
            func_quote_portable_result=dummy"$_G_char$func_quote_portable_result$_G_char"dummy
            IFS=$_G_char
            for _G_part in $func_quote_portable_result
            do
              case $1 in
              quote)
                func_append func_quote_portable_result "$3$2"
                set quote "$_G_part" "\\$_G_char"
                ;;
              start)
                set first "" ""
                func_quote_portable_result=
                ;;
              first)
                set quote "$_G_part" ""
                ;;
              esac
            done
          done
          IFS=$func_quote_portable_old_IFS
          ;;
        *) ;;
      esac
      break
    done

    func_quote_portable_unquoted_result=$func_quote_portable_result
    case $func_quote_portable_result in
      # double-quote args containing shell metacharacters to delay
      # word splitting, command substitution and variable expansion
      # for a subsequent eval.
      # many bourne shells cannot handle close brackets correctly
      # in scan sets, so we specify it separately.
      *[\[\~\#\^\&\*\(\)\{\}\|\;\<\>\?\'\ \	]*|*]*|"")
        func_quote_portable_result=\"$func_quote_portable_result\"
        ;;
    esac
}


# func_quotefast_eval ARG
# -----------------------
# Quote one ARG (internal).  This is equivalent to 'func_quote_arg eval ARG',
# but optimized for speed.  Result is stored in $func_quotefast_eval.
if test xyes = `(x=; printf -v x %q yes; echo x"$x") 2>/dev/null`; then
  printf -v _GL_test_printf_tilde %q '~'
  if test '\~' = "$_GL_test_printf_tilde"; then
    func_quotefast_eval ()
    {
      printf -v func_quotefast_eval_result %q "$1"
    }
  else
    # Broken older Bash implementations.  Make those faster too if possible.
    func_quotefast_eval ()
    {
      case $1 in
        '~'*)
          func_quote_portable false "$1"
          func_quotefast_eval_result=$func_quote_portable_result
          ;;
        *)
          printf -v func_quotefast_eval_result %q "$1"
          ;;
      esac
    }
  fi
else
  func_quotefast_eval ()
  {
    func_quote_portable false "$1"
    func_quotefast_eval_result=$func_quote_portable_result
  }
fi


# func_quote_arg MODEs ARG
# ------------------------
# Quote one ARG to be evaled later.  MODEs argument may contain zero or more
# specifiers listed below separated by ',' character.  This function returns two
# values:
#   i) func_quote_arg_result
#      double-quoted (when needed), suitable for a subsequent eval
#  ii) func_quote_arg_unquoted_result
#      has all characters that are still active within double
#      quotes backslashified.  Available only if 'unquoted' is specified.
#
# Available modes:
# ----------------
# 'eval' (default)
#       - escape shell special characters
# 'expand'
#       - the same as 'eval';  but do not quote variable references
# 'pretty'
#       - request aesthetic output, i.e. '"a b"' instead of 'a\ b'.  This might
#         be used later in func_quote to get output like: 'echo "a b"' instead
#         of 'echo a\ b'.  This is slower than default on some shells.
# 'unquoted'
#       - produce also $func_quote_arg_unquoted_result which does not contain
#         wrapping double-quotes.
#
# Examples for 'func_quote_arg pretty,unquoted string':
#
#   string      | *_result              | *_unquoted_result
#   ------------+-----------------------+-------------------
#   "           | \"                    | \"
#   a b         | "a b"                 | a b
#   "a b"       | "\"a b\""             | \"a b\"
#   *           | "*"                   | *
#   z="${x-$y}" | "z=\"\${x-\$y}\""     | z=\"\${x-\$y}\"
#
# Examples for 'func_quote_arg pretty,unquoted,expand string':
#
#   string        |   *_result          |  *_unquoted_result
#   --------------+---------------------+--------------------
#   z="${x-$y}"   | "z=\"${x-$y}\""     | z=\"${x-$y}\"
func_quote_arg ()
{
    _G_quote_expand=false
    case ,$1, in
      *,expand,*)
        _G_quote_expand=:
        ;;
    esac

    case ,$1, in
      *,pretty,*|*,expand,*|*,unquoted,*)
        func_quote_portable $_G_quote_expand "$2"
        func_quote_arg_result=$func_quote_portable_result
        func_quote_arg_unquoted_result=$func_quote_portable_unquoted_result
        ;;
      *)
        # Faster quote-for-eval for some shells.
        func_quotefast_eval "$2"
        func_quote_arg_result=$func_quotefast_eval_result
        ;;
    esac
}


# func_quote MODEs ARGs...
# ------------------------
# Quote all ARGs to be evaled later and join them into single command.  See
# func_quote_arg's description for more info.
func_quote ()
{
    $debug_cmd
    _G_func_quote_mode=$1 ; shift
    func_quote_result=
    while test 0 -lt $#; do
      func_quote_arg "$_G_func_quote_mode" "$1"
      if test -n "$func_quote_result"; then
        func_append func_quote_result " $func_quote_arg_result"
      else
        func_append func_quote_result "$func_quote_arg_result"
      fi
      shift
    done
}


# func_stripname PREFIX SUFFIX NAME
# ---------------------------------
# strip PREFIX and SUFFIX from NAME, and store in func_stripname_result.
# PREFIX and SUFFIX must not contain globbing or regex special
# characters, hashes, percent signs, but SUFFIX may contain a leading
# dot (in which case that matches only a dot).
if test yes = "$_G_HAVE_XSI_OPS"; then
  eval 'func_stripname ()
  {
    $debug_cmd

    # pdksh 5.2.14 does not do ${X%$Y} correctly if both X and Y are
    # positional parameters, so assign one to ordinary variable first.
    func_stripname_result=$3
    func_stripname_result=${func_stripname_result#"$1"}
    func_stripname_result=${func_stripname_result%"$2"}
  }'
else
  func_stripname ()
  {
    $debug_cmd

    case $2 in
      .*) func_stripname_result=`$ECHO "$3" | $SED -e "s%^$1%%" -e "s%\\\\$2\$%%"`;;
      *)  func_stripname_result=`$ECHO "$3" | $SED -e "s%^$1%%" -e "s%$2\$%%"`;;
    esac
  }
fi


# func_show_eval CMD [FAIL_EXP]
# -----------------------------
# Unless opt_quiet is true, then output CMD.  Then, if opt_dryrun is
# not true, evaluate CMD.  If the evaluation of CMD fails, and FAIL_EXP
# is given, then evaluate it.
func_show_eval ()
{
    $debug_cmd

    _G_cmd=$1
    _G_fail_exp=${2-':'}

    func_quote_arg pretty,expand "$_G_cmd"
    eval "func_notquiet $func_quote_arg_result"

    $opt_dry_run || {
      eval "$_G_cmd"
      _G_status=$?
      if test 0 -ne "$_G_status"; then
	eval "(exit $_G_status); $_G_fail_exp"
      fi
    }
}


# func_show_eval_locale CMD [FAIL_EXP]
# ------------------------------------
# Unless opt_quiet is true, then output CMD.  Then, if opt_dryrun is
# not true, evaluate CMD.  If the evaluation of CMD fails, and FAIL_EXP
# is given, then evaluate it.  Use the saved locale for evaluation.
func_show_eval_locale ()
{
    $debug_cmd

    _G_cmd=$1
    _G_fail_exp=${2-':'}

    $opt_quiet || {
      func_quote_arg expand,pretty "$_G_cmd"
      eval "func_echo $func_quote_arg_result"
    }

    $opt_dry_run || {
      eval "$_G_user_locale
	    $_G_cmd"
      _G_status=$?
      eval "$_G_safe_locale"
      if test 0 -ne "$_G_status"; then
	eval "(exit $_G_status); $_G_fail_exp"
      fi
    }
}


# func_tr_sh
# ----------
# Turn $1 into a string suitable for a shell variable name.
# Result is stored in $func_tr_sh_result.  All characters
# not in the set a-zA-Z0-9_ are replaced with '_'. Further,
# if $1 begins with a digit, a '_' is prepended as well.
func_tr_sh ()
{
    $debug_cmd

    case $1 in
    [0-9]* | *[!a-zA-Z0-9_]*)
      func_tr_sh_result=`$ECHO "$1" | $SED -e 's/^\([0-9]\)/_\1/' -e 's/[^a-zA-Z0-9_]/_/g'`
      ;;
    * )
      func_tr_sh_result=$1
      ;;
    esac
}


# func_verbose ARG...
# -------------------
# Echo program name prefixed message in verbose mode only.
func_verbose ()
{
    $debug_cmd

    $opt_verbose && func_echo "$*"

    :
}


# func_warn_and_continue ARG...
# -----------------------------
# Echo program name prefixed warning message to standard error.
func_warn_and_continue ()
{
    $debug_cmd

    $require_term_colors

    func_echo_infix_1 "${tc_red}warning$tc_reset" "$*" >&2
}


# func_warning CATEGORY ARG...
# ----------------------------
# Echo program name prefixed warning message to standard error. Warning
# messages can be filtered according to CATEGORY, where this function
# elides messages where CATEGORY is not listed in the global variable
# 'opt_warning_types'.
func_warning ()
{
    $debug_cmd

    # CATEGORY must be in the warning_categories list!
    case " $warning_categories " in
      *" $1 "*) ;;
      *) func_internal_error "invalid warning category '$1'" ;;
    esac

    _G_category=$1
    shift

    case " $opt_warning_types " in
      *" $_G_category "*) $warning_func ${1+"$@"} ;;
    esac
}


# func_sort_ver VER1 VER2
# -----------------------
# 'sort -V' is not generally available.
# Note this deviates from the version comparison in automake
# in that it treats 1.5 < 1.5.0, and treats 1.4.4a < 1.4-p3a
# but this should suffice as we won't be specifying old
# version formats or redundant trailing .0 in bootstrap.conf.
# If we did want full compatibility then we should probably
# use m4_version_compare from autoconf.
func_sort_ver ()
{
    $debug_cmd

    printf '%s\n%s\n' "$1" "$2" \
      | sort -t. -k 1,1n -k 2,2n -k 3,3n -k 4,4n -k 5,5n -k 6,6n -k 7,7n -k 8,8n -k 9,9n
}

# func_lt_ver PREV CURR
# ---------------------
# Return true if PREV and CURR are in the correct order according to
# func_sort_ver, otherwise false.  Use it like this:
#
#  func_lt_ver "$prev_ver" "$proposed_ver" || func_fatal_error "..."
func_lt_ver ()
{
    $debug_cmd

    test "x$1" = x`func_sort_ver "$1" "$2" | $SED 1q`
}


# Local variables:
# mode: shell-script
# sh-indentation: 2
# eval: (add-hook 'before-save-hook 'time-stamp)
# time-stamp-pattern: "10/scriptversion=%:y-%02m-%02d.%02H; # UTC"
# time-stamp-time-zone: "UTC"
# End:
