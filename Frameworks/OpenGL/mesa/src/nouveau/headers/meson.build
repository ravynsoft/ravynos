nvk_classes = [
  'cl902d',
  'cl9039',
  'cl906f',
  'cl9097',
  'cl90b5',
  'cl90c0',
  'cla0b5',
  'cla097',
  'clb097',
  'clb0c0',
  'clb197',
  'clb1c0',
  'clc097',
  'clc1b5',
  'cla0c0',
  'clc0c0',
  'clc397',
  'clc3c0',
  'clc597',
  'clc5c0',
  'clc6c0',
]

nvk_cl_header_depend_files = [
  files('class_parser.py')
]

cl_generated = []
foreach cl : nvk_classes
  cl_generated += custom_target(
    cl + '.h',
    input : ['class_parser.py', 'nvidia/classes/'+cl+'.h'],
    output : ['nvk_'+cl+'.h', 'nvk_'+cl+'.c'],
    command : [prog_python, '@INPUT0@', '--in_h', '@INPUT1@',
               '--out_h', '@OUTPUT0@', '--out_c', '@OUTPUT1@'],
    depend_files: nvk_cl_header_depend_files,
  )
endforeach

_libnvidia_headers = static_library(
  'nvidia_headers',
  ['nv_push.c', cl_generated],
  include_directories : ['.', 'nvidia/classes', inc_include, inc_src],
  c_args : [no_override_init_args],
  gnu_symbol_visibility : 'hidden',
)

idep_nvidia_headers = declare_dependency(
  include_directories : include_directories('.', 'nvidia/classes'),
  sources : cl_generated,
  link_with : _libnvidia_headers,
)
