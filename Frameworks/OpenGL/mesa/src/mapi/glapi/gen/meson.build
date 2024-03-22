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

glapi_gen_gl_xml = files('../registry/gl.xml')
glapi_gen_mapi_deps = [
  glapi_gen_gl_xml,
  genCommon_py,
]

gl_and_es_api_files = files('gl_and_es_API.xml')

api_xml_files = files(
  'gl_API.xml',
  'es_EXT.xml',
  'gl_and_es_API.xml',
  'gl_and_glX_API.xml',
  'ARB_base_instance.xml',
  'ARB_blend_func_extended.xml',
  'ARB_bindless_texture.xml',
  'ARB_clear_buffer_object.xml',
  'ARB_clear_texture.xml',
  'ARB_clip_control.xml',
  'ARB_color_buffer_float.xml',
  'ARB_compressed_texture_pixel_storage.xml',
  'ARB_compute_shader.xml',
  'ARB_compute_variable_group_size.xml',
  'ARB_copy_buffer.xml',
  'ARB_copy_image.xml',
  'ARB_debug_output.xml',
  'ARB_depth_buffer_float.xml',
  'ARB_depth_clamp.xml',
  'ARB_direct_state_access.xml',
  'ARB_draw_buffers.xml',
  'ARB_draw_buffers_blend.xml',
  'ARB_draw_elements_base_vertex.xml',
  'ARB_draw_indirect.xml',
  'ARB_draw_instanced.xml',
  'ARB_ES2_compatibility.xml',
  'ARB_ES3_compatibility.xml',
  'ARB_framebuffer_no_attachments.xml',
  'ARB_framebuffer_object.xml',
  'ARB_get_program_binary.xml',
  'ARB_get_texture_sub_image.xml',
  'ARB_gl_spirv.xml',
  'ARB_gpu_shader_fp64.xml',
  'ARB_gpu_shader_int64.xml',
  'ARB_gpu_shader5.xml',
  'ARB_indirect_parameters.xml',
  'ARB_instanced_arrays.xml',
  'ARB_internalformat_query.xml',
  'ARB_internalformat_query2.xml',
  'ARB_invalidate_subdata.xml',
  'ARB_map_buffer_range.xml',
  'ARB_multi_bind.xml',
  'ARB_pipeline_statistics_query.xml',
  'ARB_program_interface_query.xml',
  'ARB_robustness.xml',
  'ARB_sample_shading.xml',
  'ARB_sampler_objects.xml',
  'ARB_seamless_cube_map.xml',
  'ARB_separate_shader_objects.xml',
  'ARB_shader_atomic_counters.xml',
  'ARB_shader_image_load_store.xml',
  'ARB_shader_subroutine.xml',
  'ARB_shader_storage_buffer_object.xml',
  'ARB_sparse_buffer.xml',
  'ARB_sparse_texture.xml',
  'ARB_spirv_extensions.xml',
  'ARB_sync.xml',
  'ARB_tessellation_shader.xml',
  'ARB_texture_barrier.xml',
  'ARB_texture_buffer_object.xml',
  'ARB_texture_buffer_range.xml',
  'ARB_texture_compression_rgtc.xml',
  'ARB_texture_cube_map_array.xml',
  'ARB_texture_float.xml',
  'ARB_texture_gather.xml',
  'ARB_texture_multisample.xml',
  'ARB_texture_rgb10_a2ui.xml',
  'ARB_texture_rg.xml',
  'ARB_texture_storage_multisample.xml',
  'ARB_texture_storage.xml',
  'ARB_texture_view.xml',
  'ARB_uniform_buffer_object.xml',
  'ARB_vertex_array_object.xml',
  'ARB_vertex_attrib_64bit.xml',
  'ARB_vertex_attrib_binding.xml',
  'ARB_viewport_array.xml',
  'AMD_depth_clamp_separate.xml',
  'AMD_draw_buffers_blend.xml',
  'AMD_gpu_shader_int64.xml',
  'AMD_performance_monitor.xml',
  'ARB_vertex_type_2_10_10_10_rev.xml',
  'APPLE_object_purgeable.xml',
  'APPLE_vertex_array_object.xml',
  'EXT_direct_state_access.xml',
  'EXT_draw_buffers2.xml',
  'EXT_external_objects.xml',
  'EXT_external_objects_fd.xml',
  'EXT_external_objects_win32.xml',
  'EXT_framebuffer_object.xml',
  'EXT_gpu_shader4.xml',
  'EXT_multisampled_render_to_texture.xml',
  'EXT_packed_depth_stencil.xml',
  'EXT_provoking_vertex.xml',
  'EXT_separate_shader_objects.xml',
  'EXT_shader_image_load_store.xml',
  'EXT_texture_array.xml',
  'EXT_texture_integer.xml',
  'EXT_transform_feedback.xml',
  'EXT_vertex_attrib_64bit.xml',
  'EXT_window_rectangles.xml',
  'GREMEDY_string_marker.xml',
  'INTEL_performance_query.xml',
  'KHR_debug.xml',
  'KHR_context_flush_control.xml',
  'KHR_robustness.xml',
  'KHR_robustness_es.xml',
  'KHR_texture_compression_astc.xml',
  'NV_alpha_to_coverage_dither_control.xml',
  'NV_copy_image.xml',
  'NV_conditional_render.xml',
  'NV_primitive_restart.xml',
  'NV_texture_barrier.xml',
  'NV_vdpau_interop.xml',
  'NV_viewport_swizzle.xml',
  'OES_EGL_image.xml',
  'OES_fixed_point.xml',
  'OES_single_precision.xml',
  'OES_texture_compression_astc.xml',
  'GL3x.xml',
  'GL4x.xml',
)

glapi_gen_depends = files(
  'gl_XML.py',
  'glX_XML.py',
  'license.py',
  'static_data.py',
  'typeexpr.py',
) + api_xml_files

glx_gen_depends = files(
  'glX_API.xml',
  'glX_XML.py',
  'glX_proto_common.py',
) + api_xml_files

glapi_mapi_tmp_h = custom_target(
  'glapi_mapi_tmp.h',
  input : [mapi_abi_py, 'gl_and_es_API.xml'],
  output : 'glapi_mapi_tmp.h',
  command : [prog_python, '@INPUT0@', '--printer', 'glapi', '@INPUT1@'],
  depend_files : glapi_gen_depends,
  capture : true,
)

glprocs_h = custom_target(
  'glprocs.h',
  input : ['gl_procs.py', 'gl_and_es_API.xml'],
  output : 'glprocs.h',
  command : [prog_python, '@INPUT0@', '-c', '-f', '@INPUT1@'],
  depend_files : glapi_gen_depends,
  capture : true,
)

glapitemp_h = custom_target(
  'glapitemp.h',
  input : ['gl_apitemp.py', 'gl_and_es_API.xml'],
  output : 'glapitemp.h',
  command : [prog_python, '@INPUT0@', '-f', '@INPUT1@'],
  depend_files : glapi_gen_depends,
  capture : true,
)

glapitable_h = custom_target(
  'glapitable.h',
  input : ['gl_table.py', 'gl_and_es_API.xml'],
  output : 'glapitable.h',
  command : [prog_python, '@INPUT0@', '-f', '@INPUT1@'],
  depend_files : glapi_gen_depends,
  capture : true,
)

glapi_gentable_c = custom_target(
  'glapi_gentable.c',
  input : ['gl_gentable.py', 'gl_and_es_API.xml'],
  output : 'glapi_gentable.c',
  command : [prog_python, '@INPUT0@', '-f', '@INPUT1@'],
  depend_files : glapi_gen_depends,
  capture : true,
)

main_enums_c = custom_target(
  'enums.c',
  input : ['gl_enums.py', files('../registry/gl.xml')],
  output : 'enums.c',
  command : [prog_python, '@INPUT0@', '-f', '@INPUT1@'],
  capture : true,
)

main_api_exec_c = custom_target(
  'api_exec_init.c',
  input : ['api_exec_init.py', 'gl_and_es_API.xml'],
  output : 'api_exec_init.c',
  command : [prog_python, '@INPUT0@', '-f', '@INPUT1@'],
  depend_files : files('apiexec.py') + glapi_gen_depends,
  capture : true,
)

main_api_exec_decl_h = custom_target(
  'api_exec_decl.h',
  input : ['api_exec_decl_h.py', 'gl_and_es_API.xml'],
  output : 'api_exec_decl.h',
  command : [prog_python, '@INPUT0@', '-f', '@INPUT1@'],
  depend_files : files('apiexec.py') + glapi_gen_depends,
  capture : true,
)

main_api_save_init_h = custom_target(
  'api_save_init.h',
  input : ['api_save_init_h.py', 'gl_and_es_API.xml'],
  output : 'api_save_init.h',
  command : [prog_python, '@INPUT0@', '-f', '@INPUT1@'],
  depend_files : files('apiexec.py') + glapi_gen_depends,
  capture : true,
)

main_api_save_h = custom_target(
  'api_save.h',
  input : ['api_save_h.py', 'gl_and_es_API.xml'],
  output : 'api_save.h',
  command : [prog_python, '@INPUT0@', '-f', '@INPUT1@'],
  depend_files : files('apiexec.py') + glapi_gen_depends,
  capture : true,
)

main_api_beginend_init_h = custom_target(
  'api_beginend_init.h',
  input : ['api_beginend_init_h.py', 'gl_and_es_API.xml'],
  output : 'api_beginend_init.h',
  command : [prog_python, '@INPUT0@', '-f', '@INPUT1@'],
  depend_files : files('apiexec.py') + glapi_gen_depends,
  capture : true,
)

main_api_hw_select_init_h = custom_target(
  'api_hw_select_init.h',
  input : ['api_hw_select_init_h.py', 'gl_API.xml'],
  output : 'api_hw_select_init.h',
  command : [prog_python, '@INPUT0@', '-f', '@INPUT1@'],
  depend_files : files('apiexec.py') + glapi_gen_depends,
  capture : true,
)

main_unmarshal_table_c = custom_target(
  'unmarshal_table.c',
  input : ['gl_unmarshal_table.py', 'gl_and_es_API.xml'],
  output : 'unmarshal_table.c',
  command : [prog_python, '@INPUT0@', '@INPUT1@'],
  depend_files : files('marshal_XML.py') + glapi_gen_depends,
  capture : true,
)

main_marshal_generated_c = []
foreach x : ['0', '1', '2', '3', '4', '5', '6', '7']
  main_marshal_generated_c += custom_target(
    'marshal_generated' + x + '.c',
    input : ['gl_marshal.py', 'gl_and_es_API.xml'],
    output : 'marshal_generated' + x + '.c',
    command : [prog_python, '@INPUT0@', '@INPUT1@', x, '8'],
    depend_files : files('marshal_XML.py') + glapi_gen_depends,
    capture : true,
  )
endforeach

glx_generated = []

foreach x : [['indirect.c', 'proto'], ['indirect.h', 'init_h'], ['indirect_init.c', 'init_c']]
  glx_generated += custom_target(
    x[0],
    input : ['glX_proto_send.py', 'gl_API.xml'],
    output : x[0],
    command : [prog_python, '@INPUT0@', '-f', '@INPUT1@', '-m', x[1]],
    depend_files : glx_gen_depends,
    capture : true,
  )
endforeach

foreach x : [['indirect_size.h', ['-m', 'size_h', '--header-tag', '_INDIRECT_SIZE_H_']],
             ['indirect_size.c', ['-m', 'size_c']]]
  glx_generated += custom_target(
    x[0],
    input : ['glX_proto_size.py', 'gl_API.xml'],
    output : x[0],
    command : [prog_python, '@INPUT0@', '-f', '@INPUT1@', '--only-set', x[1]],
    depend_files : glx_gen_depends,
    capture : true,
  )
endforeach
glx_indirect_size_h = glx_generated[3]

glapi_x86_s = custom_target(
  'glapi_x86.S',
  input : ['gl_x86_asm.py', gl_and_es_api_files],
  output : 'glapi_x86.S',
  command : [prog_python, '@INPUT0@', '-f', '@INPUT1@'],
  depend_files : glapi_gen_depends,
  capture : true,
)

glapi_x86_64_s = custom_target(
  'glapi_x86-64.S',
  input : ['gl_x86-64_asm.py', gl_and_es_api_files],
  output : 'glapi_x86-64.S',
  command : [prog_python, '@INPUT0@', '-f', '@INPUT1@'],
  depend_files : glapi_gen_depends,
  capture : true,
)

glapi_sparc_s = custom_target(
  'glapi_sparc.S',
  input : ['gl_SPARC_asm.py', gl_and_es_api_files],
  output : 'glapi_sparc.S',
  command : [prog_python, '@INPUT0@', '-f', '@INPUT1@'],
  depend_files : glapi_gen_depends,
  capture : true,
)
