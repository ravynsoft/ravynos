# Copyright © 2017 Intel Corporation

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

gen_xml_files = [
  'gen4.xml',
  'gen45.xml',
  'gen5.xml',
  'gen6.xml',
  'gen7.xml',
  'gen75.xml',
  'gen8.xml',
  'gen9.xml',
  'gen11.xml',
  'gen12.xml',
  'gen125.xml',
  'gen20.xml',
]

gen_xml_rt_files = [
  'gen125_rt.xml',
  'gen20_rt.xml',
]

gen_pack_header_deps = files('intel_genxml.py', 'util.py')
foreach f : gen_xml_files + gen_xml_rt_files
  gen_pack_header_deps += files(f)
endforeach

genX_xml_h = custom_target(
  'genX_xml.h',
  input : ['gen_zipped_file.py', gen_xml_files],
  output : 'genX_xml.h',
  command : [prog_python, '@INPUT@'],
  capture : true,
)

genX_bits_included_symbols = [
  # instructions
  'MI_BATCH_BUFFER_START::Batch Buffer Start Address',
  'MI_REPORT_PERF_COUNT::Memory Address',
  'MI_STORE_DATA_IMM::Address',
  'MI_STORE_DATA_IMM::Immediate Data',
  'MI_STORE_REGISTER_MEM::Memory Address',
  '3DSTATE_DEPTH_BUFFER::Surface Base Address',
  '3DSTATE_DEPTH_BUFFER::Surface Pitch',
  '3DSTATE_STENCIL_BUFFER::Surface Base Address',
  '3DSTATE_STENCIL_BUFFER::Surface Pitch',
  '3DSTATE_HIER_DEPTH_BUFFER::Surface Base Address',
  '3DSTATE_HIER_DEPTH_BUFFER::Surface Pitch',
  '3DSTATE_CLEAR_PARAMS',
  '3DSTATE_SO_BUFFER::Surface Base Address',
  '3DSTATE_SO_BUFFER::Stream Offset',
  '3DSTATE_CPSIZE_CONTROL_BUFFER::Surface Base Address',
  '3DSTATE_CPSIZE_CONTROL_BUFFER::Surface Pitch',
  # structures
  'RENDER_SURFACE_STATE::Surface Base Address',
  'RENDER_SURFACE_STATE::Surface Pitch',
  'RENDER_SURFACE_STATE::Auxiliary Surface Base Address',
  'RENDER_SURFACE_STATE::Auxiliary Surface Pitch',
  'RENDER_SURFACE_STATE::Clear Value Address',
  'RENDER_SURFACE_STATE::Red Clear Color',
  'RENDER_SURFACE_STATE::Green Clear Color',
  'RENDER_SURFACE_STATE::Blue Clear Color',
  'RENDER_SURFACE_STATE::Alpha Clear Color',
  'RENDER_SURFACE_STATE::Width',
  'RENDER_SURFACE_STATE::Height',
  'RENDER_SURFACE_STATE::Depth',
  'RENDER_SURFACE_STATE::Surface Type',
  'RENDER_SURFACE_STATE::Render Target View Extent',
  'CLEAR_COLOR',
  'VERTEX_BUFFER_STATE::Buffer Starting Address',
  'CPS_STATE',
  'RT_DISPATCH_GLOBALS::Hit Group Table',
  'RT_DISPATCH_GLOBALS::Miss Group Table',
  'RT_DISPATCH_GLOBALS::Callable Group Table',
  'RT_DISPATCH_GLOBALS::Launch Width',
  'RT_DISPATCH_GLOBALS::Launch Height',
  'RT_DISPATCH_GLOBALS::Launch Depth',
]

genX_bits_h = custom_target(
  'genX_bits.h',
  input : ['gen_bits_header.py', gen_xml_files, gen_xml_rt_files],
  output : 'genX_bits.h',
  command : [prog_python, '@INPUT@', '-o', '@OUTPUT@',
             '--include-symbols', ','.join(genX_bits_included_symbols)],
  depend_files: gen_pack_header_deps
)

gen_xml_pack = []
foreach f : gen_xml_files + gen_xml_rt_files
  _name = '@0@_pack.h'.format(f.split('.')[0])
  gen_xml_pack += custom_target(
    _name,
    input : ['gen_pack_header.py', f],
    output : _name,
    command : [prog_python, '@INPUT@', '--engines=render,blitter,video'],
    capture : true,
    depend_files: gen_pack_header_deps
  )
endforeach

gen_pack_header_py = files('gen_pack_header.py')

idep_genxml = declare_dependency(sources : [gen_xml_pack, genX_bits_h, genX_xml_h])

foreach f : gen_xml_files + gen_xml_rt_files
  test(
    'genxml sort @0@'.format(f),
    prog_python,
    args : [files('gen_sort_tags.py'), files(f), '--validate', '--quiet'],
    suite : ['intel'],
  )
endforeach
