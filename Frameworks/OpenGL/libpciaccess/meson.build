# Copyright © 2018-2020 Intel Corporation

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

project(
  'libpciaccess',
  ['c'],
  version : '0.18',
  license : 'MIT',
  meson_version : '>= 0.48.0',
  default_options : ['buildtype=debugoptimized'],
)

c = meson.get_compiler('c')

add_project_arguments(
  c.get_supported_arguments(
    '-Wpointer-arith',
    '-Wmissing-declarations',
    '-Wformat=2',
    '-Wstrict-prototypes',
    '-Wmissing-prototypes',
    '-Wnested-externs',
    '-Wbad-function-cast',
    '-Wold-style-definition',
    '-Wdeclaration-after-statement',
    '-Wunused',
    '-Wuninitialized',
    '-Wshadow',
    '-Wmissing-noreturn',
    '-Wmissing-format-attribute',
    '-Wredundant-decls',
    '-Wlogical-op',
    '-Werror=implicit',
    '-Werror=nonnull',
    '-Werror=init-self',
    '-Werror=main',
    '-Werror=missing-braces',
    '-Werror=sequence-point',
    '-Werror=return-type',
    '-Werror=trigraphs',
    '-Werror=array-bounds',
    '-Werror=write-strings',
    '-Werror=address',
    '-Werror=int-to-pointer-cast',
  ),
  language : 'c'
)

config = configuration_data()

config.set_quoted(
  'PCIIDS_PATH',
  join_paths(get_option('prefix'), get_option('datadir'), get_option('pci-ids'))
)
if get_option('linux-rom-fallback')
  config.set('LINUX_ROM', 1)
endif

dep_zlib = dependency('zlib', required : get_option('zlib'))
if dep_zlib.found()
  config.set('HAVE_ZLIB', 1)
endif

extra_libs = []
if host_machine.system() == 'netbsd'
  extra_libs += c.find_library('pci')
  if host_machine.cpu_family() == 'x86'
    extra_libs += c.find_library('i386')
  elif host_machine.cpu_family() == 'x86_64'
    extra_libs += c.find_library('x86_64')
  elif host_machine.cpu_family() == 'alpha'  # TODO
    extra_libs += c.find_library('alpha')
  endif
elif host_machine.system() == 'sunos'
  extra_libs += c.find_library('devinfo')
endif

if host_machine.system() == 'netbsd'
  _prefix = ''
  if c.check_header('machine/sysarch.h')
    _prefix = '''
      #include <sys/types.h>
      #include <machine/sysarch.h>
    '''
  endif

  if c.check_header('machine/mtrr.h', prefix : _prefix)
    config.set('HAVE_MTRR', 1)
  endif
elif c.check_header('asm/mtrr.h')
  config.set('HAVE_MTRR', 1)
endif

foreach h : ['err.h', 'stdint.h', 'string.h', 'strings.h', 'inttypes.h']
  if c.check_header(h)
    config.set('HAVE_' + h.to_upper().underscorify(), 1)
  endif
endforeach

config_h = configure_file(
  configuration : config,
  output : 'config.h',
)
add_project_arguments('-DHAVE_CONFIG_H', language : ['c'])

install_headers('include/pciaccess.h')
inc_include = include_directories('.', 'include')

subdir('src')
subdir('scanpci')
subdir('man')

pkg = import('pkgconfig')
pkg.generate(
  libpciaccess,
  description : 'Library providing generic access to the PCI bus and devices',
)
