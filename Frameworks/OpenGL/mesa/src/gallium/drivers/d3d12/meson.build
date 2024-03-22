# Copyright © Microsoft Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

files_libd3d12 = files(
  'd3d12_batch.cpp',
  'd3d12_blit.cpp',
  'd3d12_bufmgr.cpp',
  'd3d12_cmd_signature.cpp',
  'd3d12_compiler.cpp',
  'd3d12_compute_transforms.cpp',
  'd3d12_context.cpp',
  'd3d12_descriptor_pool.cpp',
  'd3d12_draw.cpp',
  'd3d12_fence.cpp',
  'd3d12_format.c',
  'd3d12_gs_variant.cpp',
  'd3d12_lower_image_casts.c',
  'd3d12_lower_point_sprite.c',
  'd3d12_nir_passes.c',
  'd3d12_pipeline_state.cpp',
  'd3d12_query.cpp',
  'd3d12_residency.cpp',
  'd3d12_resource.cpp',
  'd3d12_resource_state.cpp',
  'd3d12_root_signature.cpp',
  'd3d12_screen.cpp',
  'd3d12_surface.cpp',
  'd3d12_tcs_variant.cpp',
)

if with_gallium_d3d12_video
  files_libd3d12 += [
    'd3d12_video_dec.cpp',
    'd3d12_video_dec_references_mgr.cpp',
    'd3d12_video_buffer.cpp',
    'd3d12_video_enc.cpp',
    'd3d12_video_encoder_bitstream.cpp',
    'd3d12_video_texture_array_dpb_manager.cpp',
    'd3d12_video_array_of_textures_dpb_manager.cpp',
    'd3d12_video_screen.cpp',
    'd3d12_video_proc.cpp',
  ]

if _codecs.contains('h264dec')
  files_libd3d12 += [
    'd3d12_video_dec_h264.cpp',
  ]
endif

if _codecs.contains('h265dec')
  files_libd3d12 += [
    'd3d12_video_dec_hevc.cpp',
  ]
endif

if _codecs.contains('av1dec')
  files_libd3d12 += [
    'd3d12_video_dec_av1.cpp',
  ]
endif

if _codecs.contains('vp9dec')
  files_libd3d12 += [
    'd3d12_video_dec_vp9.cpp',
  ]
endif

if _codecs.contains('h264enc')
  files_libd3d12 += [
    'd3d12_video_enc_h264.cpp',
    'd3d12_video_encoder_references_manager_h264.cpp',
    'd3d12_video_encoder_nalu_writer_h264.cpp',
    'd3d12_video_encoder_bitstream_builder_h264.cpp',
  ]
endif

if _codecs.contains('h265enc')
  files_libd3d12 += [
    'd3d12_video_enc_hevc.cpp',
    'd3d12_video_encoder_bitstream_builder_hevc.cpp',
    'd3d12_video_encoder_nalu_writer_hevc.cpp',
    'd3d12_video_encoder_references_manager_hevc.cpp',
  ]
endif

if _codecs.contains('av1enc')
  files_libd3d12 += [
    'd3d12_video_enc_av1.cpp',
    'd3d12_video_encoder_references_manager_av1.cpp',
    'd3d12_video_encoder_bitstream_builder_av1.cpp',
  ]
endif

endif # with_gallium_d3d12_video

is_xbox = target_machine.system().startswith('Gaming.Xbox')
if host_machine.system() == 'windows'
  if is_xbox
    files_libd3d12 += files('d3d12_dxgi_xbox_screen.cpp')
  else
    files_libd3d12 += files('d3d12_dxgi_screen.cpp')
  endif
endif
if is_xbox == false
  files_libd3d12 += files('d3d12_dxcore_screen.cpp')
endif

libd3d12 = static_library(
  'd3d12',
  [files_libd3d12, sha1_h],
  gnu_symbol_visibility : 'hidden',
  include_directories : [inc_include, inc_src, inc_mesa, inc_gallium, inc_gallium_aux],
  dependencies: [idep_nir_headers, idep_libdxil_compiler, dep_dxheaders],
)

driver_d3d12 = declare_dependency(
  compile_args : '-DGALLIUM_D3D12',
  link_with : [libd3d12, libgalliumvl],
)
