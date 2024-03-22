#!/usr/bin/env python3
# Copyright © 2021-2021 Yonggang Luo

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

gen_help = """Generates visual studio module definition file."""

import argparse

"""
For input template definition file
For gcc/x64,gcc/arm64,visual studio
`wglMakeCurrent@8                     @357` => `wglMakeCurrent @357`
`DrvCopyContext@12` => `DrvCopyContext`
`stw_get_device` => `stw_get_device`
For gcc/x86,gcc/arm
`wglMakeCurrent@8                     @357` => `wglMakeCurrent@8 @357 == wglMakeCurrent`
`DrvCopyContext@12` => `DrvCopyContext@12 == DrvCopyContext`
`stw_get_device` => `stw_get_device`

"""
def gen_vs_module_def(in_file: str, out_file: str, compiler_abi: str, compiler_id: str, cpu_family: str) -> None:
    out_file_lines = ['EXPORTS']
    with open(in_file, 'r', encoding='utf-8') as f:
        lines = f.readlines()
        for line in lines:
            line = line.strip()
            tokens = line.split(';')
            if not tokens:
                continue
            def_infos = [x for x in tokens[0].split(' ') if len(x) > 0]
            if not def_infos:
                if line:
                    out_file_lines.append('\t' + line)
                else:
                    out_file_lines.append('')
                continue
            name_infos = def_infos[0].split('@')
            if not name_infos:
                out_file_lines.append('\t;' + line)
                continue
            order_info = '' if len(def_infos) <= 1 else def_infos[1]
            if def_infos[0] != name_infos[0] and \
                (compiler_abi == 'gcc' and compiler_id != 'clang') and (cpu_family not in {'x86_64', 'aarch64'}):
                if order_info:
                    out_file_lines.append('\t' + def_infos[0] + ' ' + order_info + ' == ' + name_infos[0])
                else:
                    out_file_lines.append('\t' + def_infos[0] + ' == ' + name_infos[0])
            else:
                if order_info:
                    out_file_lines.append('\t' + name_infos[0] + ' ' + order_info)
                else:
                    out_file_lines.append('\t' + name_infos[0])
    with open(out_file, 'wb') as f:
        out_file_content = '\n'.join(out_file_lines) + '\n'
        f.write(out_file_content.encode('utf-8'))
'''
python ./bin/gen_vs_module_defs.py --in_file src/gallium/targets/libgl-gdi/opengl32.def.in --out_file src/gallium/targets/libgl-gdi/opengl32.def --compiler_abi gcc --cpu_family x86_64
python ./bin/gen_vs_module_defs.py --in_file src/gallium/targets/libgl-gdi/opengl32.def.in --out_file src/gallium/targets/libgl-gdi/opengl32.mingw.def --compiler_abi gcc --cpu_family x86

python ./bin/gen_vs_module_defs.py --in_file src/gallium/targets/osmesa/osmesa.def.in --out_file src/gallium/targets/osmesa/osmesa.def --compiler_abi gcc --cpu_family x86_64
python ./bin/gen_vs_module_defs.py --in_file src/gallium/targets/osmesa/osmesa.def.in --out_file src/gallium/targets/osmesa/osmesa.mingw.def --compiler_abi gcc --cpu_family x86

python ./bin/gen_vs_module_defs.py --in_file src/gallium/targets/wgl/gallium_wgl.def.in --out_file src/gallium/targets/wgl/gallium_wgl.def --compiler_abi gcc --cpu_family x86_64
python ./bin/gen_vs_module_defs.py --in_file src/gallium/targets/wgl/gallium_wgl.def.in --out_file src/gallium/targets/wgl/gallium_wgl.mingw.def --compiler_abi gcc --cpu_family x86

python ./bin/gen_vs_module_defs.py --in_file src/egl/main/egl.def.in --out_file src/egl/main/egl.def --compiler_abi gcc --cpu_family x86_64
python ./bin/gen_vs_module_defs.py --in_file src/egl/main/egl.def.in --out_file src/egl/main/egl.mingw.def --compiler_abi gcc --cpu_family x86

python ./bin/gen_vs_module_defs.py --in_file src/gallium/targets/lavapipe/vulkan_lvp.def.in --out_file src/gallium/targets/lavapipe/vulkan_lvp.def --compiler_abi gcc --cpu_family x86_64
python ./bin/gen_vs_module_defs.py --in_file src/gallium/targets/lavapipe/vulkan_lvp.def.in --out_file src/gallium/targets/lavapipe/vulkan_lvp.mingw.def --compiler_abi gcc --cpu_family x86

'''
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=gen_help)
    parser.add_argument('--in_file', help='input template module definition file')
    parser.add_argument('--out_file', help='output module definition file')
    parser.add_argument('--compiler_abi', help='compiler abi')
    parser.add_argument('--compiler_id', help='compiler id')
    parser.add_argument('--cpu_family', help='cpu family')
    args = parser.parse_args()
    # print(args)
    gen_vs_module_def(args.in_file, args.out_file, args.compiler_abi, args.compiler_id, args.cpu_family)
