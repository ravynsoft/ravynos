# Copyright © 2017 Jon Turney

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

# protocol defines for the Windows-DRI server extension

files_windowsdriproto = files(
 'windowsdriconst.h',
 'windowsdristr.h',
 )

install_headers(
  files_windowsdriproto,
  subdir: 'X11/extensions',
)

pkg.generate(
  name : 'windowsdriproto',
  description : 'Windows-DRI extension headers',
  version : '1.0.0',
)

# library for using the Windows-DRI server extension
files_libwindowsdri = files(
  'xwindowsdri.c',
  'xwindowsdri.h',
)

libwindowsdri = static_library(
  'driwindows',
  [files_libwindowsdri, files_windowsdriproto],
  dependencies: dep_xext,
  build_by_default: false,
)

# library for native GL on windows
files_libwindowsglx = files(
  'windowsgl.c',
  'windowsgl.h',
  'windowsgl_internal.h',
  'windows_drawable.c',
  'wgl.c',
  'wgl.h',
)

libwindowsglx = static_library(
  'glxwindows',
  [files_libwindowsglx, files_windowsdriproto],
  include_directories: [inc_include, inc_src, inc_glapi],
  gnu_symbol_visibility : 'hidden',
  build_by_default: false,
)
