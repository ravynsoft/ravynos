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

if with_shared_glapi
  files_glx_test = files(
    'clientinfo_unittest.cpp',
    'create_context_unittest.cpp',
    'enum_sizes.cpp',
    'fake_glx_screen.cpp',
    'fake_glx_screen.h',
    'indirect_api.cpp',
    'mock_xdisplay.h',
    'query_renderer_unittest.cpp',
  )
  if with_dri2
    files_glx_test += files('query_renderer_implementation_unittest.cpp')
  endif

  test(
    'dispatch-index-check',
    files('dispatch-index-check'),
    suite : ['glx'],
  )
  test(
    'glx-test',
    executable(
      'glx-test',
      [files_glx_test, glx_indirect_size_h, main_dispatch_h],
      link_with : [libglx, libglapi, libloader],
      include_directories : [
        inc_src, inc_include, inc_mesa, inc_mapi, inc_glx, inc_loader,
      ],
      dependencies : [dep_libdrm, dep_glproto, dep_thread, idep_gtest]
    ),
    suite : ['glx'],
    protocol : 'gtest',
  )
endif
