
# (C) Copyright IBM Corporation 2004, 2005
# All Rights Reserved.
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
#
# Authors:
#    Ian Romanick <idr@us.ibm.com>

import argparse

import license
import gl_XML
import glX_XML


class PrintGlProcs(gl_XML.gl_print_base):
    def __init__(self, es=False):
        gl_XML.gl_print_base.__init__(self)

        self.es = es
        self.name = "gl_procs.py (from Mesa)"
        self.license = license.bsd_license_template % ( \
"""Copyright (C) 1999-2001  Brian Paul   All Rights Reserved.
(C) Copyright IBM Corporation 2004, 2006""", "BRIAN PAUL, IBM")

    def printRealHeader(self):
        print("""
/* This file is only included by glapi.c and is used for
 * the GetProcAddress() function
 */

typedef struct {
    GLint Name_offset;
#if defined(NEED_FUNCTION_POINTER) || defined(GLX_INDIRECT_RENDERING)
    _glapi_proc Address;
#endif
    GLuint Offset;
} glprocs_table_t;

#if   !defined(NEED_FUNCTION_POINTER) && !defined(GLX_INDIRECT_RENDERING)
#  define NAME_FUNC_OFFSET(n,f1,f2,f3,o) { n , o }
#elif  defined(NEED_FUNCTION_POINTER) && !defined(GLX_INDIRECT_RENDERING)
#  define NAME_FUNC_OFFSET(n,f1,f2,f3,o) { n , (_glapi_proc) f1 , o }
#elif  defined(NEED_FUNCTION_POINTER) &&  defined(GLX_INDIRECT_RENDERING)
#  define NAME_FUNC_OFFSET(n,f1,f2,f3,o) { n , (_glapi_proc) f2 , o }
#elif !defined(NEED_FUNCTION_POINTER) &&  defined(GLX_INDIRECT_RENDERING)
#  define NAME_FUNC_OFFSET(n,f1,f2,f3,o) { n , (_glapi_proc) f3 , o }
#endif

""")
        return

    def printRealFooter(self):
        print('')
        print('#undef NAME_FUNC_OFFSET')
        return

    def printFunctionString(self, name):
        print('    "gl%s\\0"' % (name))

    def printBody(self, api):
        print('')
        print('static const char gl_string_table[] =')

        base_offset = 0
        table = []
        for func in api.functionIterateByOffset():
            name = func.dispatch_name()
            self.printFunctionString(func.name)
            table.append((base_offset, "gl" + name, "gl" + name, "NULL", func.offset))

            # The length of the function's name, plus 2 for "gl",
            # plus 1 for the NUL.

            base_offset += len(func.name) + 3


        for func in api.functionIterateByOffset():
            for n in func.entry_points:
                if n != func.name:
                    name = func.dispatch_name()
                    self.printFunctionString( n )

                    if func.has_different_protocol(n):
                        alt_name = "gl" + func.static_glx_name(n)
                        table.append((base_offset, "gl" + name, alt_name, alt_name, func.offset))
                    else:
                        table.append((base_offset, "gl" + name, "gl" + name, "NULL", func.offset))

                    base_offset += len(n) + 3


        print('    ;')
        print('')
        print('')
        print('#if defined(NEED_FUNCTION_POINTER) || defined(GLX_INDIRECT_RENDERING)')
        for func in api.functionIterateByOffset():
            for n in func.entry_points:
                if (not func.is_static_entry_point(func.name)) or (func.has_different_protocol(n) and not func.is_static_entry_point(n)):
                    print('%s GLAPIENTRY gl_dispatch_stub_%u(%s);' % (func.return_type, func.offset, func.get_parameter_string()))
                    break

        if self.es:
            categories = {}
            for func in api.functionIterateByOffset():
                for n in func.entry_points:
                    cat, num = api.get_category_for_name(n)
                    if (cat.startswith("es") or cat.startswith("GL_OES")):
                        if cat not in categories:
                            categories[cat] = []
                        proto = 'GLAPI %s GLAPIENTRY %s(%s);' \
                                        % (func.return_type, "gl" + n, func.get_parameter_string(n))
                        categories[cat].append(proto)
            if categories:
                print('')
                print('/* OpenGL ES specific prototypes */')
                print('')
                keys = sorted(categories.keys())
                for key in keys:
                    print('/* category %s */' % key)
                    print("\n".join(categories[key]))
                print('')

        print('#endif /* defined(NEED_FUNCTION_POINTER) || defined(GLX_INDIRECT_RENDERING) */')

        print('')
        print('static const glprocs_table_t static_functions[] = {')

        for info in table:
            print('    NAME_FUNC_OFFSET(%5u, %s, %s, %s, %d),' % info)

        print('    NAME_FUNC_OFFSET(-1, NULL, NULL, NULL, 0)')
        print('};')
        return


def _parser():
    """Parse arguments and return a namepsace."""

    parser = argparse.ArgumentParser()
    parser.add_argument('-f', '--filename',
                        default='gl_API.xml',
                        metavar="input_file_name",
                        dest='file_name',
                        help="Path to an XML description of OpenGL API.")
    parser.add_argument('-c', '--es-version',
                        dest='es',
                        action="store_true",
                        help="filter functions for es")
    return parser.parse_args()


def main():
    """Main function."""
    args = _parser()
    api = gl_XML.parse_GL_API(args.file_name, glX_XML.glx_item_factory())
    PrintGlProcs(args.es).Print(api)


if __name__ == '__main__':
    main()
