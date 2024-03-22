#! /usr/bin/env python3
# Taken from Crucible and modified to parse declarations

import argparse
import io
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile
from textwrap import dedent

class ShaderCompileError(RuntimeError):
    def __init__(self, *args):
        super(ShaderCompileError, self).__init__(*args)

target_env_re = re.compile(r'QO_TARGET_ENV\s+(\S+)')

stage_to_glslang_stage = {
    'VERTEX': 'vert',
    'TESS_CONTROL': 'tesc',
    'TESS_EVALUATION': 'tese',
    'GEOMETRY': 'geom',
    'FRAGMENT': 'frag',
    'COMPUTE': 'comp',
}

base_layout_qualifier_id_re = r'({0}\s*=\s*(?P<{0}>\d+))'
id_re = r'(?P<name_%d>[^(gl_)]\w+)'
type_re = r'(?P<dtype_%d>\w+)'
location_re = base_layout_qualifier_id_re.format('location')
component_re = base_layout_qualifier_id_re.format('component')
binding_re = base_layout_qualifier_id_re.format('binding')
set_re = base_layout_qualifier_id_re.format('set')
unk_re = r'\w+(=\d+)?'
layout_qualifier_re = r'layout\W*\((%s)+\)' % '|'.join([location_re, binding_re, set_re, unk_re, '[, ]+'])
ubo_decl_re = r'uniform\W+%s(\W*{)?(?P<type_ubo>)' % (id_re%0)
ssbo_decl_re = r'buffer\W+%s(\W*{)?(?P<type_ssbo>)' % (id_re%1)
image_buffer_decl_re = r'uniform\W+imageBuffer\w+%s;(?P<type_img_buf>)' % (id_re%2)
image_decl_re = r'uniform\W+image\w+\W+%s;(?P<type_img>)' % (id_re%3)
texture_buffer_decl_re = r'uniform\W+textureBuffer\w+%s;(?P<type_tex_buf>)' % (id_re%4)
combined_texture_sampler_decl_re = r'uniform\W+sampler\w+\W+%s;(?P<type_combined>)' % (id_re%5)
texture_decl_re = r'uniform\W+texture\w+\W+%s;(?P<type_tex>)' % (id_re%6)
sampler_decl_re = r'uniform\W+sampler\w+%s;(?P<type_samp>)' % (id_re%7)
input_re = r'in\W+%s\W+%s;(?P<type_in>)' % (type_re%0, id_re%8)
output_re = r'out\W+%s\W+%s;(?P<type_out>)' % (type_re%1, id_re%9)
match_decl_re = re.compile(layout_qualifier_re + r'\W*((' + r')|('.join([ubo_decl_re, ssbo_decl_re, image_buffer_decl_re, image_decl_re, texture_buffer_decl_re, combined_texture_sampler_decl_re, texture_decl_re, sampler_decl_re, input_re, output_re]) + r'))$')

class Shader:
    def __init__(self, stage):
        self.glsl = None
        self.stream = io.StringIO()
        self.stage = stage
        self.dwords = None
        self.target_env = ""
        self.declarations = []

    def add_text(self, s):
        self.stream.write(s)

    def finish_text(self, start_line, end_line):
        self.glsl = self.stream.getvalue()
        self.stream = None

        # Handle the QO_EXTENSION macro
        self.glsl = self.glsl.replace('QO_EXTENSION', '#extension')

        # Handle the QO_DEFINE macro
        self.glsl = self.glsl.replace('QO_DEFINE', '#define')

        m = target_env_re.search(self.glsl)
        if m:
            self.target_env = m.group(1)
        self.glsl = self.glsl.replace('QO_TARGET_ENV', '// --target-env')

        self.start_line = start_line
        self.end_line = end_line

    def __run_glslang(self, extra_args=[]):
        stage = stage_to_glslang_stage[self.stage]
        stage_flags = ['-S', stage]

        in_file = tempfile.NamedTemporaryFile(suffix='.'+stage)
        src = ('#version 450\n' + self.glsl).encode('utf-8')
        in_file.write(src)
        in_file.flush()
        out_file = tempfile.NamedTemporaryFile(suffix='.spirv')
        args = [glslang, '-H'] + extra_args + stage_flags
        if self.target_env:
            args += ['--target-env', self.target_env]
        args += ['-o', out_file.name, in_file.name]
        with subprocess.Popen(args,
                              stdout = subprocess.PIPE,
                              stderr = subprocess.PIPE,
                              stdin = subprocess.PIPE) as proc:

            out, err = proc.communicate(timeout=30)
            in_file.close()

            if proc.returncode != 0:
                # Unfortunately, glslang dumps errors to standard out.
                # However, since we don't really want to count on that,
                # we'll grab the output of both
                message = out.decode('utf-8') + '\n' + err.decode('utf-8')
                raise ShaderCompileError(message.strip())

            out_file.seek(0)
            spirv = out_file.read()
            out_file.close()
            return (spirv, out)

    def _parse_declarations(self):
        for line in self.glsl.splitlines():
            res = re.match(match_decl_re, line.lstrip().rstrip())
            if res == None:
                continue
            res = {k:v for k, v in res.groupdict().items() if v != None}
            name = [v for k, v in res.items() if k.startswith('name_')][0]
            data_type = ([v for k, v in res.items() if k.startswith('dtype_')] + [''])[0]
            decl_type = [k for k, v in res.items() if k.startswith('type_')][0][5:]
            location = int(res.get('location', 0))
            component = int(res.get('component', 0))
            binding = int(res.get('binding', 0))
            desc_set = int(res.get('set', 0))
            self.declarations.append('{"%s", "%s", QoShaderDeclType_%s, %d, %d, %d, %d}' %
                                     (name, data_type, decl_type, location, component, binding, desc_set))

    def compile(self):
        def dwords(f):
            while True:
                dword_str = f.read(4)
                if not dword_str:
                    return
                assert len(dword_str) == 4
                yield struct.unpack('I', dword_str)[0]

        (spirv, assembly) = self.__run_glslang()
        self.dwords = list(dwords(io.BytesIO(spirv)))
        self.assembly = str(assembly, 'utf-8')

        self._parse_declarations()

    def _dump_glsl_code(self, f):
        # Dump GLSL code for reference.  Use // instead of /* */
        # comments so we don't need to escape the GLSL code.
        f.write('// GLSL code:\n')
        f.write('//')
        for line in self.glsl.splitlines():
            f.write('\n// {0}'.format(line))
        f.write('\n\n')

    def _dump_spirv_code(self, f, var_name):
        f.write('/* SPIR-V Assembly:\n')
        f.write(' *\n')
        for line in self.assembly.splitlines():
            f.write(' * ' + line + '\n')
        f.write(' */\n')

        f.write('static const uint32_t {0}[] = {{'.format(var_name))
        line_start = 0
        while line_start < len(self.dwords):
            f.write('\n    ')
            for i in range(line_start, min(line_start + 6, len(self.dwords))):
                f.write(' 0x{:08x},'.format(self.dwords[i]))
            line_start += 6
        f.write('\n};\n')

    def dump_c_code(self, f):
        f.write('\n\n')
        var_prefix = '__qonos_shader{0}'.format(self.end_line)

        self._dump_glsl_code(f)
        self._dump_spirv_code(f, var_prefix + '_spir_v_src')
        f.write('static const QoShaderDecl {0}_decls[] = {{{1}}};\n'.format(var_prefix, ', '.join(self.declarations)))

        f.write(dedent("""\
            static const QoShaderModuleCreateInfo {0}_info = {{
                .spirvSize = sizeof({0}_spir_v_src),
                .pSpirv = {0}_spir_v_src,
                .declarationCount = sizeof({0}_decls) / sizeof({0}_decls[0]),
                .pDeclarations = {0}_decls,
            """.format(var_prefix)))

        f.write("    .stage = VK_SHADER_STAGE_{0}_BIT,\n".format(self.stage))

        f.write('};\n')

        f.write('#define __qonos_shader{0}_info __qonos_shader{1}_info\n'\
                .format(self.start_line, self.end_line))

token_exp = re.compile(r'(qoShaderModuleCreateInfoGLSL|qoCreateShaderModuleGLSL|\(|\)|,)')

class Parser:
    def __init__(self, f):
        self.infile = f
        self.paren_depth = 0
        self.shader = None
        self.line_number = 1
        self.shaders = []

        def tokenize(f):
            leftover = ''
            for line in f:
                pos = 0
                while True:
                    m = token_exp.search(line, pos)
                    if m:
                        if m.start() > pos:
                            leftover += line[pos:m.start()]
                        pos = m.end()

                        if leftover:
                            yield leftover
                            leftover = ''

                        yield m.group(0)

                    else:
                        leftover += line[pos:]
                        break

                self.line_number += 1

            if leftover:
                yield leftover

        self.token_iter = tokenize(self.infile)

    def handle_shader_src(self):
        paren_depth = 1
        for t in self.token_iter:
            if t == '(':
                paren_depth += 1
            elif t == ')':
                paren_depth -= 1
                if paren_depth == 0:
                    return

            self.current_shader.add_text(t)

    def handle_macro(self, macro):
        t = next(self.token_iter)
        assert t == '('

        start_line = self.line_number

        if macro == 'qoCreateShaderModuleGLSL':
            # Throw away the device parameter
            t = next(self.token_iter)
            t = next(self.token_iter)
            assert t == ','

        stage = next(self.token_iter).strip()

        t = next(self.token_iter)
        assert t == ','

        self.current_shader = Shader(stage)
        self.handle_shader_src()
        self.current_shader.finish_text(start_line, self.line_number)

        self.shaders.append(self.current_shader)
        self.current_shader = None

    def run(self):
        for t in self.token_iter:
            if t in ('qoShaderModuleCreateInfoGLSL', 'qoCreateShaderModuleGLSL'):
                self.handle_macro(t)

def open_file(name, mode):
    if name == '-':
        if mode == 'w':
            return sys.stdout
        elif mode == 'r':
            return sys.stdin
        else:
            assert False
    else:
        return open(name, mode)

def parse_args():
    description = dedent("""\
        This program scrapes a C file for any instance of the
        qoShaderModuleCreateInfoGLSL and qoCreateShaderModuleGLSL macaros,
        grabs the GLSL source code, compiles it to SPIR-V.  The resulting
        SPIR-V code is written to another C file as an array of 32-bit
        words.

        If '-' is passed as the input file or output file, stdin or stdout
        will be used instead of a file on disc.""")

    p = argparse.ArgumentParser(
            description=description,
            formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument('-o', '--outfile', default='-',
                        help='Output to the given file (default: stdout).')
    p.add_argument('--with-glslang', metavar='PATH',
                        default='glslangValidator',
                        dest='glslang',
                        help='Full path to the glslangValidator shader compiler.')
    p.add_argument('infile', metavar='INFILE')

    return p.parse_args()


args = parse_args()
infname = args.infile
outfname = args.outfile
glslang = args.glslang

with open_file(infname, 'r') as infile:
    parser = Parser(infile)
    parser.run()

for shader in parser.shaders:
    shader.compile()

with open_file(outfname, 'w') as outfile:
    outfile.write(dedent("""\
        /* ==========================  DO NOT EDIT!  ==========================
         *             This file is autogenerated by glsl_scraper.py.
         */

        #include <stdint.h>

        #define __QO_SHADER_INFO_VAR2(_line) __qonos_shader ## _line ## _info
        #define __QO_SHADER_INFO_VAR(_line) __QO_SHADER_INFO_VAR2(_line)

        #define qoShaderModuleCreateInfoGLSL(stage, ...)  \\
            __QO_SHADER_INFO_VAR(__LINE__)

        #define qoCreateShaderModuleGLSL(dev, stage, ...) \\
            __qoCreateShaderModule((dev), &__QO_SHADER_INFO_VAR(__LINE__))
        """))

    for shader in parser.shaders:
        shader.dump_c_code(outfile)
