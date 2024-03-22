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

nine_st_files = files(
  'adapter9.c',
  'authenticatedchannel9.c',
  'basetexture9.c',
  'buffer9.c',
  'cryptosession9.c',
  'cubetexture9.c',
  'device9.c',
  'device9ex.c',
  'device9video.c',
  'guid.c',
  'indexbuffer9.c',
  'iunknown.c',
  'nine_buffer_upload.c',
  'nine_debug.c',
  'nine_dump.c',
  'nineexoverlayextension.c',
  'nine_ff.c',
  'nine_helpers.c',
  'nine_lock.c',
  'nine_memory_helper.c',
  'nine_pipe.c',
  'nine_quirk.c',
  'nine_queue.c',
  'nine_shader.c',
  'nine_state.c',
  'pixelshader9.c',
  'query9.c',
  'resource9.c',
  'stateblock9.c',
  'surface9.c',
  'swapchain9.c',
  'swapchain9ex.c',
  'texture9.c',
  'threadpool.c',
  'vertexbuffer9.c',
  'vertexdeclaration9.c',
  'vertexshader9.c',
  'volume9.c',
  'volumetexture9.c',
)

libnine_st = static_library(
  'nine_st',
  nine_st_files,
  gnu_symbol_visibility : 'hidden',
  include_directories : [
    inc_d3d9, inc_gallium, inc_include, inc_src, inc_gallium_aux,
  ],
  dependencies : [
    dep_thread, idep_nir, idep_nir_headers
  ],
  link_with : [
    libmesa
  ]
)
