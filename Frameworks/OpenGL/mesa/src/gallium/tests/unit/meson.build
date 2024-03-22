# Copyright © 2018 Intel Corporation

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

foreach t : ['pipe_barrier_test', 'u_cache_test', 'u_half_test',
             'translate_test', 'u_prim_verts_test']
  exe = executable(
    t,
    '@0@.c'.format(t),
    include_directories : [inc_include, inc_src, inc_mapi, inc_mesa, inc_gallium, inc_gallium_aux],
    link_with : libgallium,
    dependencies : idep_mesautil,
    install : false,
  )
  if (t == 'translate_test') # translate_test have parameters.
    # FIXME: translate_test default|generic are failing
    # test('translate_test default', exe, args : [ 'default' ])
    # test('translate_test generic', exe, args : [ 'generic' ])
    if ['x86', 'x86_64'].contains(host_machine.cpu_family())
      foreach arg : ['x86', 'nosse', 'sse', 'sse2', 'sse3', 'sse4.1']
        test('translate_test ' + arg, exe, args : [ arg ])
      endforeach
    endif
  elif t != 'u_cache_test' # u_cache_test is slow
    test(t, exe, suite: 'gallium',
         should_fail : meson.get_external_property('xfail', '').contains(t),
    )
  endif
endforeach
