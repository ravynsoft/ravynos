# coding=utf-8
#
# Copyright © 2011, 2018 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

from sexps import *

def make_test_case(f_name, ret_type, body):
    """Create a simple optimization test case consisting of a single
    function with the given name, return type, and body.

    Global declarations are automatically created for any undeclared
    variables that are referenced by the function.  All undeclared
    variables are assumed to be floats.
    """
    check_sexp(body)
    declarations = {}
    def make_declarations(sexp, already_declared = ()):
        if isinstance(sexp, list):
            if len(sexp) == 2 and sexp[0] == 'var_ref':
                if sexp[1] not in already_declared:
                    declarations[sexp[1]] = [
                        'declare', ['in'], 'float', sexp[1]]
            elif len(sexp) == 4 and sexp[0] == 'assign':
                assert sexp[2][0] == 'var_ref'
                if sexp[2][1] not in already_declared:
                    declarations[sexp[2][1]] = [
                        'declare', ['out'], 'float', sexp[2][1]]
                make_declarations(sexp[3], already_declared)
            else:
                already_declared = set(already_declared)
                for s in sexp:
                    if isinstance(s, list) and len(s) >= 4 and \
                            s[0] == 'declare':
                        already_declared.add(s[3])
                    else:
                        make_declarations(s, already_declared)
    make_declarations(body)
    return list(declarations.values()) + \
        [['function', f_name, ['signature', ret_type, ['parameters'], body]]]


# The following functions can be used to build expressions.

def const_float(value):
    """Create an expression representing the given floating point value."""
    return ['constant', 'float', ['{0:.6f}'.format(value)]]

def const_bool(value):
    """Create an expression representing the given boolean value.

    If value is not a boolean, it is converted to a boolean.  So, for
    instance, const_bool(1) is equivalent to const_bool(True).
    """
    return ['constant', 'bool', ['{0}'.format(1 if value else 0)]]

def gt_zero(var_name):
    """Create Construct the expression var_name > 0"""
    return ['expression', 'bool', '<', const_float(0), ['var_ref', var_name]]


# The following functions can be used to build complex control flow
# statements.  All of these functions return statement lists (even
# those which only create a single statement), so that statements can
# be sequenced together using the '+' operator.

def return_(value = None):
    """Create a return statement."""
    if value is not None:
        return [['return', value]]
    else:
        return [['return']]

def break_():
    """Create a break statement."""
    return ['break']

def continue_():
    """Create a continue statement."""
    return ['continue']

def simple_if(var_name, then_statements, else_statements = None):
    """Create a statement of the form

    if (var_name > 0.0) {
       <then_statements>
    } else {
       <else_statements>
    }

    else_statements may be omitted.
    """
    if else_statements is None:
        else_statements = []
    check_sexp(then_statements)
    check_sexp(else_statements)
    return [['if', gt_zero(var_name), then_statements, else_statements]]

def loop(statements):
    """Create a loop containing the given statements as its loop
    body.
    """
    check_sexp(statements)
    return [['loop', statements]]

def declare_temp(var_type, var_name):
    """Create a declaration of the form

    (declare (temporary) <var_type> <var_name)
    """
    return [['declare', ['temporary'], var_type, var_name]]

def assign_x(var_name, value):
    """Create a statement that assigns <value> to the variable
    <var_name>.  The assignment uses the mask (x).
    """
    check_sexp(value)
    return [['assign', ['x'], ['var_ref', var_name], value]]

def complex_if(var_prefix, statements):
    """Create a statement of the form

    if (<var_prefix>a > 0.0) {
       if (<var_prefix>b > 0.0) {
          <statements>
       }
    }

    This is useful in testing jump lowering, because if <statements>
    ends in a jump, lower_jumps.cpp won't try to combine this
    construct with the code that follows it, as it might do for a
    simple if.

    All variables used in the if statement are prefixed with
    var_prefix.  This can be used to ensure uniqueness.
    """
    check_sexp(statements)
    return simple_if(var_prefix + 'a', simple_if(var_prefix + 'b', statements))

def declare_execute_flag():
    """Create the statements that lower_jumps.cpp uses to declare and
    initialize the temporary boolean execute_flag.
    """
    return declare_temp('bool', 'execute_flag') + \
        assign_x('execute_flag', const_bool(True))

def declare_return_flag():
    """Create the statements that lower_jumps.cpp uses to declare and
    initialize the temporary boolean return_flag.
    """
    return declare_temp('bool', 'return_flag') + \
        assign_x('return_flag', const_bool(False))

def declare_return_value():
    """Create the statements that lower_jumps.cpp uses to declare and
    initialize the temporary variable return_value.  Assume that
    return_value is a float.
    """
    return declare_temp('float', 'return_value')

def declare_break_flag():
    """Create the statements that lower_jumps.cpp uses to declare and
    initialize the temporary boolean break_flag.
    """
    return declare_temp('bool', 'break_flag') + \
        assign_x('break_flag', const_bool(False))

def lowered_return_simple(value = None):
    """Create the statements that lower_jumps.cpp lowers a return
    statement to, in situations where it does not need to clear the
    execute flag.
    """
    if value:
        result = assign_x('return_value', value)
    else:
        result = []
    return result + assign_x('return_flag', const_bool(True))

def lowered_return(value = None):
    """Create the statements that lower_jumps.cpp lowers a return
    statement to, in situations where it needs to clear the execute
    flag.
    """
    return lowered_return_simple(value) + \
        assign_x('execute_flag', const_bool(False))

def lowered_continue():
    """Create the statement that lower_jumps.cpp lowers a continue
    statement to.
    """
    return assign_x('execute_flag', const_bool(False))

def lowered_break_simple():
    """Create the statement that lower_jumps.cpp lowers a break
    statement to, in situations where it does not need to clear the
    execute flag.
    """
    return assign_x('break_flag', const_bool(True))

def lowered_break():
    """Create the statement that lower_jumps.cpp lowers a break
    statement to, in situations where it needs to clear the execute
    flag.
    """
    return lowered_break_simple() + assign_x('execute_flag', const_bool(False))

def if_execute_flag(statements):
    """Wrap statements in an if test so that they will only execute if
    execute_flag is True.
    """
    check_sexp(statements)
    return [['if', ['var_ref', 'execute_flag'], statements, []]]

def if_return_flag(then_statements, else_statements):
    """Wrap statements in an if test with return_flag as the condition.
    """
    check_sexp(then_statements)
    check_sexp(else_statements)
    return [['if', ['var_ref', 'return_flag'], then_statements, else_statements]]

def if_not_return_flag(statements):
    """Wrap statements in an if test so that they will only execute if
    return_flag is False.
    """
    check_sexp(statements)
    return [['if', ['var_ref', 'return_flag'], [], statements]]

def final_return():
    """Create the return statement that lower_jumps.cpp places at the
    end of a function when lowering returns.
    """
    return [['return', ['var_ref', 'return_value']]]

def final_break():
    """Create the conditional break statement that lower_jumps.cpp
    places at the end of a function when lowering breaks.
    """
    return [['if', ['var_ref', 'break_flag'], break_(), []]]

def bash_quote(*args):
    """Quote the arguments appropriately so that bash will understand
    each argument as a single word.
    """
    def quote_word(word):
        for c in word:
            if not (c.isalpha() or c.isdigit() or c in '@%_-+=:,./'):
                break
        else:
            if not word:
                return "''"
            return word
        return "'{0}'".format(word.replace("'", "'\"'\"'"))
    return ' '.join(quote_word(word) for word in args)

def create_test_case(input_sexp, expected_sexp, test_name,
                     pull_out_jumps=False, lower_sub_return=False,
                     lower_main_return=False, lower_continue=False):
    """Create a test case that verifies that do_lower_jumps transforms
    the given code in the expected way.
    """
    check_sexp(input_sexp)
    check_sexp(expected_sexp)
    input_str = sexp_to_string(sort_decls(input_sexp))
    expected_output = sexp_to_string(sort_decls(expected_sexp)) # XXX: don't stringify this
    optimization = (
        'do_lower_jumps({0:d}, {1:d}, {2:d}, {3:d})'.format(
            pull_out_jumps, lower_sub_return, lower_main_return,
            lower_continue))

    return (test_name, optimization, input_str, expected_output)

def test_lower_returns_main():
    """Test that do_lower_jumps respects the lower_main_return flag in deciding
    whether to lower returns in the main function.
    """
    input_sexp = make_test_case('main', 'void', (
            complex_if('', return_())
            ))
    expected_sexp = make_test_case('main', 'void', (
            declare_execute_flag() +
            declare_return_flag() +
            complex_if('', lowered_return())
            ))
    yield create_test_case(
        input_sexp, expected_sexp, 'lower_returns_main_true',
        lower_main_return=True)
    yield create_test_case(
        input_sexp, input_sexp, 'lower_returns_main_false',
        lower_main_return=False)

def test_lower_returns_sub():
    """Test that do_lower_jumps respects the lower_sub_return flag in deciding
    whether to lower returns in subroutines.
    """
    input_sexp = make_test_case('sub', 'void', (
            complex_if('', return_())
            ))
    expected_sexp = make_test_case('sub', 'void', (
            declare_execute_flag() +
            declare_return_flag() +
            complex_if('', lowered_return())
            ))
    yield create_test_case(
        input_sexp, expected_sexp, 'lower_returns_sub_true',
        lower_sub_return=True)
    yield create_test_case(
        input_sexp, input_sexp, 'lower_returns_sub_false',
        lower_sub_return=False)

def test_lower_returns_1():
    """Test that a void return at the end of a function is eliminated."""
    input_sexp = make_test_case('main', 'void', (
            assign_x('a', const_float(1)) +
            return_()
            ))
    expected_sexp = make_test_case('main', 'void', (
            assign_x('a', const_float(1))
            ))
    yield create_test_case(
        input_sexp, expected_sexp, 'lower_returns_1', lower_main_return=True)

def test_lower_returns_2():
    """Test that lowering is not performed on a non-void return at the end of
    subroutine.
    """
    input_sexp = make_test_case('sub', 'float', (
            assign_x('a', const_float(1)) +
            return_(const_float(1))
            ))
    yield create_test_case(
        input_sexp, input_sexp, 'lower_returns_2', lower_sub_return=True)

def test_lower_returns_3():
    """Test lowering of returns when there is one nested inside a complex
    structure of ifs, and one at the end of a function.

    In this case, the latter return needs to be lowered because it will not be
    at the end of the function once the final return is inserted.
    """
    input_sexp = make_test_case('sub', 'float', (
            complex_if('', return_(const_float(1))) +
            return_(const_float(2))
            ))
    expected_sexp = make_test_case('sub', 'float', (
            declare_execute_flag() +
            declare_return_value() +
            declare_return_flag() +
            complex_if('', lowered_return(const_float(1))) +
            if_execute_flag(lowered_return(const_float(2))) +
            final_return()
            ))
    yield create_test_case(
        input_sexp, expected_sexp, 'lower_returns_3', lower_sub_return=True)

def test_lower_returns_4():
    """Test that returns are properly lowered when they occur in both branches
    of an if-statement.
    """
    input_sexp = make_test_case('sub', 'float', (
            simple_if('a', return_(const_float(1)),
                      return_(const_float(2)))
            ))
    expected_sexp = make_test_case('sub', 'float', (
            declare_execute_flag() +
            declare_return_value() +
            declare_return_flag() +
            simple_if('a', lowered_return(const_float(1)),
                      lowered_return(const_float(2))) +
            final_return()
            ))
    yield create_test_case(
        input_sexp, expected_sexp, 'lower_returns_4', lower_sub_return=True)

def test_lower_unified_returns():
    """If both branches of an if statement end in a return, and pull_out_jumps
    is True, then those returns should be lifted outside the if and then
    properly lowered.

    Verify that this lowering occurs during the same pass as the lowering of
    other returns by checking that extra temporary variables aren't generated.
    """
    input_sexp = make_test_case('main', 'void', (
            complex_if('a', return_()) +
            simple_if('b', simple_if('c', return_(), return_()))
            ))
    expected_sexp = make_test_case('main', 'void', (
            declare_execute_flag() +
            declare_return_flag() +
            complex_if('a', lowered_return()) +
            if_execute_flag(simple_if('b', (simple_if('c', [], []) +
                                            lowered_return())))
            ))
    yield create_test_case(
        input_sexp, expected_sexp, 'lower_unified_returns',
        lower_main_return=True, pull_out_jumps=True)

def test_lower_pulled_out_jump():
    doc_string = """If one branch of an if ends in a jump, and control cannot
    fall out the bottom of the other branch, and pull_out_jumps is
    True, then the jump is lifted outside the if.

    Verify that this lowering occurs during the same pass as the
    lowering of other jumps by checking that extra temporary
    variables aren't generated.
    """
    input_sexp = make_test_case('main', 'void', (
            complex_if('a', return_()) +
            loop(simple_if('b', simple_if('c', break_(), continue_()),
                           return_())) +
            assign_x('d', const_float(1))
            ))
    # Note: optimization produces two other effects: the break
    # gets lifted out of the if statements, and the code after the
    # loop gets guarded so that it only executes if the return
    # flag is clear.
    expected_sexp = make_test_case('main', 'void', (
            declare_execute_flag() +
            declare_return_flag() +
            complex_if('a', lowered_return()) +
            if_execute_flag(
                loop(simple_if('b', simple_if('c', [], continue_()),
                               lowered_return_simple()) +
                     break_()) +

                if_return_flag(assign_x('return_flag', const_bool(1)) +
                               assign_x('execute_flag', const_bool(0)),
                               assign_x('d', const_float(1))))
            ))
    yield create_test_case(
        input_sexp, expected_sexp, 'lower_pulled_out_jump',
        lower_main_return=True, pull_out_jumps=True)


def test_remove_continue_at_end_of_loop():
    """Test that a redundant continue-statement at the end of a loop is
    removed.
    """
    input_sexp = make_test_case('main', 'void', (
            loop(assign_x('a', const_float(1)) +
                 continue_())
            ))
    expected_sexp = make_test_case('main', 'void', (
            loop(assign_x('a', const_float(1)))
            ))
    yield create_test_case(input_sexp, expected_sexp, 'remove_continue_at_end_of_loop')

def test_lower_return_void_at_end_of_loop():
    """Test that a return of void at the end of a loop is properly lowered."""
    input_sexp = make_test_case('main', 'void', (
            loop(assign_x('a', const_float(1)) +
                 return_()) +
            assign_x('b', const_float(2))
            ))
    expected_sexp = make_test_case('main', 'void', (
            declare_execute_flag() +
            declare_return_flag() +
            loop(assign_x('a', const_float(1)) +
                 lowered_return_simple() +
                 break_()) +
            if_return_flag(assign_x('return_flag', const_bool(1)) +
                           assign_x('execute_flag', const_bool(0)),
                           assign_x('b', const_float(2)))
            ))
    yield create_test_case(
        input_sexp, input_sexp, 'return_void_at_end_of_loop_lower_nothing')
    yield create_test_case(
        input_sexp, expected_sexp, 'return_void_at_end_of_loop_lower_return',
        lower_main_return=True)


def test_lower_return_non_void_at_end_of_loop():
    """Test that a non-void return at the end of a loop is properly lowered."""
    input_sexp = make_test_case('sub', 'float', (
            loop(assign_x('a', const_float(1)) +
                 return_(const_float(2))) +
            assign_x('b', const_float(3)) +
            return_(const_float(4))
            ))
    expected_sexp = make_test_case('sub', 'float', (
            declare_execute_flag() +
            declare_return_value() +
            declare_return_flag() +
            loop(assign_x('a', const_float(1)) +
                 lowered_return_simple(const_float(2)) +
                 break_()) +
            if_return_flag(assign_x('return_value', '(var_ref return_value)') +
                           assign_x('return_flag', const_bool(1)) +
                           assign_x('execute_flag', const_bool(0)),
                           assign_x('b', const_float(3)) +
                               lowered_return(const_float(4))) +
            final_return()
            ))
    yield create_test_case(
        input_sexp, input_sexp, 'return_non_void_at_end_of_loop_lower_nothing')
    yield create_test_case(
        input_sexp, expected_sexp,
        'return_non_void_at_end_of_loop_lower_return', lower_sub_return=True)


CASES = [
    test_lower_pulled_out_jump,
    test_lower_return_non_void_at_end_of_loop,
    test_lower_return_void_at_end_of_loop,
    test_lower_returns_1, test_lower_returns_2, test_lower_returns_3,
    test_lower_returns_4, test_lower_returns_main, test_lower_returns_sub,
    test_lower_unified_returns, test_remove_continue_at_end_of_loop,
]
