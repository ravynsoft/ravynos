
# Copyright (C) 2009 Chia-I Wu <olv@0xlab.org>
# All Rights Reserved.
#
# This is based on extension_helper.py by Ian Romanick.
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# on the rights to use, copy, modify, merge, publish, distribute, sub
# license, and/or sell copies of the Software, and to permit persons to whom
# the Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
# IBM AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

import argparse

import license
import gl_XML


class PrintGlRemap(gl_XML.gl_print_base):
    def __init__(self):
        gl_XML.gl_print_base.__init__(self)

        self.name = "remap_helper.py (from Mesa)"
        self.license = license.bsd_license_template % ("Copyright (C) 2009 Chia-I Wu <olv@0xlab.org>", "Chia-I Wu")
        return


    def printRealHeader(self):
        print('#include "main/dispatch.h"')
        print('#include "main/remap.h"')
        print('')
        return


    def printBody(self, api):
        pool_indices = {}

        print('/* this is internal to remap.c */')
        print('#ifndef need_MESA_remap_table')
        print('#error Only remap.c should include this file!')
        print('#endif /* need_MESA_remap_table */')
        print('')

        print('')
        print('static const char _mesa_function_pool[] =')

        # output string pool
        index = 0;
        for f in api.functionIterateAll():
            pool_indices[f] = index

            # a function has either assigned offset, fixed offset,
            # or no offset
            if f.assign_offset:
                comments = "will be remapped"
            elif f.offset > 0:
                comments = "offset %d" % f.offset
            else:
                comments = "dynamic"

            print('   /* _mesa_function_pool[%d]: %s (%s) */' \
                            % (index, f.name, comments))
            print('   "gl%s\\0"' % f.entry_points[0])
            index += len(f.entry_points[0]) + 3
        print('   ;')
        print('')

        print('/* these functions need to be remapped */')
        print('static const struct gl_function_pool_remap MESA_remap_table_functions[] = {')
        # output all functions that need to be remapped
        # iterate by offsets so that they are sorted by remap indices
        for f in api.functionIterateByOffset():
            if not f.assign_offset:
                continue
            print('   { %5d, %s_remap_index },' \
                            % (pool_indices[f], f.name))
        print('   {    -1, -1 }')
        print('};')
        print('')
        return


def _parser():
    """Parse input options and return a namsepace."""
    parser = argparse.ArgumentParser()
    parser.add_argument('-f', '--filename',
                        default="gl_API.xml",
                        metavar="input_file_name",
                        dest='file_name',
                        help="An xml description file.")
    return parser.parse_args()


def main():
    """Main function."""
    args = _parser()

    api = gl_XML.parse_GL_API(args.file_name)

    printer = PrintGlRemap()
    printer.Print(api)


if __name__ == '__main__':
    main()
