#!/bin/env python
COPYRIGHT = """\
/*
 * Copyright 2021 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
"""

import os
import re
import ply.lex as lex
import ply.yacc as yacc

# Libraries

libraries = {}

# LEXER

keywords = {
    '__debugbreak': 'KW_DEBUGBREAK',
    'alignas': 'KW_ALIGNAS',
    'args': 'KW_ARGS',
    'atomic': 'KW_ATOMIC',
    'atomic_return': 'KW_ATOMIC_RETURN',
    'const': 'KW_CONST',
    'control': 'KW_CONTROL',
    'define': 'KW_DEFINE',
    'dispatch': 'KW_DISPATCH',
    'dispatch_indirect': 'KW_DISPATCH_INDIRECT',
    'goto': 'KW_GOTO',
    'if': 'KW_IF',
    'kernel': 'KW_KERNEL',
    'kernel_module': 'KW_KERNEL_MODULE',
    'import': 'KW_IMPORT',
    'library': 'KW_LIBRARY',
    'links': 'KW_LINKS',
    'load_dword': 'KW_LOAD_DWORD',
    'load_qword': 'KW_LOAD_QWORD',
    'metakernel': 'KW_METAKERNEL',
    'module': 'KW_MODULE',
    'not': 'KW_NOT',
    'offsetof': 'KW_OFFSETOF',
    'postsync': 'KW_POSTSYNC',
    'print': 'KW_PRINT',
    'semaphore_wait': 'KW_SEMAPHORE_WAIT',
    'shiftof': 'KW_SHIFTOF',
    'sizeof': 'KW_SIZEOF',
    'store_dword': 'KW_STORE_DWORD',
    'store_qword': 'KW_STORE_QWORD',
    'store_timestamp': 'KW_STORE_TIMESTAMP',
    'struct': 'KW_STRUCT',
    'unsigned': 'KW_UNSIGNED',
    'while': 'KW_WHILE'
}

ops = {
    '&&': 'OP_LOGICAL_AND',
    '||': 'OP_LOGICAL_OR',
    '==': 'OP_EQUALEQUAL',
    '!=': 'OP_NOTEQUAL',
    '<=': 'OP_LESSEQUAL',
    '>=': 'OP_GREATEREQUAL',
    '<<': 'OP_LSHIFT',
    '>>': 'OP_RSHIFT'
}

tokens = [
    'INT_LITERAL',
    'STRING_LITERAL',
    'OP',
    'IDENTIFIER'
] + list(keywords.values()) + list(ops.values())

def t_INT_LITERAL(t):
    r'(0x[a-fA-F0-9]+|\d+)'
    if t.value.startswith('0x'):
        t.value = int(t.value[2:], 16)
    else:
        t.value = int(t.value)
    return t

def t_OP(t):
    r'(&&|\|\||==|!=|<=|>=|<<|>>)'
    t.type = ops.get(t.value)
    return t

def t_IDENTIFIER(t):
    r'[a-zA-Z_][a-zA-Z_0-9]*'
    t.type = keywords.get(t.value, 'IDENTIFIER')
    return t

def t_STRING_LITERAL(t):
    r'"(\\.|[^"\\])*"'
    t.value = t.value[1:-1]
    return t

literals = "+*/(){};:,=&|!~^.%?-<>[]"

t_ignore = ' \t'

def t_newline(t):
    r'\n+'
    t.lexer.lineno += len(t.value)

def t_error(t):
    print("WUT: {}".format(t.value))
    t.lexer.skip(1)

LEXER = lex.lex()

# PARSER

precedence = (
    ('right', '?', ':'),
    ('left', 'OP_LOGICAL_OR', 'OP_LOGICAL_AND'),
    ('left', '|'),
    ('left', '^'),
    ('left', '&'),
    ('left', 'OP_EQUALEQUAL', 'OP_NOTEQUAL'),
    ('left', '<', '>', 'OP_LESSEQUAL', 'OP_GREATEREQUAL'),
    ('left', 'OP_LSHIFT', 'OP_RSHIFT'),
    ('left', '+', '-'),
    ('left', '*', '/', '%'),
    ('right', '!', '~'),
    ('left', '[', ']', '.')
)

def p_module(p):
    'module : element_list'
    p[0] = p[1]

def p_element_list(p):
    '''element_list : element_list element
                    | element'''
    if len(p) == 2:
        p[0] = [p[1]]
    else:
        p[0] = p[1] + [p[2]]

def p_element(p):
    '''element : kernel_definition
               | kernel_module_definition
               | library_definition
               | metakernel_definition
               | module_name
               | struct_definition
               | const_definition
               | import_definition'''
    p[0] = p[1]

def p_module_name(p):
    'module_name : KW_MODULE IDENTIFIER ";"'
    p[0] = ('module-name', p[2])

def p_kernel_module_definition(p):
    'kernel_module_definition : KW_KERNEL_MODULE IDENTIFIER "(" STRING_LITERAL ")" "{" kernel_definition_list "}"'
    p[0] = ('kernel-module', p[2], p[4], p[7])

def p_kernel_definition(p):
    'kernel_definition : KW_KERNEL IDENTIFIER optional_annotation_list'
    p[0] = ('kernel', p[2], p[3])

def p_library_definition(p):
    'library_definition : KW_LIBRARY IDENTIFIER "{" library_definition_list "}"'
    p[0] = ('library', p[2], p[4])

def p_library_definition_list(p):
    '''library_definition_list :
                              | library_definition_list IDENTIFIER STRING_LITERAL ";"'''
    if len(p) < 3:
        p[0] = []
    else:
        p[0] = p[1]
        p[0].append((p[2], p[3]))

def p_import_definition(p):
    'import_definition : KW_IMPORT KW_STRUCT IDENTIFIER STRING_LITERAL ";"'
    p[0] = ('import', p[4], 'struct', p[3])

def p_links_definition(p):
    'links_definition : KW_LINKS IDENTIFIER'

    # Process a library include like a preprocessor
    global libraries

    if not p[2] in libraries:
        raise "Not able to find library {0}".format(p[2])
    p[0] = libraries[p[2]]

def p_metakernel_definition(p):
    'metakernel_definition : KW_METAKERNEL IDENTIFIER "(" optional_parameter_list ")" optional_annotation_list scope'
    p[0] = ('meta-kernel', p[2], p[4], p[6], p[7])

def p_kernel_definition_list(p):
    '''kernel_definition_list :
                              | kernel_definition_list kernel_definition ";"
                              | kernel_definition_list links_definition ";"'''
    if len(p) < 3:
        p[0] = []
    else:
        p[0] = p[1]
        p[0].append(p[2])

def p_optional_annotation_list(p):
    '''optional_annotation_list :
                                | "<" ">"
                                | "<" annotation_list ">"'''
    if len(p) < 4:
        p[0] = {}
    else:
        p[0] = p[2]

def p_optional_parameter_list(p):
    '''optional_parameter_list :
                               | parameter_list'''
    p[0] = p[1]

def p_annotation_list(p):
    '''annotation_list : annotation'''
    p[0] = p[1]

def p_annotation_list_append(p):
    '''annotation_list : annotation_list "," annotation'''
    p[0] = {**p[1], **p[3]}

def p_annotation(p):
    '''annotation : IDENTIFIER "=" INT_LITERAL
                  | IDENTIFIER "=" IDENTIFIER
                  | IDENTIFIER "=" STRING_LITERAL'''
    p[0] = {p[1]: p[3]}

def p_parameter_list(p):
    '''parameter_list : parameter_definition'''
    p[0] = [p[1]]

def p_parameter_list_append(p):
    '''parameter_list : parameter_list "," parameter_definition'''
    p[0] = p[1]
    p[0].append(p[3])

def p_parameter_definition(p):
    'parameter_definition : IDENTIFIER IDENTIFIER'
    p[0] = (p[1], p[2])

def p_scope(p):
    '''scope : "{" optional_statement_list "}"'''
    p[0] = p[2]

def p_optional_statement_list(p):
    '''optional_statement_list :
                               | statement_list'''
    p[0] = p[1]

def p_statement_list(p):
    '''statement_list : statement'''
    p[0] = [p[1]]

def p_statement_list_append(p):
    '''statement_list : statement_list statement'''
    p[0] = p[1]
    p[0].append(p[2])

def p_statement(p):
    '''statement : definition_statement ";"
                 | assignment_statement ";"
                 | load_store_statement ";"
                 | dispatch_statement ";"
                 | semaphore_statement ";"
                 | label
                 | goto_statement ";"
                 | scope_statement
                 | atomic_op_statement ";"
                 | control_statement ";"
                 | print_statement ";"
                 | debug_break_statement ";"'''
    p[0] = p[1]

def p_definition_statement(p):
    'definition_statement : KW_DEFINE IDENTIFIER value'
    p[0] = ('define', p[2], p[3])

def p_assignemt_statement(p):
    'assignment_statement : value "=" value'
    p[0] = ('assign', p[1], p[3])

def p_load_store_statement_load_dword(p):
    '''load_store_statement : value "=" KW_LOAD_DWORD "(" value ")"'''
    p[0] = ('load-dword', p[1], p[5])

def p_load_store_statement_load_qword(p):
    '''load_store_statement : value "=" KW_LOAD_QWORD "(" value ")"'''
    p[0] = ('load-qword', p[1], p[5])

def p_load_store_statement_store_dword(p):
    '''load_store_statement : KW_STORE_DWORD "(" value "," value ")"'''
    p[0] = ('store-dword', p[3], p[5])

def p_load_store_statement_store_qword(p):
    '''load_store_statement : KW_STORE_QWORD "(" value "," value ")"'''
    p[0] = ('store-qword', p[3], p[5])

def p_dispatch_statement(p):
    '''dispatch_statement : direct_dispatch_statement
                          | indirect_dispatch_statement'''
    p[0] = p[1]

def p_direct_dispatch_statement(p):
    '''direct_dispatch_statement : KW_DISPATCH IDENTIFIER "(" value "," value "," value ")" optional_kernel_arg_list optional_postsync'''
    p[0] = ('dispatch', p[2], (p[4], p[6], p[8]), p[10], p[11])

def p_indirect_dispatch_statement(p):
    '''indirect_dispatch_statement : KW_DISPATCH_INDIRECT IDENTIFIER optional_kernel_arg_list optional_postsync'''
    p[0] = ('dispatch', p[2], None, p[3], p[4])

def p_optional_kernel_arg_list(p):
    '''optional_kernel_arg_list :
                                | KW_ARGS "(" value_list ")"'''
    p[0] = p[3]

def p_value_list(p):
    '''value_list : value'''
    p[0] = [p[1]]

def p_value_list_append(p):
    '''value_list : value_list "," value'''
    p[0] = p[1]
    p[0].append(p[3])

def p_optional_postsync(p):
    '''optional_postsync :
                         | postsync_operation'''
    if len(p) > 1:
        p[0] = p[1]

def p_postsync_operation(p):
    '''postsync_operation : postsync_write_dword
                          | postsync_write_timestamp'''
    p[0] = p[1]

def p_postsync_write_dword(p):
    '''postsync_write_dword : KW_POSTSYNC KW_STORE_DWORD "(" value "," value ")"'''
    p[0] = ('postsync', 'store-dword', p[4], p[6])

def p_postsync_write_timestamp(p):
    '''postsync_write_timestamp : KW_POSTSYNC KW_STORE_TIMESTAMP "(" value ")"'''
    p[0] = ('postsync', 'timestamp', p[4])

def p_semaphore_statement(p):
    '''semaphore_statement : KW_SEMAPHORE_WAIT KW_WHILE "(" "*" value "<" value ")"
                           | KW_SEMAPHORE_WAIT KW_WHILE "(" "*" value ">" value ")"
                           | KW_SEMAPHORE_WAIT KW_WHILE "(" "*" value OP_LESSEQUAL value ")"
                           | KW_SEMAPHORE_WAIT KW_WHILE "(" "*" value OP_GREATEREQUAL value ")"
                           | KW_SEMAPHORE_WAIT KW_WHILE "(" "*" value OP_EQUALEQUAL value ")"
                           | KW_SEMAPHORE_WAIT KW_WHILE "(" "*" value OP_NOTEQUAL value ")"'''
    p[0] = ('sem-wait-while', p[5], p[6], p[7])

def p_atomic_op_statement(p):
    '''atomic_op_statement : KW_ATOMIC IDENTIFIER IDENTIFIER "(" value_list ")"'''
    p[0] = ('atomic', p[2], p[3], p[5])

def p_atomic_op_statement_return(p):
    '''atomic_op_statement : KW_ATOMIC_RETURN IDENTIFIER IDENTIFIER "(" value_list ")"'''
    p[0] = ('atomic-return', p[2], p[3], p[5])

def p_label(p):
    '''label : IDENTIFIER ":"'''
    p[0] = ('label', p[1])

def p_goto_statement(p):
    '''goto_statement : KW_GOTO IDENTIFIER'''
    p[0] = ('goto', p[2])

def p_goto_statement_if(p):
    '''goto_statement : KW_GOTO IDENTIFIER KW_IF "(" value ")"'''
    p[0] = ('goto-if', p[2], p[5])

def p_goto_statement_if_not(p):
    '''goto_statement : KW_GOTO IDENTIFIER KW_IF KW_NOT "(" value ")"'''
    p[0] = ('goto-if-not', p[2], p[6])

def p_scope_statement(p):
    '''scope_statement : scope'''
    p[0] = (p[1])

def p_control_statement(p):
    '''control_statement : KW_CONTROL "(" id_list ")"'''
    p[0] = ('control', p[3])

def p_print_statement(p):
    '''print_statement : KW_PRINT "(" printable_list ")"'''
    p[0] = ('print', p[3])

def p_printable_list(p):
    '''printable_list : printable'''
    p[0] = [p[1]]

def p_printable_list_append(p):
    '''printable_list : printable_list "," printable'''
    p[0] = p[1]
    p[0].append(p[3])

def p_printable_str_lit(p):
    '''printable : STRING_LITERAL'''
    p[0] = '"{}"'.format(p[1])

def p_printable_value(p):
    '''printable : value'''
    p[0] = p[1]

def p_printable_str_lit_value(p):
    '''printable : STRING_LITERAL value'''
    p[0] = ('"{}"'.format(p[1]), p[2])

def p_debug_break_statement(p):
    '''debug_break_statement : KW_DEBUGBREAK'''
    p[0] = ('debug-break')

def p_id_list(p):
    '''id_list : IDENTIFIER'''
    p[0] = p[1]

def p_id_list_append(p):
    '''id_list : id_list "," IDENTIFIER'''
    p[0] = p[1]
    p[0].append(p[3])

def p_value(p):
    '''value : IDENTIFIER
             | INT_LITERAL'''
    p[0] = p[1]

def p_value_braces(p):
    '''value : "(" value ")"'''
    p[0] = (p[2])

def p_value_member(p):
    '''value : value "." IDENTIFIER'''
    p[0] = ('member', p[1], p[3])

def p_value_idx(p):
    '''value : value "[" value "]"'''
    p[0] = ('index', p[1], p[3])

def p_value_binop(p):
    '''value : value "+" value
             | value "-" value
             | value "*" value
             | value "/" value
             | value "%" value
             | value "&" value
             | value "|" value
             | value "<" value
             | value ">" value
             | value "^" value
             | value OP_LESSEQUAL value
             | value OP_GREATEREQUAL value
             | value OP_EQUALEQUAL value
             | value OP_NOTEQUAL value
             | value OP_LOGICAL_AND value
             | value OP_LOGICAL_OR value
             | value OP_LSHIFT value
             | value OP_RSHIFT value'''
    p[0] = (p[2], p[1], p[3])

def p_value_uniop(p):
    '''value : "!" value
             | "~" value'''
    p[0] = (p[1], p[2])

def p_value_cond(p):
    '''value : value "?" value ":" value'''
    p[0] = ('?', p[1], p[3], p[5])

def p_value_funcop(p):
    '''value : KW_OFFSETOF "(" offset_expression ")"
             | KW_SHIFTOF "(" IDENTIFIER ")"
             | KW_SIZEOF "(" IDENTIFIER ")"'''
    p[0] = (p[1], p[3])

def p_offset_expression(p):
    '''offset_expression : IDENTIFIER'''
    p[0] = p[1]

def p_offset_expression_member(p):
    '''offset_expression : offset_expression "." IDENTIFIER'''
    p[0] = ('member', p[1], p[3])

def p_offset_expression_idx(p):
    '''offset_expression : offset_expression "[" INT_LITERAL "]"'''
    p[0] = ('index', p[1], p[3])

def p_struct_definition(p):
    '''struct_definition : KW_STRUCT optional_alignment_specifier IDENTIFIER "{" optional_struct_member_list "}" ";"'''
    p[0] = ('struct', p[3], p[5], p[2])

def p_optional_alignment_specifier(p):
    '''optional_alignment_specifier :
                                    | KW_ALIGNAS "(" INT_LITERAL ")"'''
    if len(p) == 1:
        p[0] = 0
    else:
        p[0] = p[3]

def p_optional_struct_member_list(p):
    '''optional_struct_member_list :
                                   | struct_member_list'''
    if len(p) == 1:
        p[0] = {}
    else:
        p[0] = p[1]

def p_struct_member_list(p):
    '''struct_member_list : struct_member'''
    p[0] = [p[1]]

def p_struct_member_list_append(p):
    '''struct_member_list : struct_member_list struct_member'''
    p[0] = p[1] + [p[2]]

def p_struct_member(p):
    '''struct_member : struct_member_typename IDENTIFIER ";"'''
    p[0] = (p[1], p[2])

def p_struct_member_array(p):
    '''struct_member : struct_member_typename IDENTIFIER "[" INT_LITERAL "]" ";"'''
    '''struct_member : struct_member_typename IDENTIFIER "[" IDENTIFIER "]" ";"'''
    p[0] = {p[1]: p[2], 'count': p[4]}

def p_struct_member_typename(p):
    '''struct_member_typename : IDENTIFIER'''
    p[0] = p[1]

def p_struct_member_typename_unsigned(p):
    '''struct_member_typename : KW_UNSIGNED IDENTIFIER'''
    p[0] = ('unsigned', p[2])

def p_struct_member_typename_struct(p):
    '''struct_member_typename : KW_STRUCT IDENTIFIER'''
    p[0] = ('struct', p[2])

def p_const_definition(p):
    '''const_definition : KW_CONST IDENTIFIER "=" INT_LITERAL ";"'''
    p[0] = ('named-constant', p[2], p[4])

PARSER = yacc.yacc()

# Shamelessly stolen from some StackOverflow answer
def _remove_comments(text):
    def replacer(match):
        s = match.group(0)
        if s.startswith('/'):
            return " " # note: a space and not an empty string
        else:
            return s
    pattern = re.compile(
        r'//.*?$|/\*.*?\*/|\'(?:\\.|[^\\\'])*\'|"(?:\\.|[^\\"])*"',
        re.DOTALL | re.MULTILINE
    )
    return re.sub(pattern, replacer, text)

def parse_grl_file(grl_fname, libs):
    global libraries

    libraries = libs
    with open(grl_fname, 'r') as f:
        return PARSER.parse(_remove_comments(f.read()))
