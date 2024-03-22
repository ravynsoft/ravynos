# Copyright © 2022 Imagination Technologies Ltd.

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

pvr_xml_files = [
  'rogue_cdm.xml',
  'rogue_cr.xml',
  'rogue_ipf.xml',
  'rogue_kmd_stream.xml',
  'rogue_lls.xml',
  'rogue_pbestate.xml',
  'rogue_pds.xml',
  'rogue_ppp.xml',
  'rogue_texstate.xml',
  'rogue_vdm.xml',
]

pvr_xml_pack = []
foreach f : pvr_xml_files
  _name = '@0@.h'.format(f.split('.')[0])
  pvr_xml_pack += custom_target(
    _name,
    input : ['gen_pack_header.py', f],
    output : _name,
    command : [prog_python, '@INPUT@'],
    capture : true,
  )
endforeach

dep_csbgen = declare_dependency(sources : [pvr_xml_pack])
