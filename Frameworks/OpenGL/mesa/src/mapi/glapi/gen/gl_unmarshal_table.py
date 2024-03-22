# Copyright (C) 2012 Intel Corporation
# Copyright (C) 2022 Advanced Micro Devices, Inc.
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
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

import contextlib
import gl_XML
import license
import marshal_XML
import sys

header = """#include "glthread_marshal.h"
"""

current_indent = 0


def out(str):
    if str:
        print(' '*current_indent + str)
    else:
        print('')


@contextlib.contextmanager
def indent(delta=3):
    global current_indent
    current_indent += delta
    yield
    current_indent -= delta


class PrintCode(gl_XML.gl_print_base):
    def __init__(self):
        super(PrintCode, self).__init__()

        self.name = 'gl_marshal.py'
        self.license = license.bsd_license_template % (
            'Copyright (C) 2012 Intel Corporation', 'INTEL CORPORATION')

    def printRealHeader(self):
        print(header)

    def printRealFooter(self):
        pass

    def printBody(self, api):
        out('const _mesa_unmarshal_func _mesa_unmarshal_dispatch[NUM_DISPATCH_CMD] = {')
        with indent():
            for func in api.functionIterateAll():
                if func.marshal_flavor() in ('skip', 'sync'):
                    continue
                out('[DISPATCH_CMD_{0}] = (_mesa_unmarshal_func)_mesa_unmarshal_{0},'.format(func.name))
        out('};')

        # Print the string table of function names.
        out('')
        out('const char *_mesa_unmarshal_func_name[NUM_DISPATCH_CMD] = {')
        with indent():
            for func in api.functionIterateAll():
                if func.marshal_flavor() in ('skip', 'sync'):
                    continue
                out('[DISPATCH_CMD_{0}] = "{0}",'.format(func.name))
        out('};')


def show_usage():
    print('Usage: %s [file_name]' % sys.argv[0])
    sys.exit(1)


if __name__ == '__main__':
    try:
        file_name = sys.argv[1]
    except Exception:
        show_usage()

    printer = PrintCode()

    api = gl_XML.parse_GL_API(file_name, marshal_XML.marshal_item_factory())
    printer.Print(api)
