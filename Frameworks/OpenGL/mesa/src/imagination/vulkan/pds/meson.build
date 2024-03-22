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


libpowervr_pds_files = files(
  'pvr_pds.c',
  'pvr_pds_disasm.c',
  'pvr_pds_printer.c',
  'pvr_pipeline_pds.c',
)

libpowervr_pds_includes = include_directories(
  '..',
  '.',
  'pvr_pds_programs',
)

libpowervr_pds = static_library(
  'pvr_pds',
  [libpowervr_pds_files],
  include_directories : [
    libpowervr_pds_includes,
    inc_include,
    inc_src,
    inc_imagination,
  ],
  c_args : [
    imagination_c_args,
    no_override_init_args,
  ],
  gnu_symbol_visibility : 'hidden',
  pic : true,
)
