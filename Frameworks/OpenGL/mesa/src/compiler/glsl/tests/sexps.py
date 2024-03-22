# coding=utf-8
#
# Copyright © 2011 Intel Corporation
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

# This file contains helper functions for manipulating sexps in Python.
#
# We represent a sexp in Python using nested lists containing strings.
# So, for example, the sexp (constant float (1.000000)) is represented
# as ['constant', 'float', ['1.000000']].

import re

def check_sexp(sexp):
    """Verify that the argument is a proper sexp.

    That is, raise an exception if the argument is not a string or a
    list, or if it contains anything that is not a string or a list at
    any nesting level.
    """
    if isinstance(sexp, list):
        for s in sexp:
            check_sexp(s)
    elif not isinstance(sexp, str):
        raise Exception('Not a sexp: {0!r}'.format(sexp))

def parse_sexp(sexp):
    """Convert a string, of the form that would be output by mesa,
    into a sexp represented as nested lists containing strings.
    """
    sexp_token_regexp = re.compile(
        '[a-zA-Z_]+(@[0-9]+)?|[0-9]+(\\.[0-9]+)?|[^ \r?\n]')
    stack = [[]]
    for match in sexp_token_regexp.finditer(sexp):
        token = match.group(0)
        if token == '(':
            stack.append([])
        elif token == ')':
            if len(stack) == 1:
                raise Exception('Unmatched )')
            sexp = stack.pop()
            stack[-1].append(sexp)
        else:
            stack[-1].append(token)
    if len(stack) != 1:
        raise Exception('Unmatched (')
    if len(stack[0]) != 1:
        raise Exception('Multiple sexps')
    return stack[0][0]

def sexp_to_string(sexp):
    """Convert a sexp, represented as nested lists containing strings,
    into a single string of the form parseable by mesa.
    """
    if isinstance(sexp, str):
        return sexp
    assert isinstance(sexp, list)
    result = ''
    for s in sexp:
        sub_result = sexp_to_string(s)
        if result == '':
            result = sub_result
        elif '\n' not in result and '\n' not in sub_result and \
                len(result) + len(sub_result) + 1 <= 70:
            result += ' ' + sub_result
        else:
            result += '\n' + sub_result
    return '({0})'.format(result.replace('\n', '\n '))

def sort_decls(sexp):
    """Sort all toplevel variable declarations in sexp.

    This is used to work around the fact that
    ir_reader::read_instructions reorders declarations.
    """
    assert isinstance(sexp, list)
    decls = []
    other_code = []
    for s in sexp:
        if isinstance(s, list) and len(s) >= 4 and s[0] == 'declare':
            decls.append(s)
        else:
            other_code.append(s)
    return sorted(decls) + other_code

