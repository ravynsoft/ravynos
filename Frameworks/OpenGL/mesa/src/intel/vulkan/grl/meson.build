# Copyright © 2021 Intel Corporation

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

fs = import('fs')

grl_lib_files = [
  'gpu/libs/libraries.grl',
]

grl_grl_files = [
  'gpu/build_leaf.grl',
  'gpu/build_primref.grl',
#  'gpu/build_refit.grl',
  'gpu/copy.grl',
#  'gpu/grl_api_interface_verify.grl',
  'gpu/misc.grl',
#  'gpu/morton_builder.grl',
#  'gpu/msb_radix_bitonic_sort.grl',
  'gpu/new_sah_builder.grl',
  'gpu/postbuild_info.grl',
#  'gpu/presplit.grl',
#  'gpu/radix_sort.grl',
#  'gpu/rebraid.grl',
#  'gpu/traversal_shader.grl',
]

grl_lib_args = []
foreach libfile : grl_lib_files
  grl_lib_args += '--library'
  grl_lib_args += files(libfile)
endforeach

grl_genX_files = [
  'genX_grl_dispatch.c',
  'genX_grl_uuid.cpp',
]

grl_lib_args = []
foreach libfile : grl_lib_files
  grl_lib_args += '--library'
  grl_lib_args += files(libfile)
endforeach

grl_cl_kernel_h = custom_target(
  'grl_cl_kernel.h',
  input : ['grl_cl_kernel_gen.py', grl_grl_files, grl_lib_files],
  output : 'grl_cl_kernel.h',
  command : [
    prog_python, '@INPUT0@', '--out-h', '@OUTPUT@',
    grl_lib_args, files(grl_grl_files),
  ],
)

has_ply = run_command(
  prog_python, '-c',
  '''
import ply
  ''', check : false)
if has_ply.returncode() != 0
  error('Python (3.x) ply module required to build GRL kernels.')
endif

r = run_command(prog_python, 'grl_cl_kernel_gen.py',
                grl_lib_args, '--ls-kernels', grl_grl_files, check : false)
assert(r.returncode() == 0, 'Failed to fetch GRL CL kernels')
grl_kernels = r.stdout().strip().split()

grl_metakernel_c = []
grl_metakernel_h = []
foreach grl_file : grl_grl_files
  base_outfile = 'grl_metakernel_' + fs.replace_suffix(fs.name(grl_file), '')
  outfiles = custom_target(
    base_outfile,
    input : ['grl_metakernel_gen.py', grl_file, grl_lib_files],
    output : [base_outfile + '.h', base_outfile + '.c'],
    command : [
      prog_python, '@INPUT0@', '--out-h', '@OUTPUT0@',
      '--out-c', '@OUTPUT1@', grl_lib_args, '@INPUT1@',
    ],
  )
  grl_metakernel_h += outfiles[0]
  grl_metakernel_c += outfiles[1]
endforeach

grl_genX_libs = []
foreach t : [['125', 'gfx125', 'dg2']]
  verX10 = t[0]
  genX_prefix = t[1]
  platform = t[2]

  grl_compiled_cl_kernels = []
  foreach k : grl_kernels
    # get_cl_files dumps out filename:entrypoint:libfile1,libfile2,libfile3
    cl_file = k.split(':')[0]
    entrypoint = k.split(':')[1]
    library_files = k.split(':')[2]
    kernel_prefix = '_'.join([
      genX_prefix,
      fs.replace_suffix(cl_file, '').replace('gpu/', '').replace('/', '_'),
      entrypoint
    ])
    input_args = [ files(cl_file), ]
    if library_files != ''
      foreach lib_file : library_files.split(',')
        input_args += [ lib_file ]
      endforeach
    endif
    prepended_input_args = []
    foreach input_arg : input_args
      prepended_input_args += ['--in', input_arg]
    endforeach
    outfile = kernel_prefix + '.h'
    grl_compiled_cl_kernels += custom_target(
      outfile,
      input : cl_file,
      output : outfile,
      command : [
        prog_intel_clc, '-p', platform, '--prefix', kernel_prefix,
        '-e', entrypoint, prepended_input_args, '-o', '@OUTPUT@', '--',
        '-cl-std=cl2.0', '-D__OPENCL_VERSION__=200',
        '-DMAX_HW_SIMD_WIDTH=16', '-DMAX_WORKGROUP_SIZE=16',
        '-I' + join_paths(meson.current_source_dir(), 'gpu'),
        '-I' + join_paths(meson.current_source_dir(), 'include'),
        '-include', 'opencl-c.h', # added to bypass build failure from clang15
                                   # without modifying grl source code, remove
                                   # if fixed there
      ],
      env: ['MESA_SHADER_CACHE_DISABLE=true',
            'MESA_SPIRV_LOG_LEVEL=error'],
      depends : dep_prog_intel_clc
    )
  endforeach

  grl_cl_kernel_c = custom_target(
    'grl_@0@_cl_kernel.c'.format(genX_prefix),
    input : ['grl_cl_kernel_gen.py', grl_grl_files, grl_lib_files],
    output : 'grl_@0@_cl_kernel.c'.format(genX_prefix),
    command : [
      prog_python, '@INPUT0@', '--out-c', '@OUTPUT@',
      grl_lib_args, '--prefix', genX_prefix, files(grl_grl_files),
    ],
  )

  grl_genX_libs += static_library(
    'grl_@0@'.format(genX_prefix),
    [grl_cl_kernel_h, grl_compiled_cl_kernels, grl_cl_kernel_c,
     grl_genX_files, grl_metakernel_c, grl_metakernel_h],
    include_directories : [
      inc_include, inc_src,
      inc_intel,
    ],
    c_args : [
      no_override_init_args, sse2_args,
      '-DGFX_VERx10=@0@'.format(verX10),
    ],
    cpp_args : [
      sse2_args,
      '-DGFX_VERx10=@0@'.format(verX10),
    ],
    dependencies : [
      dep_valgrind, idep_nir_headers, idep_vulkan_util_headers, idep_vulkan_wsi_headers,
      idep_vulkan_runtime_headers, idep_anv_headers,
    ],
    gnu_symbol_visibility : 'hidden',
  )
endforeach

libgrl_deps = [
  dep_valgrind,
  idep_nir_headers,
  idep_vulkan_util_headers,
  idep_vulkan_wsi_headers,
]

libgrl = static_library(
  'grl',
  [grl_cl_kernel_h],
  include_directories : [
    inc_include, inc_src, inc_intel,
  ],
  link_whole : [grl_genX_libs],
  dependencies : [libgrl_deps, idep_anv_headers],
)
idep_grl = declare_dependency(
  link_with : libgrl,
  dependencies : libgrl_deps,
  sources : [grl_metakernel_h, grl_cl_kernel_h],
  include_directories : include_directories('include', 'gpu'),
)
