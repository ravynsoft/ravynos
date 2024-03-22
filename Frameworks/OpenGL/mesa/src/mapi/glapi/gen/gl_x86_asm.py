
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
import gl_XML, glX_XML

class PrintGenericStubs(gl_XML.gl_print_base):

    def __init__(self):
        gl_XML.gl_print_base.__init__(self)

        self.name = "gl_x86_asm.py (from Mesa)"
        self.license = license.bsd_license_template % ( \
"""Copyright (C) 1999-2001  Brian Paul   All Rights Reserved.
(C) Copyright IBM Corporation 2004, 2005""", "BRIAN PAUL, IBM")
        return


    def get_stack_size(self, f):
        size = 0
        for p in f.parameterIterator():
            if p.is_padding:
                continue

            size += p.get_stack_size()

        return size


    def printRealHeader(self):
        print('#include "x86/assyntax.h"')
        print('')
        print('#if defined(STDCALL_API)')
        print('#  define GL_PREFIX(n,n2) GLNAME(CONCAT(gl,n2))')
        print('#else')
        print('#  define GL_PREFIX(n,n2) GLNAME(CONCAT(gl,n))')
        print('#endif')
        print('')
        print('#define GL_OFFSET(x) CODEPTR(REGOFF(4 * x, EAX))')
        print('')
        print('#if defined(GNU_ASSEMBLER) && !defined(__MINGW32__) && !defined(__APPLE__)')
        print('#define GLOBL_FN(x) GLOBL x ; .type x, @function')
        print('#else')
        print('#define GLOBL_FN(x) GLOBL x')
        print('#endif')
        print('')
        print('')
        print('#ifdef REALLY_INITIAL_EXEC')
        print('')
        print('#ifdef GLX_X86_READONLY_TEXT')
        print('# define CTX_INSNS MOV_L(GS:(EAX), EAX)')
        print('#else')
        print('# define CTX_INSNS NOP /* Pad for init_glapi_relocs() */')
        print('#endif')
        print('')
        print('#  define GL_STUB(fn,off,fn_alt)\t\t\t\\')
        print('ALIGNTEXT16;\t\t\t\t\t\t\\')
        print('GLOBL_FN(GL_PREFIX(fn, fn_alt));\t\t\t\\')
        print('GL_PREFIX(fn, fn_alt):\t\t\t\t\t\\')
        print('\tCALL(_x86_get_dispatch) ;\t\t\t\\')
        print('\tCTX_INSNS ;					\\')
        print('\tJMP(GL_OFFSET(off))')
        print('')
        print('#else')
        print('#  define GL_STUB(fn,off,fn_alt)\t\t\t\\')
        print('ALIGNTEXT16;\t\t\t\t\t\t\\')
        print('GLOBL_FN(GL_PREFIX(fn, fn_alt));\t\t\t\\')
        print('GL_PREFIX(fn, fn_alt):\t\t\t\t\t\\')
        print('\tMOV_L(CONTENT(GLNAME(_glapi_Dispatch)), EAX) ;\t\\')
        print('\tTEST_L(EAX, EAX) ;\t\t\t\t\\')
        print('\tJE(1f) ;\t\t\t\t\t\\')
        print('\tJMP(GL_OFFSET(off)) ;\t\t\t\t\\')
        print('1:\tCALL(_glapi_get_dispatch) ;\t\t\t\\')
        print('\tJMP(GL_OFFSET(off))')
        print('#endif')
        print('')
        print('#ifdef HAVE_FUNC_ATTRIBUTE_ALIAS')
        print('#  define GL_STUB_ALIAS(fn,off,fn_alt,alias,alias_alt)\t\\')
        print('\t.globl\tGL_PREFIX(fn, fn_alt) ;\t\t\t\\')
        print('\t.set\tGL_PREFIX(fn, fn_alt), GL_PREFIX(alias, alias_alt)')
        print('#else')
        print('#  define GL_STUB_ALIAS(fn,off,fn_alt,alias,alias_alt)\t\\')
        print('    GL_STUB(fn, off, fn_alt)')
        print('#endif')
        print('')
        print('SEG_TEXT')
        print('')
        print('#ifdef REALLY_INITIAL_EXEC')
        print('')
        print('\tGLOBL\tGLNAME(_x86_get_dispatch)')
        print('\tHIDDEN(GLNAME(_x86_get_dispatch))')
        print('ALIGNTEXT16')
        print('GLNAME(_x86_get_dispatch):')
        print('\tcall	1f')
        print('1:\tpopl	%eax')
        print('\taddl	$_GLOBAL_OFFSET_TABLE_+[.-1b], %eax')
        print('\tmovl	_glapi_tls_Dispatch@GOTNTPOFF(%eax), %eax')
        print('\tret')
        print('')
        print('#else')
        print('EXTERN GLNAME(_glapi_Dispatch)')
        print('EXTERN GLNAME(_glapi_get_dispatch)')
        print('#endif')
        print('')

        print('#if !defined( GLX_X86_READONLY_TEXT )')
        print('\t\t.section\twtext, "awx", @progbits')
        print('#endif /* !defined( GLX_X86_READONLY_TEXT ) */')

        print('')
        print('\t\tALIGNTEXT16')
        print('\t\tGLOBL GLNAME(gl_dispatch_functions_start)')
        print('\t\tHIDDEN(GLNAME(gl_dispatch_functions_start))')
        print('GLNAME(gl_dispatch_functions_start):')
        print('')
        return


    def printRealFooter(self):
        print('')
        print('\t\tGLOBL\tGLNAME(gl_dispatch_functions_end)')
        print('\t\tHIDDEN(GLNAME(gl_dispatch_functions_end))')
        print('\t\tALIGNTEXT16')
        print('GLNAME(gl_dispatch_functions_end):')
        print('')
        print('#if defined (__ELF__) && defined (__linux__)')
        print('	.section .note.GNU-stack,"",%progbits')
        print('#endif')
        return


    def printBody(self, api):
        for f in api.functionIterateByOffset():
            name = f.dispatch_name()
            stack = self.get_stack_size(f)
            alt = "%s@%u" % (name, stack)

            print('\tGL_STUB(%s, %d, %s)' % (name, f.offset, alt))

            if not f.is_static_entry_point(f.name):
                print('\tHIDDEN(GL_PREFIX(%s, %s))' % (name, alt))


        for f in api.functionIterateByOffset():
            name = f.dispatch_name()
            stack = self.get_stack_size(f)
            alt = "%s@%u" % (name, stack)

            for n in f.entry_points:
                if f.is_static_entry_point(n):
                    if n != f.name:
                        alt2 = "%s@%u" % (n, stack)
                        text = '\tGL_STUB_ALIAS(%s, %d, %s, %s, %s)' % (n, f.offset, alt2, name, alt)

                        if f.has_different_protocol(n):
                            print('#if GLAPI_EXPORT_PROTO_ENTRY_POINTS')
                            print(text)
                            print('#endif')
                        else:
                            print(text)

        return

def _parser():
    parser = argparse.ArgumentParser()
    parser.add_argument('-f',
                        dest='filename',
                        default='gl_API.xml',
                        help='An XML file describing an API.')
    return parser.parse_args()


def main():
    args = _parser()
    printer = PrintGenericStubs()

    api = gl_XML.parse_GL_API(args.filename, glX_XML.glx_item_factory())
    printer.Print(api)


if __name__ == '__main__':
    main()
