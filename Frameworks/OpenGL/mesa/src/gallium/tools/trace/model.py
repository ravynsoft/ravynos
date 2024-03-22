#!/usr/bin/env python3
##########################################################################
# 
# Copyright 2008 VMware, Inc.
# All Rights Reserved.
# 
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sub license, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
# 
# The above copyright notice and this permission notice (including the
# next paragraph) shall be included in all copies or substantial portions
# of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
# IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
# ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
# 
##########################################################################


'''Trace data model.'''


import sys
import string
import binascii
from io import StringIO
import format


class ModelOptions:

    def __init__(self, args=None):
        # Initialize the options we need to exist,
        # with some reasonable defaults
        self.plain = False
        self.suppress_variants = False
        self.named_ptrs = False
        self.method_only = False

        # If args is specified, we assume it is the result object
        # from ArgumentParser.parse_args(). Copy the attribute values
        # we have from it, if they exist.
        if args is not None:
            for var in self.__dict__:
                if var in args.__dict__:
                    self.__dict__[var] = args.__dict__[var]


class TraceStateData:

    def __init__(self):
        self.ptr_list = {}
        self.ptr_type_list = {}
        self.ptr_types_list = {}


class Node:
    
    def visit(self, visitor):
        raise NotImplementedError

    def __str__(self):
        stream = StringIO()
        formatter = format.Formatter(stream)
        pretty_printer = PrettyPrinter(formatter, {})
        self.visit(pretty_printer)
        return stream.getvalue()

    def __hash__(self):
        raise NotImplementedError


class Literal(Node):
    
    def __init__(self, value):
        self.value = value

    def visit(self, visitor):
        visitor.visit_literal(self)

    def __hash__(self):
        return hash(self.value)


class Blob(Node):
    
    def __init__(self, value):
        self.value = binascii.a2b_hex(value)

    def getValue(self):
        return self.value

    def visit(self, visitor):
        visitor.visit_blob(self)

    def __hash__(self):
        return hash(self.value)


class NamedConstant(Node):
    
    def __init__(self, name):
        self.name = name

    def visit(self, visitor):
        visitor.visit_named_constant(self)

    def __hash__(self):
        return hash(self.name)
    

class Array(Node):
    
    def __init__(self, elements):
        self.elements = elements

    def visit(self, visitor):
        visitor.visit_array(self)

    def __hash__(self):
        tmp = 0
        for mobj in self.elements:
            tmp = tmp ^ hash(mobj)
        return tmp


class Struct(Node):
    
    def __init__(self, name, members):
        self.name = name
        self.members = members        

    def visit(self, visitor):
        visitor.visit_struct(self)

    def __hash__(self):
        tmp = hash(self.name)
        for mname, mobj in self.members:
            tmp = tmp ^ hash(mname) ^ hash(mobj)
        return tmp


class Pointer(Node):

    ptr_ignore_list = ["ret", "elem"]

    def __init__(self, state, address, pname):
        self.address = address
        self.state = state

        # Check if address exists in list and if it is a return value address
        t1 = address in state.ptr_list
        if t1:
            rname = state.ptr_type_list[address]
            t2 = rname in self.ptr_ignore_list and pname not in self.ptr_ignore_list
        else:
            rname = pname
            t2 = False

        # If address does NOT exist (add it), OR IS a ret value (update with new type)
        if not t1 or t2:
            # If previously set to ret value, remove one from count
            if t1 and t2:
                self.adjust_ptr_type_count(rname, -1)

            # Add / update
            self.adjust_ptr_type_count(pname, 1)
            tmp = "{}_{}".format(pname, state.ptr_types_list[pname])
            state.ptr_list[address] = tmp
            state.ptr_type_list[address] = pname

    def adjust_ptr_type_count(self, pname, delta):
        if pname not in self.state.ptr_types_list:
            self.state.ptr_types_list[pname] = 0

        self.state.ptr_types_list[pname] += delta

    def named_address(self):
        return self.state.ptr_list[self.address]

    def visit(self, visitor):
        visitor.visit_pointer(self)

    def __hash__(self):
        return hash(self.named_address())


class Call:
    
    def __init__(self, no, klass, method, args, ret, time):
        self.no = no
        self.klass = klass
        self.method = method
        self.args = args
        self.ret = ret
        self.time = time

        # Calculate hashvalue "cached" into a variable
        self.hashvalue = hash(self.klass) ^ hash(self.method)
        for mname, mobj in self.args:
            self.hashvalue = self.hashvalue ^ hash(mname) ^ hash(mobj)
        
    def visit(self, visitor):
        visitor.visit_call(self)

    def __hash__(self):
        return self.hashvalue

    def __eq__(self, other):
        return self.hashvalue == other.hashvalue


class Trace:
    
    def __init__(self, calls):
        self.calls = calls
        
    def visit(self, visitor):
        visitor.visit_trace(self)
    
    
class Visitor:
    
    def visit_literal(self, node):
        raise NotImplementedError
    
    def visit_blob(self, node):
        raise NotImplementedError
    
    def visit_named_constant(self, node):
        raise NotImplementedError
    
    def visit_array(self, node):
        raise NotImplementedError
    
    def visit_struct(self, node):
        raise NotImplementedError
    
    def visit_pointer(self, node):
        raise NotImplementedError
    
    def visit_call(self, node):
        raise NotImplementedError
    
    def visit_trace(self, node):
        raise NotImplementedError


class PrettyPrinter:

    def __init__(self, formatter, options):
        self.formatter = formatter
        self.options = options

    def visit_literal(self, node):
        if node.value is None:
            self.formatter.literal('NULL')
            return

        if isinstance(node.value, str):
            self.formatter.literal('"' + node.value + '"')
            return

        self.formatter.literal(repr(node.value))
    
    def visit_blob(self, node):
        self.formatter.address('blob()')
    
    def visit_named_constant(self, node):
        self.formatter.literal(node.name)
    
    def visit_array(self, node):
        self.formatter.text('{')
        sep = ''
        for value in node.elements:
            self.formatter.text(sep)
            value.visit(self) 
            sep = ', '
        self.formatter.text('}')
    
    def visit_struct(self, node):
        self.formatter.text('{')
        sep = ''
        for name, value in node.members:
            self.formatter.text(sep)
            self.formatter.variable(name)
            self.formatter.text(' = ')
            value.visit(self) 
            sep = ', '
        self.formatter.text('}')
    
    def visit_pointer(self, node):
        if self.options.named_ptrs:
            self.formatter.address(node.named_address())
        else:
            self.formatter.address(node.address)

    def visit_call(self, node):
        if not self.options.suppress_variants:
            self.formatter.text(f'{node.no} ')

        if node.klass is not None:
            self.formatter.function(node.klass + '::' + node.method)
        else:
            self.formatter.function(node.method)

        if not self.options.method_only:
            self.formatter.text('(')
            sep = ''
            for name, value in node.args:
                self.formatter.text(sep)
                self.formatter.variable(name)
                self.formatter.text(' = ')
                value.visit(self)
                sep = ', '
            self.formatter.text(')')
            if node.ret is not None:
                self.formatter.text(' = ')
                node.ret.visit(self)

        if not self.options.suppress_variants and node.time is not None:
            self.formatter.text(' // time ')
            node.time.visit(self)

        self.formatter.newline()

    def visit_trace(self, node):
        for call in node.calls:
            call.visit(self)

