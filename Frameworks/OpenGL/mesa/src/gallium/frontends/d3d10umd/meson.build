inc_d3d10umd = include_directories('.')
_c_args_d3d10umd = []

if cc.get_argument_syntax() == 'gcc'
  _c_args_d3d10umd += '-Wno-unknown-pragmas'
endif

inc_winddk = include_directories('../../../../include/winddk')

libd3d10umd = static_library(
  'd3d10umd',
  files(
    'Adapter.cpp',
    'Debug.cpp',
    'Device.cpp',
    'Draw.cpp',
    'DxgiFns.cpp',
    'Format.cpp',
    'InputAssembly.cpp',
    'OutputMerger.cpp',
    'Query.cpp',
    'Rasterizer.cpp',
    'Resource.cpp',
    'Shader.cpp',
    'ShaderDump.cpp',
    'ShaderParse.c',
    'ShaderTGSI.c',
    'D3DKMT.cpp',
  ),
  c_args : _c_args_d3d10umd,
  cpp_args : _c_args_d3d10umd,
  include_directories : [
    inc_include, inc_src, inc_gallium, inc_gallium_aux, inc_winddk,
  ],
)
