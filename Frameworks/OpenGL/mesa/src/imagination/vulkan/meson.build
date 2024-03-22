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

subdir('pds')

pvr_entrypoints = custom_target(
  'pvr_entrypoints',
  input : [vk_entrypoints_gen, vk_api_xml],
  output : ['pvr_entrypoints.h', 'pvr_entrypoints.c'],
  command : [
    prog_python, '@INPUT0@', '--xml', '@INPUT1@', '--proto', '--weak',
    '--out-h', '@OUTPUT0@', '--out-c', '@OUTPUT1@', '--prefix', 'pvr',
    '--beta', with_vulkan_beta.to_string()
  ],
  depend_files : vk_entrypoints_gen_depend_files,
)

pvr_files = files(
  'winsys/powervr/pvr_drm.c',
  'winsys/powervr/pvr_drm_bo.c',
  'winsys/powervr/pvr_drm_job_compute.c',
  'winsys/powervr/pvr_drm_job_null.c',
  'winsys/powervr/pvr_drm_job_render.c',
  'winsys/powervr/pvr_drm_job_transfer.c',
  'winsys/pvr_winsys.c',
  'winsys/pvr_winsys_helper.c',
  'pvr_blit.c',
  'pvr_bo.c',
  'pvr_border.c',
  'pvr_clear.c',
  'pvr_cmd_buffer.c',
  'pvr_csb.c',
  'pvr_descriptor_set.c',
  'pvr_device.c',
  'pvr_dump_bo.c',
  'pvr_dump_csb.c',
  'pvr_formats.c',
  'pvr_hardcode.c',
  'pvr_hw_pass.c',
  'pvr_image.c',
  'pvr_job_common.c',
  'pvr_job_compute.c',
  'pvr_job_context.c',
  'pvr_job_render.c',
  'pvr_job_transfer.c',
  'pvr_pass.c',
  'pvr_pipeline.c',
  'pvr_transfer_frag_store.c',
  'pvr_query.c',
  'pvr_query_compute.c',
  'pvr_queue.c',
  'pvr_robustness.c',
  'pvr_shader.c',
  'pvr_spm.c',
  'pvr_tex_state.c',
  'pvr_wsi.c',

  'usc/pvr_uscgen.c',
  'usc/pvr_uscgen_tq.c',
)

pvr_includes = [
  include_directories('usc/programs'),
  include_directories('usc'),
  include_directories('winsys'),
  libpowervr_pds_includes,
]

pvr_deps = [
  dep_csbgen,
  dep_libdrm,
  dep_valgrind,
  idep_mesautil,
  idep_vulkan_runtime,
  idep_vulkan_util,
  idep_vulkan_wsi,
]

pvr_flags = [
  imagination_c_args,
  no_override_init_args,
]

if with_imagination_srv
  pvr_files += files(
    'winsys/pvrsrvkm/pvr_srv.c',
    'winsys/pvrsrvkm/pvr_srv_bo.c',
    'winsys/pvrsrvkm/pvr_srv_bridge.c',
    'winsys/pvrsrvkm/pvr_srv_job_common.c',
    'winsys/pvrsrvkm/pvr_srv_job_compute.c',
    'winsys/pvrsrvkm/pvr_srv_job_null.c',
    'winsys/pvrsrvkm/pvr_srv_job_render.c',
    'winsys/pvrsrvkm/pvr_srv_job_transfer.c',
    'winsys/pvrsrvkm/pvr_srv_sync.c',
    'winsys/pvrsrvkm/pvr_srv_sync_prim.c',
  )
  pvr_flags += '-DPVR_SUPPORT_SERVICES_DRIVER'
endif

libvulkan_powervr_mesa = shared_library(
  'vulkan_powervr_mesa',
  [pvr_files, pvr_entrypoints, u_format_pack_h],
  include_directories : [
    pvr_includes,
    inc_imagination,
    inc_include,
    inc_src,
  ],
  link_with : [
    libpowervr_common,
    libpowervr_pds,
    libpowervr_rogue,
    libvulkan_wsi,
  ],
  dependencies : [
    pvr_deps,
    idep_nir,
  ],
  c_args : pvr_flags,
  link_args : [
    ld_args_build_id,
    ld_args_bsymbolic,
    ld_args_gc_sections
  ],
  gnu_symbol_visibility : 'hidden',
  install : true,
)

if with_symbols_check
  test(
    'pvr symbols check',
    symbols_check,
    args : [
      '--lib', libvulkan_powervr_mesa,
      '--symbols-file', vulkan_icd_symbols,
      symbols_check_args,
    ],
    suite : ['imagination'],
  )
endif

powervr_mesa_icd = custom_target(
  'powervr_mesa_icd',
  input : [vk_icd_gen, vk_api_xml],
  output : 'powervr_mesa_icd.@0@.json'.format(host_machine.cpu()),
  command : [
    prog_python, '@INPUT0@',
    '--api-version', '1.0', '--xml', '@INPUT1@',
    '--lib-path', get_option('prefix') / get_option('libdir') / 'libvulkan_powervr_mesa.so',
    '--out', '@OUTPUT@',
  ],
  build_by_default : true,
  install_dir : with_vulkan_icd_dir,
  install_tag : 'runtime',
  install : true,
)

_dev_icd = custom_target(
  'powervr_mesa_devenv_icd',
  input : [vk_icd_gen, vk_api_xml],
  output : 'powervr_mesa_devenv_icd.@0@.json'.format(host_machine.cpu()),
  command : [
    prog_python, '@INPUT0@',
    '--api-version', '1.0', '--xml', '@INPUT1@',
    '--lib-path', meson.current_build_dir() / 'libvulkan_powervr_mesa.so',
    '--out', '@OUTPUT@',
  ],
  build_by_default : true,
)

devenv.append('VK_ICD_FILENAMES', _dev_icd.full_path())
