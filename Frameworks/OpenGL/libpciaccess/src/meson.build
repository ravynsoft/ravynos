# Copyright © 2020 Intel Corporation

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

host = host_machine.system()
_pci_access_host_files = []
if host == 'linux'
  _pci_access_host_files += ['linux_sysfs.c', 'linux_devmem.c', 'common_vgaarb.c']
elif host == 'freebsd'
  _pci_access_host_files += ['freebsd_pci.c', 'common_vgaarb_stub.c']
elif host == 'netbsd'
  _pci_access_host_files += ['netbsd_pci.c', 'common_vgaarb_stub.c']
elif host == 'openbsd'
  _pci_access_host_files += ['openbsd_pci.c']  # VGA arbiter code is in netbsd_pci.c
elif host == 'cygwin'
  _pci_access_host_files += ['x86_pci.c', 'common_vgaarb_stub.c']
elif host == 'sunos'
  _pci_access_host_files += ['solx_devfs.c', 'common_vgaarb_stub.c']
elif host == 'gnu'
  _pci_access_host_files += ['hurd_pci.c', 'x86_pci.c', 'common_vgaarb_stub.c']
endif

inc_src = include_directories('.')

libpciaccess = shared_library(
  'pciaccess',
  [
    'common_bridge.c',
    'common_iterator.c',
    'common_init.c',
    'common_interface.c',
    'common_io.c',
    'common_capability.c',
    'common_device_name.c',
    'common_map.c',
    _pci_access_host_files,
    config_h,
  ],
  include_directories : inc_include,
  dependencies : [dep_zlib, extra_libs],
  version : '0.11.1',
  install : true,
)

dep_pciaccess = declare_dependency(
  link_with : libpciaccess,
  include_directories : [include_directories('.'), inc_include]
)

if meson.version().version_compare('>= 0.54.0')
  meson.override_dependency('pciaccess', dep_pciaccess)
endif
