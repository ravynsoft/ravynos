'''
Generates dumper for the SVGA 3D command stream using pygccxml.

Jose Fonseca <jfonseca@vmware.com>
'''

copyright = '''
/**********************************************************
 * Copyright 2009 VMware, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************/
 '''

import os
import sys

from pygccxml import parser
from pygccxml import declarations

from pygccxml.declarations import algorithm
from pygccxml.declarations import decl_visitor
from pygccxml.declarations import type_traits
from pygccxml.declarations import type_visitor


enums = True


class decl_dumper_t(decl_visitor.decl_visitor_t):

    def __init__(self, instance = '', decl = None):
        decl_visitor.decl_visitor_t.__init__(self)
        self._instance = instance
        self.decl = decl

    def clone(self):
        return decl_dumper_t(self._instance, self.decl)

    def visit_class(self):
        class_ = self.decl
        assert self.decl.class_type in ('struct', 'union')

        for variable in class_.variables():
            if variable.name != '':
                #print 'variable = %r' % variable.name
                dump_type(self._instance + '.' + variable.name, variable.type)

    def visit_enumeration(self):
        if enums:
            print '   switch(%s) {' % ("(*cmd)" + self._instance,)
            for name, value in self.decl.values:
                print '   case %s:' % (name,)
                print '      _debug_printf("\\t\\t%s = %s\\n");' % (self._instance, name)
                print '      break;'
            print '   default:'
            print '      _debug_printf("\\t\\t%s = %%i\\n", %s);' % (self._instance, "(*cmd)" + self._instance)
            print '      break;'
            print '   }'
        else:
            print '   _debug_printf("\\t\\t%s = %%i\\n", %s);' % (self._instance, "(*cmd)" + self._instance)


def dump_decl(instance, decl):
    dumper = decl_dumper_t(instance, decl)
    algorithm.apply_visitor(dumper, decl)


class type_dumper_t(type_visitor.type_visitor_t):

    def __init__(self, instance, type_):
        type_visitor.type_visitor_t.__init__(self)
        self.instance = instance
        self.type = type_

    def clone(self):
        return type_dumper_t(self.instance, self.type)

    def visit_char(self):
        self.print_instance('%i')
        
    def visit_unsigned_char(self):
        self.print_instance('%u')

    def visit_signed_char(self):
        self.print_instance('%i')
    
    def visit_wchar(self):
        self.print_instance('%i')
        
    def visit_short_int(self):
        self.print_instance('%i')
        
    def visit_short_unsigned_int(self):
        self.print_instance('%u')
        
    def visit_bool(self):
        self.print_instance('%i')
        
    def visit_int(self):
        self.print_instance('%i')
        
    def visit_unsigned_int(self):
        self.print_instance('%u')
        
    def visit_long_int(self):
        self.print_instance('%li')
        
    def visit_long_unsigned_int(self):
        self.print_instance('%lu')
        
    def visit_long_long_int(self):
        self.print_instance('%lli')
        
    def visit_long_long_unsigned_int(self):
        self.print_instance('%llu')
        
    def visit_float(self):
        self.print_instance('%f')
        
    def visit_double(self):
        self.print_instance('%f')
        
    def visit_array(self):
        for i in range(type_traits.array_size(self.type)):
            dump_type(self.instance + '[%i]' % i, type_traits.base_type(self.type))

    def visit_pointer(self):
        self.print_instance('%p')

    def visit_declarated(self):
        #print 'decl = %r' % self.type.decl_string
        decl = type_traits.remove_declarated(self.type)
        dump_decl(self.instance, decl)

    def print_instance(self, format):
        print '   _debug_printf("\\t\\t%s = %s\\n", %s);' % (self.instance, format, "(*cmd)" + self.instance)


def dump_type(instance, type_):
    type_ = type_traits.remove_alias(type_)
    visitor = type_dumper_t(instance, type_)
    algorithm.apply_visitor(visitor, type_)


def dump_struct(decls, class_):
    print 'static void'
    print 'dump_%s(const %s *cmd)' % (class_.name, class_.name)
    print '{'
    dump_decl('', class_)
    print '}'
    print ''


cmds = [
    ('SVGA_3D_CMD_SURFACE_DEFINE', 'SVGA3dCmdDefineSurface', (), 'SVGA3dSize'),
    ('SVGA_3D_CMD_SURFACE_DESTROY', 'SVGA3dCmdDestroySurface', (), None),
    ('SVGA_3D_CMD_SURFACE_COPY', 'SVGA3dCmdSurfaceCopy', (), 'SVGA3dCopyBox'),
    ('SVGA_3D_CMD_SURFACE_STRETCHBLT', 'SVGA3dCmdSurfaceStretchBlt', (), None),
    ('SVGA_3D_CMD_SURFACE_DMA', 'SVGA3dCmdSurfaceDMA', (), 'SVGA3dCopyBox'),
    ('SVGA_3D_CMD_CONTEXT_DEFINE', 'SVGA3dCmdDefineContext', (), None),
    ('SVGA_3D_CMD_CONTEXT_DESTROY', 'SVGA3dCmdDestroyContext', (), None),
    ('SVGA_3D_CMD_SETTRANSFORM', 'SVGA3dCmdSetTransform', (), None),
    ('SVGA_3D_CMD_SETZRANGE', 'SVGA3dCmdSetZRange', (), None),
    ('SVGA_3D_CMD_SETRENDERSTATE', 'SVGA3dCmdSetRenderState', (), 'SVGA3dRenderState'),
    ('SVGA_3D_CMD_SETRENDERTARGET', 'SVGA3dCmdSetRenderTarget', (), None),
    ('SVGA_3D_CMD_SETTEXTURESTATE', 'SVGA3dCmdSetTextureState', (), 'SVGA3dTextureState'),
    ('SVGA_3D_CMD_SETMATERIAL', 'SVGA3dCmdSetMaterial', (), None),
    ('SVGA_3D_CMD_SETLIGHTDATA', 'SVGA3dCmdSetLightData', (), None),
    ('SVGA_3D_CMD_SETLIGHTENABLED', 'SVGA3dCmdSetLightEnabled', (), None),
    ('SVGA_3D_CMD_SETVIEWPORT', 'SVGA3dCmdSetViewport', (), None),
    ('SVGA_3D_CMD_SETCLIPPLANE', 'SVGA3dCmdSetClipPlane', (), None),
    ('SVGA_3D_CMD_CLEAR', 'SVGA3dCmdClear', (), 'SVGA3dRect'),
    ('SVGA_3D_CMD_PRESENT', 'SVGA3dCmdPresent', (), 'SVGA3dCopyRect'),
    ('SVGA_3D_CMD_SHADER_DEFINE', 'SVGA3dCmdDefineShader', (), None),
    ('SVGA_3D_CMD_SHADER_DESTROY', 'SVGA3dCmdDestroyShader', (), None),
    ('SVGA_3D_CMD_SET_SHADER', 'SVGA3dCmdSetShader', (), None),
    ('SVGA_3D_CMD_SET_SHADER_CONST', 'SVGA3dCmdSetShaderConst', (), None),
    ('SVGA_3D_CMD_DRAW_PRIMITIVES', 'SVGA3dCmdDrawPrimitives', (('SVGA3dVertexDecl', 'numVertexDecls'), ('SVGA3dPrimitiveRange', 'numRanges')), 'SVGA3dVertexDivisor'),
    ('SVGA_3D_CMD_SETSCISSORRECT', 'SVGA3dCmdSetScissorRect', (), None),
    ('SVGA_3D_CMD_BEGIN_QUERY', 'SVGA3dCmdBeginQuery', (), None),
    ('SVGA_3D_CMD_END_QUERY', 'SVGA3dCmdEndQuery', (), None),
    ('SVGA_3D_CMD_WAIT_FOR_QUERY', 'SVGA3dCmdWaitForQuery', (), None),
    #('SVGA_3D_CMD_PRESENT_READBACK', None, (), None),
    ('SVGA_3D_CMD_BLIT_SURFACE_TO_SCREEN', 'SVGA3dCmdBlitSurfaceToScreen', (), 'SVGASignedRect'),
]

def dump_cmds():
    print r'''
void            
svga_dump_command(uint32_t cmd_id, const void *data, uint32_t size)
{
   const uint8_t *body = (const uint8_t *)data;
   const uint8_t *next = body + size;
'''
    print '   switch(cmd_id) {'
    indexes = 'ijklmn'
    for id, header, body, footer in cmds:
        print '   case %s:' % id
        print '      _debug_printf("\\t%s\\n");' % id
        print '      {'
        print '         const %s *cmd = (const %s *)body;' % (header, header)
        if len(body):
            print '         unsigned ' + ', '.join(indexes[:len(body)]) + ';'
        print '         dump_%s(cmd);' % header
        print '         body = (const uint8_t *)&cmd[1];'
        for i in range(len(body)):
            struct, count = body[i]
            idx = indexes[i]
            print '         for(%s = 0; %s < cmd->%s; ++%s) {' % (idx, idx, count, idx)
            print '            dump_%s((const %s *)body);' % (struct, struct)
            print '            body += sizeof(%s);' % struct
            print '         }'
        if footer is not None:
            print '         while(body + sizeof(%s) <= next) {' % footer
            print '            dump_%s((const %s *)body);' % (footer, footer)
            print '            body += sizeof(%s);' % footer
            print '         }'
        if id == 'SVGA_3D_CMD_SHADER_DEFINE':
            print '         svga_shader_dump((const uint32_t *)body,'
            print '                          (unsigned)(next - body)/sizeof(uint32_t),'
            print '                          FALSE);'
            print '         body = next;'
        print '      }'
        print '      break;'
    print '   default:'
    print '      _debug_printf("\\t0x%08x\\n", cmd_id);'
    print '      break;'
    print '   }'
    print r'''
   while(body + sizeof(uint32_t) <= next) {
      _debug_printf("\t\t0x%08x\n", *(const uint32_t *)body);
      body += sizeof(uint32_t);
   }
   while(body + sizeof(uint32_t) <= next)
      _debug_printf("\t\t0x%02x\n", *body++);
}
'''
    print r'''
void            
svga_dump_commands(const void *commands, uint32_t size)
{
   const uint8_t *next = commands;
   const uint8_t *last = next + size;
   
   assert(size % sizeof(uint32_t) == 0);
   
   while(next < last) {
      const uint32_t cmd_id = *(const uint32_t *)next;

      if(SVGA_3D_CMD_BASE <= cmd_id && cmd_id < SVGA_3D_CMD_MAX) {
         const SVGA3dCmdHeader *header = (const SVGA3dCmdHeader *)next;
         const uint8_t *body = (const uint8_t *)&header[1];

         next = body + header->size;
         if(next > last)
            break;

         svga_dump_command(cmd_id, body, header->size);
      }
      else if(cmd_id == SVGA_CMD_FENCE) {
         _debug_printf("\tSVGA_CMD_FENCE\n");
         _debug_printf("\t\t0x%08x\n", ((const uint32_t *)next)[1]);
         next += 2*sizeof(uint32_t);
      }
      else {
         _debug_printf("\t0x%08x\n", cmd_id);
         next += sizeof(uint32_t);
      }
   }
}
'''

def main():
    print copyright.strip()
    print
    print '/**'
    print ' * @file'
    print ' * Dump SVGA commands.'
    print ' *'
    print ' * Generated automatically from svga3d_reg.h by svga_dump.py.'
    print ' */'
    print
    print '#include "svga_types.h"'
    print '#include "svga_shader_dump.h"'
    print '#include "svga3d_reg.h"'
    print
    print '#include "util/u_debug.h"'
    print '#include "svga_dump.h"'
    print

    config = parser.config_t(
        include_paths = ['../../../include', '../include'],
        compiler = 'gcc',
    )

    headers = [
        'svga_types.h', 
        'svga3d_reg.h', 
    ]

    decls = parser.parse(headers, config, parser.COMPILATION_MODE.ALL_AT_ONCE)
    global_ns = declarations.get_global_namespace(decls)

    names = set()
    for id, header, body, footer in cmds:
        names.add(header)
        for struct, count in body:
            names.add(struct)
        if footer is not None:
            names.add(footer)

    for class_ in global_ns.classes(lambda decl: decl.name in names):
        dump_struct(decls, class_)

    dump_cmds()


if __name__ == '__main__':
    main()
