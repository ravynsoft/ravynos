# Copyright (C) 2012 Intel Corporation
# Copyright (C) 2021 Advanced Micro Devices, Inc.
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

# This script generates the file api_beginend_init.h.

import argparse
import collections
import license
import gl_XML
import sys
import apiexec
import re


class PrintCode(gl_XML.gl_print_base):
    def __init__(self):
        super().__init__()

        self.name = 'api_beginend_init_h.py'
        self.license = license.bsd_license_template % (
            'Copyright (C) 2012 Intel Corporation\n'
            'Copyright (C) 2021 Advanced Micro Devices, Inc.',
            'AUTHORS')

    def printBody(self, api):
        # Collect SET_* calls by the condition under which they should
        # be called.
        settings_by_condition = collections.defaultdict(lambda: [])

        for f in api.functionIterateAll():
            if f.exec_flavor != 'beginend':
                continue

            condition = apiexec.get_api_condition(f)
            if not condition:
                continue

            if (condition == '_mesa_is_desktop_gl(ctx) || _mesa_is_gles2(ctx)' and
                re.match('VertexAttrib[1-4].*ARB', f.name)):
                # These functions should map to an *ES callback for GLES2.
                settings_by_condition['_mesa_is_desktop_gl(ctx)'].append(
                    'SET_{0}(tab, NAME({0}));'.format(f.name))
                settings_by_condition['_mesa_is_gles2(ctx)'].append(
                    'SET_{0}(tab, NAME_ES({0}));'.format(f.name))
            else:
                macro = ('NAME_CALLLIST' if f.name[0:8] == 'CallList' else
                         'NAME_AE' if f.name == 'ArrayElement' else 'NAME')
                settings_by_condition[condition].append(
                    'SET_{0}(tab, {1}({0}));'.format(f.name, macro))

        # Print out an if statement for each unique condition, with
        # the SET_* calls nested inside it.
        for condition in sorted(settings_by_condition.keys()):
            print('if ({0}) {{'.format(condition))
            for setting in sorted(settings_by_condition[condition]):
                print('   {0}'.format(setting))
            print('}')


if __name__ == '__main__':
    apiexec.print_glapi_file(PrintCode())
