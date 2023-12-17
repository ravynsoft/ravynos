#! /bin/sh
#
# Copyright (C) 2006-2023 Free Software Foundation, Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#

# This script determines the declared global symbols in a C header file.
# Assumptions:
# - The header files are in C, with only C89 comments.
# - No use of macros with parameters.
# - All global declarations are marked with 'extern'.
# - All declarations end in ';' on the same line.
# - Not more than one symbol is declared in a declaration.

# This script requires GNU sed.

# func_usage
# outputs to stdout the --help usage message.
func_usage ()
{
  echo "\
Usage: declared.sh [OPTION]... < SOURCE.h

Extracts the declared global symbols of a C header file.

Options:
      --help           print this help and exit
      --version        print version information and exit

Report bugs to <bruno@clisp.org>."
}

# func_version
# outputs to stdout the --version message.
func_version ()
{
  echo "declared.sh (GNU gnulib)"
  echo "Copyright (C) 2022 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law."
  echo "Written by" "Bruno Haible"
}

# func_fatal_error message
# outputs to stderr a fatal error message, and terminates the program.
func_fatal_error ()
{
  echo "declared.sh: *** $1" 1>&2
  echo "declared.sh: *** Stop." 1>&2
  exit 1
}

# Command-line option processing.
while test $# -gt 0; do
  case "$1" in
    --help | --hel | --he | --h )
      func_usage
      exit 0 ;;
   --version | --versio | --versi | --vers | --ver | --ve | --v )
      func_version
      exit 0 ;;
    -- )      # Stop option processing
      shift; break ;;
    -* )
      func_fatal_error "unrecognized option: $1"
      ;;
    * )
      break ;;
  esac
done

if test $# -gt 0; then
  func_fatal_error "too many arguments"
fi

# A sed expression that removes ANSI C and ISO C99 comments.
sed_remove_comments="
/[/][/*]/{
  ta
  :a
  s,^\\(\\([^\"'/]\\|\"\\([^\\\"]\\|[\\].\\)*\"\\|'\\([^\\']\\|[\\].\\)*'\\|[/][^\"'/*]\\|[/]\"\\([^\\\"]\\|[\\].\\)*\"\\|[/]'\\([^\\']\\|[\\].\\)*'\\)*\\)//.*,\\1,
  te
  s,^\\(\\([^\"'/]\\|\"\\([^\\\"]\\|[\\].\\)*\"\\|'\\([^\\']\\|[\\].\\)*'\\|[/][^\"'/*]\\|[/]\"\\([^\\\"]\\|[\\].\\)*\"\\|[/]'\\([^\\']\\|[\\].\\)*'\\)*\\)/[*]\\([^*]\\|[*][^/*]\\)*[*][*]*/,\\1 ,
  ta
  /^\\([^\"'/]\\|\"\\([^\\\"]\\|[\\].\\)*\"\\|'\\([^\\']\\|[\\].\\)*'\\|[/][^\"'/*]\\|[/]\"\\([^\\\"]\\|[\\].\\)*\"\\|[/]'\\([^\\']\\|[\\].\\)*'\\)*[/][*]/{
    s,^\\(\\([^\"'/]\\|\"\\([^\\\"]\\|[\\].\\)*\"\\|'\\([^\\']\\|[\\].\\)*'\\|[/][^\"'/*]\\|[/]\"\\([^\\\"]\\|[\\].\\)*\"\\|[/]'\\([^\\']\\|[\\].\\)*'\\)*\\)/[*].*,\\1 ,
    tu
    :u
    n
    s,^\\([^*]\\|[*][^/*]\\)*[*][*]*/,,
    tv
    s,^.*\$,,
    bu
    :v
  }
  :e
}"

# Check that 'sed' supports the kind of regular expressions used in
# sed_remove_comments. The use of \| meaning alternation of basic regular
# expressions is a GNU extension.
sed_test='s,^\(\(a\|X\)*\)//.*,\1,'
sed_result=`echo 'aaa//bcd' | sed -e "$sed_test"`
test "$sed_result" = 'aaa' \
  || func_fatal_error "The 'sed' program is not GNU sed. Try installing GNU sed."

# A sed expression that joins 'extern' declarations that are broken over
# several lines.
sed_join_multiline_externs='
/^extern [^;"]*$/{
  :a
  N
  s/\n/ /g
  /^extern [^;"]*$/{
    ba
  }
}'

# A sed expression that extracts the identifier of each 'extern' declaration.
sed_extract_extern_declared='s/^extern [^()]*[ *]\([A-Za-z_][A-Za-z0-9_]*\) *[;(].*$/\1/p'

sed -e "$sed_remove_comments" \
  | sed -e "$sed_join_multiline_externs" \
  | sed -n -e "$sed_extract_extern_declared"
