#!/usr/bin/env python3
##########################################################################
#
# Copyright 2011 Jose Fonseca
# All Rights Reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
##########################################################################/


import json
import argparse
import re
import difflib
import sys


def strip_object_hook(obj):
    if '__class__' in obj:
        return None
    for name in obj.keys():
        if name.startswith('__') and name.endswith('__'):
            del obj[name]
    return obj


class Visitor:

    def visit(self, node, *args, **kwargs):
        if isinstance(node, dict):
            return self.visitObject(node, *args, **kwargs)
        elif isinstance(node, list):
            return self.visitArray(node, *args, **kwargs)
        else:
            return self.visitValue(node, *args, **kwargs)

    def visitObject(self, node, *args, **kwargs):
        pass

    def visitArray(self, node, *args, **kwargs):
        pass

    def visitValue(self, node, *args, **kwargs):
        pass


class Dumper(Visitor):

    def __init__(self, stream = sys.stdout):
        self.stream = stream
        self.level = 0

    def _write(self, s):
        self.stream.write(s)

    def _indent(self):
        self._write('  '*self.level)

    def _newline(self):
        self._write('\n')

    def visitObject(self, node):
        self.enter_object()

        members = sorted(node)
        for i in range(len(members)):
            name = members[i]
            value = node[name]
            self.enter_member(name)
            self.visit(value)
            self.leave_member(i == len(members) - 1)
        self.leave_object()

    def enter_object(self):
        self._write('{')
        self._newline()
        self.level += 1

    def enter_member(self, name):
        self._indent()
        self._write('%s: ' % name)

    def leave_member(self, last):
        if not last:
            self._write(',')
        self._newline()

    def leave_object(self):
        self.level -= 1
        self._indent()
        self._write('}')
        if self.level <= 0:
            self._newline()

    def visitArray(self, node):
        self.enter_array()
        for i in range(len(node)):
            value = node[i]
            self._indent()
            self.visit(value)
            if i != len(node) - 1:
                self._write(',')
            self._newline()
        self.leave_array()

    def enter_array(self):
        self._write('[')
        self._newline()
        self.level += 1

    def leave_array(self):
        self.level -= 1
        self._indent()
        self._write(']')

    def visitValue(self, node):
        self._write(json.dumps(node, allow_nan=True))



class Comparer(Visitor):

    def __init__(self, ignore_added = False, tolerance = 2.0 ** -24):
        self.ignore_added = ignore_added
        self.tolerance = tolerance

    def visitObject(self, a, b):
        if not isinstance(b, dict):
            return False
        if len(a) != len(b) and not self.ignore_added:
            return False
        ak = sorted(a)
        bk = sorted(b)
        if ak != bk and not self.ignore_added:
            return False
        for k in ak:
            ae = a[k]
            try:
                be = b[k]
            except KeyError:
                return False
            if not self.visit(ae, be):
                return False
        return True

    def visitArray(self, a, b):
        if not isinstance(b, list):
            return False
        if len(a) != len(b):
            return False
        for ae, be in zip(a, b):
            if not self.visit(ae, be):
                return False
        return True

    def visitValue(self, a, b):
        if isinstance(a, float) and isinstance(b, float):
            if a == 0:
                return abs(b) < self.tolerance
            else:
                return abs((b - a) / a) < self.tolerance
        else:
            return a == b


class Differ(Visitor):

    def __init__(self, stream = sys.stdout, ignore_added = False):
        self.dumper = Dumper(stream)
        self.comparer = Comparer(ignore_added = ignore_added)

    def visit(self, a, b):
        if self.comparer.visit(a, b):
            return
        Visitor.visit(self, a, b)

    def visitObject(self, a, b):
        if not isinstance(b, dict):
            self.replace(a, b)
        else:
            self.dumper.enter_object()
            names = set(a.keys())
            if not self.comparer.ignore_added:
                names.update(b.keys())
            names = list(names)
            names.sort()

            for i in range(len(names)):
                name = names[i]
                ae = a.get(name, None)
                be = b.get(name, None)
                if not self.comparer.visit(ae, be):
                    self.dumper.enter_member(name)
                    self.visit(ae, be)
                    self.dumper.leave_member(i == len(names) - 1)

            self.dumper.leave_object()

    def visitArray(self, a, b):
        if not isinstance(b, list):
            self.replace(a, b)
        else:
            self.dumper.enter_array()
            max_len = max(len(a), len(b))
            for i in range(max_len):
                try:
                    ae = a[i]
                except IndexError:
                    ae = None
                try:
                    be = b[i]
                except IndexError:
                    be = None
                self.dumper._indent()
                if self.comparer.visit(ae, be):
                    self.dumper.visit(ae)
                else:
                    self.visit(ae, be)
                if i != max_len - 1:
                    self.dumper._write(',')
                self.dumper._newline()

            self.dumper.leave_array()

    def visitValue(self, a, b):
        if a != b:
            self.replace(a, b)

    def replace(self, a, b):
        if isinstance(a, str) and isinstance(b, str):
            if '\n' in a or '\n' in b:
                a = a.splitlines()
                b = b.splitlines()
                differ = difflib.Differ()
                result = differ.compare(a, b)
                self.dumper.level += 1
                for entry in result:
                    self.dumper._newline()
                    self.dumper._indent()
                    tag = entry[:2]
                    text = entry[2:]
                    if tag == '? ':
                        tag = '  '
                        prefix = ' '
                        text = text.rstrip()
                        suffix = ''
                    else:
                        prefix = '"'
                        suffix = '\\n"'
                    line = tag + prefix + text + suffix
                    self.dumper._write(line)
                self.dumper.level -= 1
                return
        self.dumper.visit(a)
        self.dumper._write(' -> ')
        self.dumper.visit(b)

    def isMultilineString(self, value):
        return isinstance(value, str) and '\n' in value
    
    def replaceMultilineString(self, a, b):
        self.dumper.visit(a)
        self.dumper._write(' -> ')
        self.dumper.visit(b)


#
# Unfortunately JSON standard does not include comments, but this is a quite
# useful feature to have on regressions tests
#

_token_res = [
    r'//[^\r\n]*', # comment
    r'"[^"\\]*(\\.[^"\\]*)*"', # string
]

_tokens_re = re.compile(r'|'.join(['(' + token_re + ')' for token_re in _token_res]), re.DOTALL)


def _strip_comment(mo):
    if mo.group(1):
        return ''
    else:
        return mo.group(0)


def _strip_comments(data):
    '''Strip (non-standard) JSON comments.'''
    return _tokens_re.sub(_strip_comment, data)


assert _strip_comments('''// a comment
"// a comment in a string
"''') == '''
"// a comment in a string
"'''


def load(stream, strip_images = True, strip_comments = True):
    if strip_images:
        object_hook = strip_object_hook
    else:
        object_hook = None
    if strip_comments:
        data = stream.read()
        data = _strip_comments(data)
        return json.loads(data, strict=False, object_hook = object_hook)
    else:
        return json.load(stream, strict=False, object_hook = object_hook)


def main():
    optparser = argparse.ArgumentParser(
        description="Diff JSON format state dump files")
    optparser.add_argument("-k", "--keep-images",
        action="store_false", dest="strip_images", default=True,
        help="compare images")

    optparser.add_argument("ref_json", action="store",
        type=str, help="reference state file")
    optparser.add_argument("src_json", action="store",
        type=str, help="source state file")

    args = optparser.parse_args()

    a = load(open(args.ref_json, 'rt'), args.strip_images)
    b = load(open(args.src_json, 'rt'), args.strip_images)

    if False:
        dumper = Dumper()
        dumper.visit(a)

    differ = Differ()
    differ.visit(a, b)


if __name__ == '__main__':
    main()
