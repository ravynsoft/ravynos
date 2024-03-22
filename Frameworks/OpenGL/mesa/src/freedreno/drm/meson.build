# Copyright © 2018 Rob Clark

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

libfreedreno_drm_files = files(
  'freedreno_bo.c',
  'freedreno_bo_heap.c',
  'freedreno_bo_cache.c',
  'freedreno_device.c',
  'freedreno_drmif.h',
  'freedreno_pipe.c',
  'freedreno_priv.h',
  'freedreno_ringbuffer.c',
  'freedreno_ringbuffer.h',
  'freedreno_ringbuffer_sp.c',
  'freedreno_ringbuffer_sp.h',
  'freedreno_ringbuffer_sp_reloc.h',
)
libfreedreno_drm_flags = []
libfreedreno_drm_includes = [
  inc_freedreno,
  inc_include,
  inc_src,
]
libfreedreno_drm_deps = [
  dep_libdrm,
  dep_valgrind,
]

libfreedreno_drm_msm_files = files(
  'msm/msm_bo.c',
  'msm/msm_device.c',
  'msm/msm_pipe.c',
  'msm/msm_priv.h',
  'msm/msm_ringbuffer.c',
  'msm/msm_ringbuffer_sp.c',
)
libfreedreno_drm_files += libfreedreno_drm_msm_files

libfreedreno_drm_virtio_files = files(
  'virtio/virtio_bo.c',
  'virtio/virtio_device.c',
  'virtio/virtio_pipe.c',
  'virtio/virtio_priv.h',
  'virtio/virtio_ringbuffer.c',
)
if freedreno_kmds.contains('virtio')
  libfreedreno_drm_files += libfreedreno_drm_virtio_files
  libfreedreno_drm_flags += '-DHAVE_FREEDRENO_VIRTIO'
  libfreedreno_drm_includes += [
    inc_virtio_gpu,
    inc_virtio_vdrm,
  ]
endif

libfreedreno_drm = static_library(
  'freedreno_drm',
  [
    libfreedreno_drm_files,
    freedreno_xml_header_files,
  ],
  include_directories : libfreedreno_drm_includes,
  c_args : [no_override_init_args, libfreedreno_drm_flags],
  gnu_symbol_visibility : 'hidden',
  dependencies : libfreedreno_drm_deps,
  # TODO
  link_with : [libvdrm],
  build_by_default : false,
)

