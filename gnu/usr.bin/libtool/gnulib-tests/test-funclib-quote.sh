#! /bin/sh

# Unit tests for funclib.sh

# This is free software.  There is NO warranty; not even for
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# Copyright (C) 2015-2019, 2021 Bootstrap Authors
#
# This file is dual licensed under the terms of the MIT license
# <https://opensource.org/license/MIT>, and GPL version 2 or later
# <http://www.gnu.org/licenses/gpl-2.0.html>.  You must apply one of
# these licenses when using or redistributing this software or any of
# the files within it.  See the URLs above, or the file `LICENSE`
# included in the Bootstrap distribution for the full license texts.

# Please report bugs or propose patches to:
# <https://github.com/gnulib-modules/bootstrap/issues>


all_shells_script=$0
. "$abs_srcdir/test-all-shells.sh" || exit 1


_compare_or_error ()
{
    _G_msg="$1: strings differ:
    a: $2
    b: $3"
    test "$2" = "$3" || all_shells_error "$_G_msg"
}


dump_args ()
{
    dump_args_result=
    _separator=
    for arg
    do
      func_append dump_args_result "$_separator$arg"
      _separator=' '
    done
}


# test_q_eval DESC ARGs...
# ------------------------
# Apply 'func_quote eval' on ARGs and eval the result.  The eval-ed result must
# 100% match the original ARGs.
test_q_eval ()
{
    description="$1" ; shift

    dump_args ${1+"$@"}
    original=$dump_args_result

    $ECHO "~> func_quote eval: $description: $original"

    # Pretty implementation
    func_quote eval,pretty ${1+"$@"} \
      || all_shells_error "can't pretty func_quote args '$*'"
    eval "set dummy $func_quote_result" ; shift
    dump_args ${1+"$@"}
    pretty=$dump_args_result

    # Fast implementation (if available)
    func_quote eval ${1+"$@"} || all_shells_error "can't fast func_quote args '$*'"
    eval "set dummy $func_quote_result" ; shift
    dump_args ${1+"$@"}
    fast=$dump_args_result

    _compare_or_error "$description (pretty)"     "$original" "$pretty"
    _compare_or_error "$description (fast)"       "$original" "$fast"
}


# test_q_arg_eval DESC ARG
# ------------------------
# Apply 'func_quote_arg eval' on ARG and eval the result.  Echo-ed result within
# eval must match original echo-ed ARG.
test_q_arg_eval ()
{
    description=$1
    original=$2
    original_echo=`$ECHO "$original"`

    $ECHO "~> func_quote_arg eval: $description: $original_echo"

    func_quote_arg pretty,unquoted "$original" \
        || all_shells_error "can't quote_arg: $original"
    pretty=$func_quote_arg_result
    pretty_unquoted=$func_quote_arg_unquoted_result
    pretty_echo=`eval '$ECHO '"$pretty"` \
        || all_shells_error "can't eval: $pretty"

    pretty_unquoted_echo=`eval '$ECHO '"\"$pretty_unquoted\""` \
        || all_shells_error "can't eval: $pretty"

    # Fast implementation.
    func_quote_arg eval "$original"
    fast=$func_quote_arg_result
    fast_echo=`eval '$ECHO '"$fast"` || all_shells_error "can't eval: $pretty"

    _compare_or_error "$description (pretty)" \
        "$original_echo" "$pretty_echo"
    _compare_or_error "$description (pretty_unquoted)" \
        "$original_echo" "$pretty_unquoted_echo"
    _compare_or_error "$description (fast)" \
        "$original_echo" "$fast_echo"
}


# test_q_expand DESC EXP_RESULT ARG
# ---------------------------------
# Test that 'func_quote expand' works fine --> all shell special characters are
# quoted except '$' -- while all variables are expanded.
test_q_expand ()
{
    description=$1 ; shift
    exp_result=$1 ; shift

    dump_args ${1+"$@"}
    $ECHO "~> func_quote expand: $description: $dump_args_result"

    func_quote expand ${1+"$@"}
    eval "set dummy $func_quote_result" ; shift
    dump_args ${1+"$@"}

    _compare_or_error "$description (expand)" "$exp_result" "$dump_args_result"
}


## ============== ##
## Start testing! ##
## ============== ##

aaa=aaa ; bbb=bbb ; ccc=ccc ; ddd=ddd

# Needed for later testing of globbing.
touch fltestA fltestB

test_q_eval     basic             a b c
# TODO: Intentionally not checking newline here yet, it never worked.
test_q_eval     spaces            'space space' 'tab	tab'
test_q_eval     empty_arg         '' '' ''
test_q_eval     globs             '*' '.*' '[a-zA-Z0-9_]' '?' '~'
test_q_eval     variables         '$aaa' '${bbb}' '"${ccc} $ddd"'
test_q_eval     exclamation-mark  '$!' '!$' '!'
test_q_eval     tilde             '"~"'
test_q_eval     single-quotes     "'a'" "'"'$bbb'"'"
test_q_eval     shell-vars        '$1' '$@' '$*' 'ending$'
test_q_eval     complicated-cmd   grep b '>' /noperm '<' /noperm

test_q_arg_eval basic             a
test_q_arg_eval single-quotes     "'''"
test_q_arg_eval double-quotes     '"""'
test_q_arg_eval tilde             '~'
test_q_arg_eval ampersand         '&'
test_q_arg_eval pipe              '|'
test_q_arg_eval questionmark      'fltest?'
test_q_arg_eval glob-bracket      'fltest[A-Z]'
test_q_arg_eval space             'space space'
test_q_arg_eval tab               'tab	tab'
test_q_arg_eval '`command`'       '`false command`'
test_q_arg_eval '$(command)'      '$(false command)'
test_q_arg_eval semicolon         '; false'
test_q_arg_eval vars              '$aaa ${bbb} "${ccc} $ddd"'
test_q_arg_eval if-then-else      'if false; then false; else false; fi'
test_q_arg_eval file-redirect     'echo a > /no-perm 2> /no-perm'
test_q_arg_eval case-stmt         'case $empty in "") false;; a) broken ;; esac'
test_q_arg_eval comment           'unexistent #'
test_q_arg_eval func              'func () { } # syntax error'

test_q_expand   basic             'a b c aaa d'   a b c '$aaa' d
test_q_expand   double-quotes     '" " " "bbb"'   '"' '" "' '"$bbb"'
test_q_expand   spaces            '  	 ccc'     ' ' '	' '${ccc}'
# Note the *no expected space* here!
test_q_expand   non-existent      ''              '$empty' '${empty}'
test_q_expand   non-existent-2    '"" ""'         '"$empty"' '"${empty}"'
# TODO: Intentionally not checking '$(cmd)' yet.
test_q_expand   '`command`'       '`aaa bbb`'     '`$aaa $empty${bbb}`'

$all_shells_exit_cmd && rm -rf fltest*
