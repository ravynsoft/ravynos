# Copyright © 2022 Yonggang Luo

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

inc_util_c11 = [include_directories('../..')]

files_mesa_util_c11 = files(
  'time.c',
)

if with_c11_threads
  # no need implement c11 threads
elif host_machine.system() == 'windows'
  files_mesa_util_c11 += 'threads_win32.c'
  files_mesa_util_c11 += 'threads_win32_tls_callback.cpp'
else
  files_mesa_util_c11 += 'threads_posix.c'
endif

_libmesa_util_c11 = static_library(
  'mesa_util_c11',
  [files_mesa_util_c11],
  include_directories : inc_util_c11,
  c_args : [c_msvc_compat_args],
  gnu_symbol_visibility : 'hidden',
  build_by_default : false
)

idep_mesautilc11 = declare_dependency(
  link_with : _libmesa_util_c11,
  include_directories : inc_util_c11,
  dependencies : [dep_clock, dep_thread, dep_atomic, dep_m, dep_valgrind],
)
