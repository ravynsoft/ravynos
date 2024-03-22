.. _screen:

Screen
======

A screen is an object representing the context-independent part of a device.

Flags and enumerations
----------------------

XXX some of these don't belong in this section.


.. _pipe_cap:

PIPE_CAP_*
^^^^^^^^^^

Capability queries return information about the features and limits of the
driver/GPU.  For floating-point values, use :ref:`get_paramf`, and for boolean
or integer values, use :ref:`get_param`.

The integer capabilities:

* ``PIPE_CAP_GRAPHICS``: Whether graphics is supported. If not, contexts can
  only be created with PIPE_CONTEXT_COMPUTE_ONLY.
* ``PIPE_CAP_NPOT_TEXTURES``: Whether :term:`NPOT` textures may have repeat modes,
  normalized coordinates, and mipmaps.
* ``PIPE_CAP_MAX_DUAL_SOURCE_RENDER_TARGETS``: How many dual-source blend RTs are support.
  :ref:`Blend` for more information.
* ``PIPE_CAP_ANISOTROPIC_FILTER``: Whether textures can be filtered anisotropically.
* ``PIPE_CAP_MAX_RENDER_TARGETS``: The maximum number of render targets that may be
  bound.
* ``PIPE_CAP_OCCLUSION_QUERY``: Whether occlusion queries are available.
* ``PIPE_CAP_QUERY_TIME_ELAPSED``: Whether PIPE_QUERY_TIME_ELAPSED queries are available.
* ``PIPE_CAP_TEXTURE_SHADOW_MAP``: indicates whether the fragment shader hardware
  can do the depth texture / Z comparison operation in TEX instructions
  for shadow testing.
* ``PIPE_CAP_TEXTURE_SWIZZLE``: Whether swizzling through sampler views is
  supported.
* ``PIPE_CAP_MAX_TEXTURE_2D_SIZE``: The maximum size of 2D (and 1D) textures.
* ``PIPE_CAP_MAX_TEXTURE_3D_LEVELS``: The maximum number of mipmap levels available
  for a 3D texture.
* ``PIPE_CAP_MAX_TEXTURE_CUBE_LEVELS``: The maximum number of mipmap levels available
  for a cubemap.
* ``PIPE_CAP_TEXTURE_MIRROR_CLAMP_TO_EDGE``: Whether mirrored texture coordinates are
  supported with the clamp-to-edge wrap mode.
* ``PIPE_CAP_TEXTURE_MIRROR_CLAMP``: Whether mirrored texture coordinates are supported
  with clamp or clamp-to-border wrap modes.
* ``PIPE_CAP_BLEND_EQUATION_SEPARATE``: Whether alpha blend equations may be different
  from color blend equations, in :ref:`Blend` state.
* ``PIPE_CAP_MAX_STREAM_OUTPUT_BUFFERS``: The maximum number of stream buffers.
* ``PIPE_CAP_PRIMITIVE_RESTART``: Whether primitive restart is supported.
* ``PIPE_CAP_PRIMITIVE_RESTART_FIXED_INDEX``: Subset of
  PRIMITIVE_RESTART where the restart index is always the fixed maximum
  value for the index type.
* ``PIPE_CAP_INDEP_BLEND_ENABLE``: Whether per-rendertarget blend enabling and channel
  masks are supported. If 0, then the first rendertarget's blend mask is
  replicated across all MRTs.
* ``PIPE_CAP_INDEP_BLEND_FUNC``: Whether per-rendertarget blend functions are
  available. If 0, then the first rendertarget's blend functions affect all
  MRTs.
* ``PIPE_CAP_MAX_TEXTURE_ARRAY_LAYERS``: The maximum number of texture array
  layers supported. If 0, the array textures are not supported at all and
  the ARRAY texture targets are invalid.
* ``PIPE_CAP_FS_COORD_ORIGIN_UPPER_LEFT``: Whether the upper-left origin
  fragment convention is supported.
* ``PIPE_CAP_FS_COORD_ORIGIN_LOWER_LEFT``: Whether the lower-left origin
  fragment convention is supported.
* ``PIPE_CAP_FS_COORD_PIXEL_CENTER_HALF_INTEGER``: Whether the half-integer
  pixel-center fragment convention is supported.
* ``PIPE_CAP_FS_COORD_PIXEL_CENTER_INTEGER``: Whether the integer
  pixel-center fragment convention is supported.
* ``PIPE_CAP_DEPTH_CLIP_DISABLE``: Whether the driver is capable of disabling
  depth clipping (through pipe_rasterizer_state).
* ``PIPE_CAP_DEPTH_CLIP_DISABLE_SEPARATE``: Whether the driver is capable of
  disabling depth clipping (through pipe_rasterizer_state) separately for
  the near and far plane. If not, depth_clip_near and depth_clip_far will be
  equal.
  ``PIPE_CAP_DEPTH_CLAMP_ENABLE``: Whether the driver is capable of
  enabling depth clamping (through pipe_rasterizer_state) separately from depth
  clipping. If not, depth_clamp will be the inverse of depth_clip_far.
* ``PIPE_CAP_SHADER_STENCIL_EXPORT``: Whether a stencil reference value can be
  written from a fragment shader.
* ``PIPE_CAP_VS_INSTANCEID``: Whether ``SYSTEM_VALUE_INSTANCE_ID`` is
  supported in the vertex shader.
* ``PIPE_CAP_VERTEX_ELEMENT_INSTANCE_DIVISOR``: Whether the driver supports
  per-instance vertex attribs.
* ``PIPE_CAP_FRAGMENT_COLOR_CLAMPED``: Whether fragment color clamping is
  supported.  That is, is the pipe_rasterizer_state::clamp_fragment_color
  flag supported by the driver?  If not, gallium frontends will insert
  clamping code into the fragment shaders when needed.

* ``PIPE_CAP_MIXED_COLORBUFFER_FORMATS``: Whether mixed colorbuffer formats are
  supported, e.g. RGBA8 and RGBA32F as the first and second colorbuffer, resp.
* ``PIPE_CAP_VERTEX_COLOR_UNCLAMPED``: Whether the driver is capable of
  outputting unclamped vertex colors from a vertex shader. If unsupported,
  the vertex colors are always clamped. This is the default for DX9 hardware.
* ``PIPE_CAP_VERTEX_COLOR_CLAMPED``: Whether the driver is capable of
  clamping vertex colors when they come out of a vertex shader, as specified
  by the pipe_rasterizer_state::clamp_vertex_color flag.  If unsupported,
  the vertex colors are never clamped. This is the default for DX10 hardware.
  If both clamped and unclamped CAPs are supported, the clamping can be
  controlled through pipe_rasterizer_state.  If the driver cannot do vertex
  color clamping, gallium frontends may insert clamping code into the vertex
  shader.
* ``PIPE_CAP_GLSL_FEATURE_LEVEL``: Whether the driver supports features
  equivalent to a specific GLSL version. E.g. for GLSL 1.3, report 130.
* ``PIPE_CAP_GLSL_FEATURE_LEVEL_COMPATIBILITY``: Whether the driver supports
  features equivalent to a specific GLSL version including all legacy OpenGL
  features only present in the OpenGL compatibility profile.
  The only legacy features that Gallium drivers must implement are
  the legacy shader inputs and outputs (colors, texcoords, fog, clipvertex,
  edgeflag).
* ``PIPE_CAP_ESSL_FEATURE_LEVEL``: An optional cap to allow drivers to
  report a higher GLSL version for GLES contexts.  This is useful when a
  driver does not support all the required features for a higher GL version,
  but does support the required features for a higher GLES version.  A driver
  is allowed to return ``0`` in which case ``PIPE_CAP_GLSL_FEATURE_LEVEL`` is
  used.
  Note that simply returning the same value as the GLSL feature level cap is
  incorrect.  For example, GLSL version 3.30 does not require
  :ext:`GL_EXT_gpu_shader5`, but ESSL version 3.20 es does require
  :ext:`GL_EXT_gpu_shader5`
* ``PIPE_CAP_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION``: Whether quads adhere to
  the flatshade_first setting in ``pipe_rasterizer_state``.
* ``PIPE_CAP_USER_VERTEX_BUFFERS``: Whether the driver supports user vertex
  buffers.  If not, gallium frontends must upload all data which is not in HW
  resources.  If user-space buffers are supported, the driver must also still
  accept HW resource buffers.
* ``PIPE_CAP_VERTEX_BUFFER_OFFSET_4BYTE_ALIGNED_ONLY``: This CAP describes a HW
  limitation.  If true, pipe_vertex_buffer::buffer_offset must always be aligned
  to 4.  If false, there are no restrictions on the offset.
* ``PIPE_CAP_VERTEX_BUFFER_STRIDE_4BYTE_ALIGNED_ONLY``: This CAP describes a HW
  limitation.  If true, pipe_vertex_buffer::stride must always be aligned to 4.
  If false, there are no restrictions on the stride.
* ``PIPE_CAP_VERTEX_ELEMENT_SRC_OFFSET_4BYTE_ALIGNED_ONLY``: This CAP describes
  a HW limitation.  If true, pipe_vertex_element::src_offset must always be
  aligned to 4.  If false, there are no restrictions on src_offset.
* ``PIPE_CAP_VERTEX_ATTRIB_ELEMENT_ALIGNED_ONLY``: This CAP describes
  a HW limitation.  If true, the sum of
  ``pipe_vertex_element::src_offset + pipe_vertex_buffer::buffer_offset + pipe_vertex_buffer::stride``
  must always be aligned to the component size for the vertex attributes
  which access that buffer.  If false, there are no restrictions on these values.
  This CAP cannot be used with any other alignment-requiring CAPs.
* ``PIPE_CAP_COMPUTE``: Whether the implementation supports the
  compute entry points defined in pipe_context and pipe_screen.
* ``PIPE_CAP_CONSTANT_BUFFER_OFFSET_ALIGNMENT``: Describes the required
  alignment of pipe_constant_buffer::buffer_offset.
* ``PIPE_CAP_START_INSTANCE``: Whether the driver supports
  pipe_draw_info::start_instance.
* ``PIPE_CAP_QUERY_TIMESTAMP``: Whether PIPE_QUERY_TIMESTAMP and
  the pipe_screen::get_timestamp hook are implemented.
* ``PIPE_CAP_QUERY_TIMESTAMP_BITS``: How many bits the driver uses for the
  results of GL_TIMESTAMP queries.
* ``PIPE_CAP_TIMER_RESOLUTION``: The resolution of the timer in nanos.
* ``PIPE_CAP_TEXTURE_MULTISAMPLE``: Whether all MSAA resources supported
  for rendering are also supported for texturing.
* ``PIPE_CAP_MIN_MAP_BUFFER_ALIGNMENT``: The minimum alignment that should be
  expected for a pointer returned by transfer_map if the resource is
  PIPE_BUFFER. In other words, the pointer returned by transfer_map is
  always aligned to this value.
* ``PIPE_CAP_TEXTURE_BUFFER_OFFSET_ALIGNMENT``: Describes the required
  alignment for pipe_sampler_view::u.buf.offset, in bytes.
  If a driver does not support offset/size, it should return 0.
* ``PIPE_CAP_LINEAR_IMAGE_PITCH_ALIGNMENT``: Describes the row pitch alignment
  size that pipe_sampler_view::u.tex2d_from_buf must be multiple of, in pixels.
  If a driver does not support images created from buffers, it should return 0.
* ``PIPE_CAP_LINEAR_IMAGE_BASE_ADDRESS_ALIGNMENT``: Describes the minimum alignment
  in pixels of the offset of a host pointer for images created from buffers.
  If a driver does not support images created from buffers, it should return 0.
* ``PIPE_CAP_BUFFER_SAMPLER_VIEW_RGBA_ONLY``: Whether the driver only
  supports R, RG, RGB and RGBA formats for PIPE_BUFFER sampler views.
  When this is the case it should be assumed that the swizzle parameters
  in the sampler view have no effect.
* ``PIPE_CAP_TGSI_TEXCOORD``: This CAP describes a HW limitation.
  If true, the hardware cannot replace arbitrary shader inputs with sprite
  coordinates and hence the inputs that are desired to be replaceable must
  be declared with TGSI_SEMANTIC_TEXCOORD instead of TGSI_SEMANTIC_GENERIC.
  The rasterizer's sprite_coord_enable state therefore also applies to the
  TEXCOORD semantic.
  Also, TGSI_SEMANTIC_PCOORD becomes available, which labels a fragment shader
  input that will always be replaced with sprite coordinates.
* ``PIPE_CAP_TEXTURE_TRANSFER_MODES``: The ``pipe_texture_transfer_mode`` modes
  that are supported for implementing a texture transfer which needs format conversions
  and swizzling in gallium frontends. Generally, all hardware drivers with
  dedicated memory should return PIPE_TEXTURE_TRANSFER_BLIT and all software rasterizers
  should return PIPE_TEXTURE_TRANSFER_DEFAULT. PIPE_TEXTURE_TRANSFER_COMPUTE requires drivers
  to support 8bit and 16bit shader storage buffer writes and to implement
  pipe_screen::is_compute_copy_faster.
* ``PIPE_CAP_QUERY_PIPELINE_STATISTICS``: Whether PIPE_QUERY_PIPELINE_STATISTICS
  is supported.
* ``PIPE_CAP_TEXTURE_BORDER_COLOR_QUIRK``: Bitmask indicating whether special
  considerations have to be given to the interaction between the border color
  in the sampler object and the sampler view used with it.
  If PIPE_QUIRK_TEXTURE_BORDER_COLOR_SWIZZLE_R600 is set, the border color
  may be affected in undefined ways for any kind of permutational swizzle
  (any swizzle XYZW where X/Y/Z/W are not ZERO, ONE, or R/G/B/A respectively)
  in the sampler view.
  If PIPE_QUIRK_TEXTURE_BORDER_COLOR_SWIZZLE_NV50 is set, the border color
  state should be swizzled manually according to the swizzle in the sampler
  view it is intended to be used with, or herein undefined results may occur
  for permutational swizzles.
* ``PIPE_CAP_MAX_TEXEL_BUFFER_ELEMENTS_UINT``: The maximum accessible number of
  elements within a sampler buffer view and image buffer view. This is unsigned
  integer with the maximum of 4G - 1.
* ``PIPE_CAP_MAX_VIEWPORTS``: The maximum number of viewports (and scissors
  since they are linked) a driver can support. Returning 0 is equivalent
  to returning 1 because every driver has to support at least a single
  viewport/scissor combination.
* ``PIPE_CAP_ENDIANNESS``:: The endianness of the device.  Either
  PIPE_ENDIAN_BIG or PIPE_ENDIAN_LITTLE.
* ``PIPE_CAP_MIXED_FRAMEBUFFER_SIZES``: Whether it is allowed to have
  different sizes for fb color/zs attachments. This controls whether
  :ext:`GL_ARB_framebuffer_object` is provided.
* ``PIPE_CAP_VS_LAYER_VIEWPORT``: Whether ``VARYING_SLOT_LAYER`` and
  ``VARYING_SLOT_VIEWPORT`` are supported as vertex shader outputs. Note that
  the viewport will only be used if multiple viewports are exposed.
* ``PIPE_CAP_MAX_GEOMETRY_OUTPUT_VERTICES``: The maximum number of vertices
  output by a single invocation of a geometry shader.
* ``PIPE_CAP_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS``: The maximum number of
  vertex components output by a single invocation of a geometry shader.
  This is the product of the number of attribute components per vertex and
  the number of output vertices.
* ``PIPE_CAP_MAX_TEXTURE_GATHER_COMPONENTS``: Max number of components
  in format that texture gather can operate on. 1 == RED, ALPHA etc,
  4 == All formats.
* ``PIPE_CAP_TEXTURE_GATHER_SM5``: Whether the texture gather
  hardware implements the SM5 features, component selection,
  shadow comparison, and run-time offsets.
* ``PIPE_CAP_BUFFER_MAP_PERSISTENT_COHERENT``: Whether
  PIPE_MAP_PERSISTENT and PIPE_MAP_COHERENT are supported
  for buffers.
* ``PIPE_CAP_TEXTURE_QUERY_LOD``: Whether the ``LODQ`` instruction is
  supported.
* ``PIPE_CAP_MIN_TEXTURE_GATHER_OFFSET``: The minimum offset that can be used
  in conjunction with a texture gather opcode.
* ``PIPE_CAP_MAX_TEXTURE_GATHER_OFFSET``: The maximum offset that can be used
  in conjunction with a texture gather opcode.
* ``PIPE_CAP_SAMPLE_SHADING``: Whether there is support for per-sample
  shading. The context->set_min_samples function will be expected to be
  implemented.
* ``PIPE_CAP_TEXTURE_GATHER_OFFSETS``: Whether the ``TG4`` instruction can
  accept 4 offsets.
* ``PIPE_CAP_VS_WINDOW_SPACE_POSITION``: Whether window-space position is
  supported, which disables clipping and viewport transformation.
* ``PIPE_CAP_MAX_VERTEX_STREAMS``: The maximum number of vertex streams
  supported by the geometry shader. If stream-out is supported, this should be
  at least 1. If stream-out is not supported, this should be 0.
* ``PIPE_CAP_DRAW_INDIRECT``: Whether the driver supports taking draw arguments
  { count, instance_count, start, index_bias } from a PIPE_BUFFER resource.
  See pipe_draw_info.
* ``PIPE_CAP_MULTI_DRAW_INDIRECT``: Whether the driver supports
  pipe_draw_info::indirect_stride and ::indirect_count
* ``PIPE_CAP_MULTI_DRAW_INDIRECT_PARAMS``: Whether the driver supports
  taking the number of indirect draws from a separate parameter
  buffer, see pipe_draw_indirect_info::indirect_draw_count.
* ``PIPE_CAP_MULTI_DRAW_INDIRECT_PARTIAL_STRIDE``: Whether the driver supports
  indirect draws with an arbitrary stride.
* ``PIPE_CAP_FS_FINE_DERIVATIVE``: Whether the fragment shader supports
  the FINE versions of DDX/DDY.
* ``PIPE_CAP_VENDOR_ID``: The vendor ID of the underlying hardware. If it's
  not available one should return 0xFFFFFFFF.
* ``PIPE_CAP_DEVICE_ID``: The device ID (PCI ID) of the underlying hardware.
  0xFFFFFFFF if not available.
* ``PIPE_CAP_ACCELERATED``: Whether the renderer is hardware accelerated. 0 means
  not accelerated (i.e. CPU rendering), 1 means accelerated (i.e. GPU rendering),
  -1 means unknown (i.e. an API translation driver which doesn't known what kind of
  hardware it's running above).
* ``PIPE_CAP_VIDEO_MEMORY``: The amount of video memory in megabytes.
* ``PIPE_CAP_UMA``: If the device has a unified memory architecture or on-card
  memory and GART.
* ``PIPE_CAP_CONDITIONAL_RENDER_INVERTED``: Whether the driver supports inverted
  condition for conditional rendering.
* ``PIPE_CAP_MAX_VERTEX_ATTRIB_STRIDE``: The maximum supported vertex stride.
* ``PIPE_CAP_SAMPLER_VIEW_TARGET``: Whether the sampler view's target can be
  different than the underlying resource's, as permitted by
  :ext:`GL_ARB_texture_view`. For example a 2d array texture may be reinterpreted as a
  cube (array) texture and vice-versa.
* ``PIPE_CAP_CLIP_HALFZ``: Whether the driver supports the
  pipe_rasterizer_state::clip_halfz being set to true. This is required
  for enabling :ext:`GL_ARB_clip_control`.
* ``PIPE_CAP_POLYGON_OFFSET_CLAMP``: If true, the driver implements support
  for ``pipe_rasterizer_state::offset_clamp``.
* ``PIPE_CAP_MULTISAMPLE_Z_RESOLVE``: Whether the driver supports blitting
  a multisampled depth buffer into a single-sampled texture (or depth buffer).
  Only the first sampled should be copied.
* ``PIPE_CAP_RESOURCE_FROM_USER_MEMORY``: Whether the driver can create
  a pipe_resource where an already-existing piece of (malloc'd) user memory
  is used as its backing storage. In other words, whether the driver can map
  existing user memory into the device address space for direct device access.
  The create function is pipe_screen::resource_from_user_memory. The address
  and size must be page-aligned.
* ``PIPE_CAP_RESOURCE_FROM_USER_MEMORY_COMPUTE_ONLY``: Same as
  ``PIPE_CAP_RESOURCE_FROM_USER_MEMORY`` but indicates it is only supported from
  the compute engines.
* ``PIPE_CAP_DEVICE_RESET_STATUS_QUERY``:
  Whether pipe_context::get_device_reset_status is implemented.
* ``PIPE_CAP_MAX_SHADER_PATCH_VARYINGS``:
  How many per-patch outputs and inputs are supported between tessellation
  control and tessellation evaluation shaders, not counting in TESSINNER and
  TESSOUTER. The minimum allowed value for OpenGL is 30.
* ``PIPE_CAP_TEXTURE_FLOAT_LINEAR``: Whether the linear minification and
  magnification filters are supported with single-precision floating-point
  textures.
* ``PIPE_CAP_TEXTURE_HALF_FLOAT_LINEAR``: Whether the linear minification and
  magnification filters are supported with half-precision floating-point
  textures.
* ``PIPE_CAP_DEPTH_BOUNDS_TEST``: Whether bounds_test, bounds_min, and
  bounds_max states of pipe_depth_stencil_alpha_state behave according
  to the :ext:`GL_EXT_depth_bounds_test` specification.
* ``PIPE_CAP_TEXTURE_QUERY_SAMPLES``: Whether the ``TXQS`` opcode is supported
* ``PIPE_CAP_FORCE_PERSAMPLE_INTERP``: If the driver can force per-sample
  interpolation for all fragment shader inputs if
  pipe_rasterizer_state::force_persample_interp is set. This is only used
  by GL3-level sample shading (:ext:`GL_ARB_sample_shading`). GL4-level sample
  shading (:ext:`GL_ARB_gpu_shader5`) doesn't use this. While GL3 hardware has a
  state for it, GL4 hardware will likely need to emulate it with a shader
  variant, or by selecting the interpolation weights with a conditional
  assignment in the shader.
* ``PIPE_CAP_SHAREABLE_SHADERS``: Whether shader CSOs can be used by any
  pipe_context.  Important for reducing jank at draw time by letting GL shaders
  linked in one thread be used in another thread without recompiling.
* ``PIPE_CAP_COPY_BETWEEN_COMPRESSED_AND_PLAIN_FORMATS``:
  Whether copying between compressed and plain formats is supported where
  a compressed block is copied to/from a plain pixel of the same size.
* ``PIPE_CAP_CLEAR_SCISSORED``: Whether ``clear`` can accept a scissored
  bounding box.
* ``PIPE_CAP_DRAW_PARAMETERS``: Whether ``TGSI_SEMANTIC_BASEVERTEX``,
  ``TGSI_SEMANTIC_BASEINSTANCE``, and ``TGSI_SEMANTIC_DRAWID`` are
  supported in vertex shaders.
* ``PIPE_CAP_SHADER_PACK_HALF_FLOAT``: Whether packed 16-bit float
  packing/unpacking opcodes are supported.
* ``PIPE_CAP_FS_POSITION_IS_SYSVAL``: If gallium frontends should use a
  system value for the POSITION fragment shader input.
* ``PIPE_CAP_FS_POINT_IS_SYSVAL``: If gallium frontends should use a system
  value for the POINT fragment shader input.
* ``PIPE_CAP_FS_FACE_IS_INTEGER_SYSVAL``: If gallium frontends should use
  a system value for the FACE fragment shader input.
  Also, the FACE system value is integer, not float.
* ``PIPE_CAP_SHADER_BUFFER_OFFSET_ALIGNMENT``: Describes the required
  alignment for pipe_shader_buffer::buffer_offset, in bytes. Maximum
  value allowed is 256 (for GL conformance). 0 is only allowed if
  shader buffers are not supported.
* ``PIPE_CAP_INVALIDATE_BUFFER``: Whether the use of ``invalidate_resource``
  for buffers is supported.
* ``PIPE_CAP_GENERATE_MIPMAP``: Indicates whether pipe_context::generate_mipmap
  is supported.
* ``PIPE_CAP_STRING_MARKER``: Whether pipe->emit_string_marker() is supported.
* ``PIPE_CAP_SURFACE_REINTERPRET_BLOCKS``: Indicates whether
  pipe_context::create_surface supports reinterpreting a texture as a surface
  of a format with different block width/height (but same block size in bits).
  For example, a compressed texture image can be interpreted as a
  non-compressed surface whose texels are the same number of bits as the
  compressed blocks, and vice versa. The width and height of the surface is
  adjusted appropriately.
* ``PIPE_CAP_QUERY_BUFFER_OBJECT``: Driver supports
  context::get_query_result_resource callback.
* ``PIPE_CAP_PCI_GROUP``: Return the PCI segment group number.
* ``PIPE_CAP_PCI_BUS``: Return the PCI bus number.
* ``PIPE_CAP_PCI_DEVICE``: Return the PCI device number.
* ``PIPE_CAP_PCI_FUNCTION``: Return the PCI function number.
* ``PIPE_CAP_FRAMEBUFFER_NO_ATTACHMENT``:
  If non-zero, rendering to framebuffers with no surface attachments
  is supported. The context->is_format_supported function will be expected
  to be implemented with PIPE_FORMAT_NONE yielding the MSAA modes the hardware
  supports. N.B., The maximum number of layers supported for rasterizing a
  primitive on a layer is obtained from ``PIPE_CAP_MAX_TEXTURE_ARRAY_LAYERS``
  even though it can be larger than the number of layers supported by either
  rendering or textures.
* ``PIPE_CAP_ROBUST_BUFFER_ACCESS_BEHAVIOR``: Implementation uses bounds
  checking on resource accesses by shader if the context is created with
  PIPE_CONTEXT_ROBUST_BUFFER_ACCESS. See the
  :ext:`GL_ARB_robust_buffer_access_behavior` extension for information on the
  required behavior for out of bounds accesses and accesses to unbound
  resources.
* ``PIPE_CAP_CULL_DISTANCE``: Whether the driver supports the
  :ext:`GL_ARB_cull_distance` extension and thus implements proper support for
  culling planes.
* ``PIPE_CAP_CULL_DISTANCE_NOCOMBINE``: Whether the driver wants to skip
  running the ``nir_lower_clip_cull_distance_arrays`` pass in order to get
  VARYING_SLOT_CULL_DIST0 slot variables.
* ``PIPE_CAP_PRIMITIVE_RESTART_FOR_PATCHES``: Whether primitive restart is
  supported for patch primitives.
* ``PIPE_CAP_SHADER_GROUP_VOTE``: Whether the ``VOTE_*`` ops can be used in
  shaders.
* ``PIPE_CAP_MAX_WINDOW_RECTANGLES``: The maximum number of window rectangles
  supported in ``set_window_rectangles``.
* ``PIPE_CAP_POLYGON_OFFSET_UNITS_UNSCALED``: If true, the driver implements support
  for ``pipe_rasterizer_state::offset_units_unscaled``.
* ``PIPE_CAP_VIEWPORT_SUBPIXEL_BITS``: Number of bits of subpixel precision for
  floating point viewport bounds.
* ``PIPE_CAP_RASTERIZER_SUBPIXEL_BITS``: Number of bits of subpixel precision used
  by the rasterizer.
* ``PIPE_CAP_MIXED_COLOR_DEPTH_BITS``: Whether there is non-fallback
  support for color/depth format combinations that use a different
  number of bits. For the purpose of this cap, Z24 is treated as
  32-bit. If set to off, that means that a B5G6R5 + Z24 or RGBA8 + Z16
  combination will require a driver fallback, and should not be
  advertised in the GLX/EGL config list.
* ``PIPE_CAP_SHADER_ARRAY_COMPONENTS``: If true, the driver interprets the
  UsageMask of input and output declarations and allows declaring arrays
  in overlapping ranges. The components must be a contiguous range, e.g. a
  UsageMask of  xy or yzw is allowed, but xz or yw isn't. Declarations with
  overlapping locations must have matching semantic names and indices, and
  equal interpolation qualifiers.
  Components may overlap, notably when the gaps in an array of dvec3 are
  filled in.
* ``PIPE_CAP_STREAM_OUTPUT_PAUSE_RESUME``: Whether
  :ext:`GL_ARB_transform_feedback2` is supported, including pausing/resuming
  queries and having ``count_from_stream_output`` set on indirect draws to
  implement glDrawTransformFeedback.  Required for OpenGL 4.0.
* ``PIPE_CAP_STREAM_OUTPUT_INTERLEAVE_BUFFERS``: Whether interleaved stream
  output mode is able to interleave across buffers. This is required for
  :ext:`GL_ARB_transform_feedback3`.
* ``PIPE_CAP_SHADER_CAN_READ_OUTPUTS``: Whether every TGSI shader stage can read
  from the output file.
* ``PIPE_CAP_FBFETCH``: The number of render targets whose value in the
  current framebuffer can be read in the shader.  0 means framebuffer fetch
  is not supported.  1 means that only the first render target can be read,
  and a larger value would mean that multiple render targets are supported.
* ``PIPE_CAP_FBFETCH_COHERENT``: Whether framebuffer fetches from the fragment
  shader can be guaranteed to be coherent with framebuffer writes.
* ``PIPE_CAP_FBFETCH_ZS``: Whether fragment shader can fetch current values of
  Z/S attachments. These fetches are always coherent with framebuffer writes.
* ``PIPE_CAP_LEGACY_MATH_RULES``: Whether NIR shaders support the
  ``shader_info.use_legacy_math_rules`` flag (see documentation there), and
  TGSI shaders support the corresponding ``TGSI_PROPERTY_LEGACY_MATH_RULES``.
* ``PIPE_CAP_DOUBLES``: Whether double precision floating-point operations
  are supported.
* ``PIPE_CAP_INT64``: Whether 64-bit integer operations are supported.
* ``PIPE_CAP_TGSI_TEX_TXF_LZ``: Whether TEX_LZ and TXF_LZ opcodes are
  supported.
* ``PIPE_CAP_SHADER_CLOCK``: Whether the CLOCK opcode is supported.
* ``PIPE_CAP_POLYGON_MODE_FILL_RECTANGLE``: Whether the
  PIPE_POLYGON_MODE_FILL_RECTANGLE mode is supported for
  ``pipe_rasterizer_state::fill_front`` and
  ``pipe_rasterizer_state::fill_back``.
* ``PIPE_CAP_SPARSE_BUFFER_PAGE_SIZE``: The page size of sparse buffers in
  bytes, or 0 if sparse buffers are not supported. The page size must be at
  most 64KB.
* ``PIPE_CAP_SHADER_BALLOT``: Whether the BALLOT and READ_* opcodes as well as
  the SUBGROUP_* semantics are supported.
* ``PIPE_CAP_TES_LAYER_VIEWPORT``: Whether ``VARYING_SLOT_LAYER`` and
  ``VARYING_SLOT_VIEWPORT`` are supported as tessellation evaluation
  shader outputs.
* ``PIPE_CAP_CAN_BIND_CONST_BUFFER_AS_VERTEX``: Whether a buffer with just
  PIPE_BIND_CONSTANT_BUFFER can be legally passed to set_vertex_buffers.
* ``PIPE_CAP_ALLOW_MAPPED_BUFFERS_DURING_EXECUTION``: As the name says.
* ``PIPE_CAP_POST_DEPTH_COVERAGE``: whether
  ``TGSI_PROPERTY_FS_POST_DEPTH_COVERAGE`` is supported.
* ``PIPE_CAP_BINDLESS_TEXTURE``: Whether bindless texture operations are
  supported.
* ``PIPE_CAP_NIR_SAMPLERS_AS_DEREF``: Whether NIR tex instructions should
  reference texture and sampler as NIR derefs instead of by indices.
* ``PIPE_CAP_QUERY_SO_OVERFLOW``: Whether the
  ``PIPE_QUERY_SO_OVERFLOW_PREDICATE`` and
  ``PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE`` query types are supported. Note that
  for a driver that does not support multiple output streams (i.e.,
  ``PIPE_CAP_MAX_VERTEX_STREAMS`` is 1), both query types are identical.
* ``PIPE_CAP_MEMOBJ``: Whether operations on memory objects are supported.
* ``PIPE_CAP_LOAD_CONSTBUF``: True if the driver supports ``TGSI_OPCODE_LOAD`` use
  with constant buffers.
* ``PIPE_CAP_TILE_RASTER_ORDER``: Whether the driver supports
  :ext:`GL_MESA_tile_raster_order`, using the tile_raster_order_* fields in
  pipe_rasterizer_state.
* ``PIPE_CAP_MAX_COMBINED_SHADER_OUTPUT_RESOURCES``: Limit on combined shader
  output resources (images + buffers + fragment outputs). If 0 the state
  tracker works it out.
* ``PIPE_CAP_FRAMEBUFFER_MSAA_CONSTRAINTS``: This determines limitations
  on the number of samples that framebuffer attachments can have.
  Possible values:

    0. color.nr_samples == zs.nr_samples == color.nr_storage_samples
       (standard MSAA quality)
    1. color.nr_samples >= zs.nr_samples == color.nr_storage_samples
       (enhanced MSAA quality)
    2. color.nr_samples >= zs.nr_samples >= color.nr_storage_samples
       (full flexibility in tuning MSAA quality and performance)

  All color attachments must have the same number of samples and the same
  number of storage samples.
* ``PIPE_CAP_SIGNED_VERTEX_BUFFER_OFFSET``:
  Whether pipe_vertex_buffer::buffer_offset is treated as signed. The u_vbuf
  module needs this for optimal performance in workstation applications.
* ``PIPE_CAP_CONTEXT_PRIORITY_MASK``: For drivers that support per-context
  priorities, this returns a bitmask of ``PIPE_CONTEXT_PRIORITY_x`` for the
  supported priority levels.  A driver that does not support prioritized
  contexts can return 0.
* ``PIPE_CAP_FENCE_SIGNAL``: True if the driver supports signaling semaphores
  using fence_server_signal().
* ``PIPE_CAP_CONSTBUF0_FLAGS``: The bits of pipe_resource::flags that must be
  set when binding that buffer as constant buffer 0. If the buffer doesn't have
  those bits set, pipe_context::set_constant_buffer(.., 0, ..) is ignored
  by the driver, and the driver can throw assertion failures.
* ``PIPE_CAP_PACKED_UNIFORMS``: True if the driver supports packed uniforms
  as opposed to padding to vec4s.  Requires ``PIPE_SHADER_CAP_INTEGERS`` if
  ``lower_uniforms_to_ubo`` is set.
* ``PIPE_CAP_CONSERVATIVE_RASTER_POST_SNAP_TRIANGLES``: Whether the
  ``PIPE_CONSERVATIVE_RASTER_POST_SNAP`` mode is supported for triangles.
  The post-snap mode means the conservative rasterization occurs after
  the conversion from floating-point to fixed-point coordinates
  on the subpixel grid.
* ``PIPE_CAP_CONSERVATIVE_RASTER_POST_SNAP_POINTS_LINES``: Whether the
  ``PIPE_CONSERVATIVE_RASTER_POST_SNAP`` mode is supported for points and lines.
* ``PIPE_CAP_CONSERVATIVE_RASTER_PRE_SNAP_TRIANGLES``: Whether the
  ``PIPE_CONSERVATIVE_RASTER_PRE_SNAP`` mode is supported for triangles.
  The pre-snap mode means the conservative rasterization occurs before
  the conversion from floating-point to fixed-point coordinates.
* ``PIPE_CAP_CONSERVATIVE_RASTER_PRE_SNAP_POINTS_LINES``: Whether the
  ``PIPE_CONSERVATIVE_RASTER_PRE_SNAP`` mode is supported for points and lines.
* ``PIPE_CAP_CONSERVATIVE_RASTER_POST_DEPTH_COVERAGE``: Whether
  ``PIPE_CAP_POST_DEPTH_COVERAGE`` works with conservative rasterization.
* ``PIPE_CAP_CONSERVATIVE_RASTER_INNER_COVERAGE``: Whether
  inner_coverage from :ext:`GL_INTEL_conservative_rasterization` is supported.
* ``PIPE_CAP_MAX_CONSERVATIVE_RASTER_SUBPIXEL_PRECISION_BIAS``: The maximum
  subpixel precision bias in bits during conservative rasterization.
* ``PIPE_CAP_PROGRAMMABLE_SAMPLE_LOCATIONS``: True is the driver supports
  programmable sample location through ```get_sample_pixel_grid``` and
  ```set_sample_locations```.
* ``PIPE_CAP_MAX_GS_INVOCATIONS``: Maximum supported value of
  TGSI_PROPERTY_GS_INVOCATIONS.
* ``PIPE_CAP_MAX_SHADER_BUFFER_SIZE_UINT``: Maximum supported size for binding
  with set_shader_buffers. This is unsigned integer with the maximum of 4GB - 1.
* ``PIPE_CAP_MAX_COMBINED_SHADER_BUFFERS``: Maximum total number of shader
  buffers. A value of 0 means the sum of all per-shader stage maximums (see
  ``PIPE_SHADER_CAP_MAX_SHADER_BUFFERS``).
* ``PIPE_CAP_MAX_COMBINED_HW_ATOMIC_COUNTERS``: Maximum total number of atomic
  counters. A value of 0 means the default value (MAX_ATOMIC_COUNTERS = 4096).
* ``PIPE_CAP_MAX_COMBINED_HW_ATOMIC_COUNTER_BUFFERS``: Maximum total number of
  atomic counter buffers. A value of 0 means the sum of all per-shader stage
  maximums (see ``PIPE_SHADER_CAP_MAX_HW_ATOMIC_COUNTER_BUFFERS``).
* ``PIPE_CAP_MAX_TEXTURE_UPLOAD_MEMORY_BUDGET``: Maximum recommend memory size
  for all active texture uploads combined. This is a performance hint.
  0 means no limit.
* ``PIPE_CAP_MAX_VERTEX_ELEMENT_SRC_OFFSET``: The maximum supported value for
  of pipe_vertex_element::src_offset.
* ``PIPE_CAP_SURFACE_SAMPLE_COUNT``: Whether the driver
  supports pipe_surface overrides of resource nr_samples. If set, will
  enable :ext:`GL_EXT_multisampled_render_to_texture`.
* ``PIPE_CAP_IMAGE_ATOMIC_FLOAT_ADD``: Atomic floating point adds are
  supported on images, buffers, and shared memory.
* ``PIPE_CAP_GLSL_TESS_LEVELS_AS_INPUTS``: True if the driver wants TESSINNER and TESSOUTER to be inputs (rather than system values) for tessellation evaluation shaders.
* ``PIPE_CAP_DEST_SURFACE_SRGB_CONTROL``: Indicates whether the drivers
  supports switching the format between sRGB and linear for a surface that is
  used as destination in draw and blit calls.
* ``PIPE_CAP_NIR_COMPACT_ARRAYS``: True if the compiler backend supports NIR's compact array feature, for all shader stages.
* ``PIPE_CAP_MAX_VARYINGS``: The maximum number of fragment shader
  varyings. This will generally correspond to
  ``PIPE_SHADER_CAP_MAX_INPUTS`` for the fragment shader, but in some
  cases may be a smaller number.
* ``PIPE_CAP_COMPUTE_GRID_INFO_LAST_BLOCK``: Whether pipe_grid_info::last_block
  is implemented by the driver. See struct pipe_grid_info for more details.
* ``PIPE_CAP_COMPUTE_SHADER_DERIVATIVE``: True if the driver supports derivatives (and texture lookups with implicit derivatives) in compute shaders.
* ``PIPE_CAP_IMAGE_LOAD_FORMATTED``: True if a format for image loads does not need to be specified in the shader IR
* ``PIPE_CAP_IMAGE_STORE_FORMATTED``: True if a format for image stores does not need to be specified in the shader IR
* ``PIPE_CAP_THROTTLE``: Whether or not gallium frontends should throttle pipe_context
  execution. 0 = throttling is disabled.
* ``PIPE_CAP_DMABUF``: Whether Linux DMABUF handles are supported by
  resource_from_handle and resource_get_handle.
  Possible bit field values:

    1. ``DRM_PRIME_CAP_IMPORT``: resource_from_handle is supported
    2. ``DRM_PRIME_CAP_EXPORT``: resource_get_handle is supported

* ``PIPE_CAP_PREFER_COMPUTE_FOR_MULTIMEDIA``: Whether VDPAU, VAAPI, and
  OpenMAX should use a compute-based blit instead of pipe_context::blit and compute pipeline for compositing images.
* ``PIPE_CAP_FRAGMENT_SHADER_INTERLOCK``: True if fragment shader interlock
  functionality is supported.
* ``PIPE_CAP_ATOMIC_FLOAT_MINMAX``: Atomic float point minimum,
  maximum, exchange and compare-and-swap support to buffer and shared variables.
* ``PIPE_CAP_TGSI_DIV``: Whether opcode DIV is supported
* ``PIPE_CAP_DITHERING``: Whether dithering is supported
* ``PIPE_CAP_FRAGMENT_SHADER_TEXTURE_LOD``: Whether texture lookups with
  explicit LOD is supported in the fragment shader.
* ``PIPE_CAP_FRAGMENT_SHADER_DERIVATIVES``: True if the driver supports
  derivatives in fragment shaders.
* ``PIPE_CAP_TEXTURE_SHADOW_LOD``: True if the driver supports shadow sampler
  types with texture functions having interaction with LOD of texture lookup.
* ``PIPE_CAP_SHADER_SAMPLES_IDENTICAL``: True if the driver supports a shader query to tell whether all samples of a multisampled surface are definitely identical.
* ``PIPE_CAP_IMAGE_ATOMIC_INC_WRAP``: Atomic increment/decrement + wrap around
  are supported.
* ``PIPE_CAP_PREFER_IMM_ARRAYS_AS_CONSTBUF``: True if gallium frontends should
  turn arrays whose contents can be deduced at compile time into constant
  buffer loads, or false if the driver can handle such arrays itself in a more
  efficient manner (such as through nir_opt_large_constants() and nir->constant_data).
* ``PIPE_CAP_GL_SPIRV``: True if the driver supports :ext:`GL_ARB_gl_spirv` extension.
* ``PIPE_CAP_GL_SPIRV_VARIABLE_POINTERS``: True if the driver supports Variable Pointers in SPIR-V shaders.
* ``PIPE_CAP_DEMOTE_TO_HELPER_INVOCATION``: True if driver supports demote keyword in GLSL programs.
* ``PIPE_CAP_TGSI_TG4_COMPONENT_IN_SWIZZLE``: True if driver wants the TG4 component encoded in sampler swizzle rather than as a separate source.
* ``PIPE_CAP_FLATSHADE``: Driver supports pipe_rasterizer_state::flatshade.  Must be 1
    for non-NIR drivers or gallium nine.
* ``PIPE_CAP_ALPHA_TEST``: Driver supports alpha-testing.  Must be 1
    for non-NIR drivers or gallium nine.  If set, frontend may set
    ``pipe_depth_stencil_alpha_state->alpha_enabled`` and ``alpha_func``.
    Otherwise, alpha test will be lowered to a comparison and discard_if in the
    fragment shader.
* ``PIPE_CAP_POINT_SIZE_FIXED``: Driver supports point-sizes that are fixed,
  as opposed to writing gl_PointSize for every point.
* ``PIPE_CAP_TWO_SIDED_COLOR``: Driver supports two-sided coloring.  Must be 1
    for non-NIR drivers.  If set, pipe_rasterizer_state may be set to indicate
    that back-facing primitives should use the back-side color as the FS input
    color.  If unset, mesa/st will lower it to gl_FrontFacing reads in the
    fragment shader.
* ``PIPE_CAP_CLIP_PLANES``: Driver supports user-defined clip-planes. 0 denotes none, 1 denotes MAX_CLIP_PLANES. > 1 overrides MAX. When is 0, pipe_rasterizer_state::clip_plane_enable is unused.
* ``PIPE_CAP_MAX_VERTEX_BUFFERS``: Number of supported vertex buffers.
* ``PIPE_CAP_OPENCL_INTEGER_FUNCTIONS``: Driver supports extended OpenCL-style integer functions.  This includes average, saturating addition, saturating subtraction, absolute difference, count leading zeros, and count trailing zeros.
* ``PIPE_CAP_INTEGER_MULTIPLY_32X16``: Driver supports integer multiplication between a 32-bit integer and a 16-bit integer.  If the second operand is 32-bits, the upper 16-bits are ignored, and the low 16-bits are possibly sign extended as necessary.
* ``PIPE_CAP_NIR_IMAGES_AS_DEREF``: Whether NIR image load/store intrinsics should be nir_intrinsic_image_deref_* instead of nir_intrinsic_image_*.  Defaults to true.
* ``PIPE_CAP_PACKED_STREAM_OUTPUT``: Driver supports packing optimization for stream output (e.g. GL transform feedback captured variables). Defaults to true.
* ``PIPE_CAP_VIEWPORT_TRANSFORM_LOWERED``: Driver needs the nir_lower_viewport_transform pass to be enabled. This also means that the gl_Position value is modified and should be lowered for transform feedback, if needed. Defaults to false.
* ``PIPE_CAP_PSIZ_CLAMPED``: Driver needs for the point size to be clamped. Additionally, the gl_PointSize has been modified and its value should be lowered for transform feedback, if needed. Defaults to false.
* ``PIPE_CAP_GL_BEGIN_END_BUFFER_SIZE``: Buffer size used to upload vertices for glBegin/glEnd.
* ``PIPE_CAP_VIEWPORT_SWIZZLE``: Whether pipe_viewport_state::swizzle can be used to specify pre-clipping swizzling of coordinates (see :ext:`GL_NV_viewport_swizzle`).
* ``PIPE_CAP_SYSTEM_SVM``: True if all application memory can be shared with the GPU without explicit mapping.
* ``PIPE_CAP_VIEWPORT_MASK``: Whether ``TGSI_SEMANTIC_VIEWPORT_MASK`` and ``TGSI_PROPERTY_LAYER_VIEWPORT_RELATIVE`` are supported (see :ext:`GL_NV_viewport_array2`).
* ``PIPE_CAP_MAP_UNSYNCHRONIZED_THREAD_SAFE``: Whether mapping a buffer as unsynchronized from any thread is safe.
* ``PIPE_CAP_GLSL_ZERO_INIT``: Choose a default zero initialization some GLSL variables. If ``1``, then all GLSL shader variables and gl_FragColor are initialized to zero. If ``2``, then shader out variables are not initialized but function out variables are.
* ``PIPE_CAP_BLEND_EQUATION_ADVANCED``: Driver supports blend equation advanced without necessarily supporting FBFETCH.
* ``PIPE_CAP_NIR_ATOMICS_AS_DEREF``: Whether NIR atomics instructions should reference atomics as NIR derefs instead of by indices.
* ``PIPE_CAP_NO_CLIP_ON_COPY_TEX``: Driver doesn't want x/y/width/height clipped based on src size when doing a copy texture operation (e.g.: may want out-of-bounds reads that produce 0 instead of leaving the texture content undefined)
* ``PIPE_CAP_MAX_TEXTURE_MB``: Maximum texture size in MB (default is 1024)
* ``PIPE_CAP_DEVICE_PROTECTED_SURFACE``: Whether the device support protected / encrypted content.
* ``PIPE_CAP_PREFER_REAL_BUFFER_IN_CONSTBUF0``: The state tracker is encouraged to upload constants into a real buffer and bind it into constant buffer 0 instead of binding a user pointer. This may enable a faster code-path in a gallium frontend for drivers that really prefer a real buffer.
* ``PIPE_CAP_GL_CLAMP``: Driver natively supports GL_CLAMP.  Required for non-NIR drivers with the GL frontend.  NIR drivers with the cap unavailable will have GL_CLAMP lowered to txd/txl with a saturate on the coordinates.
* ``PIPE_CAP_TEXRECT``: Driver supports rectangle textures.  Required for OpenGL on ``!prefers_nir`` drivers.  If this cap is not present, st/mesa will lower the NIR to use normal 2D texture sampling by using either ``txs`` or ``nir_intrinsic_load_texture_scaling`` to normalize the texture coordinates.
* ``PIPE_CAP_SAMPLER_REDUCTION_MINMAX``: Driver supports EXT min/max sampler reduction.
* ``PIPE_CAP_SAMPLER_REDUCTION_MINMAX_ARB``: Driver supports ARB min/max sampler reduction with format queries.
* ``PIPE_CAP_EMULATE_NONFIXED_PRIMITIVE_RESTART``: Driver requests all draws using a non-fixed restart index to be rewritten to use a fixed restart index.
* ``PIPE_CAP_SUPPORTED_PRIM_MODES``: A bitmask of the ``mesa_prim`` enum values that the driver can natively support.
* ``PIPE_CAP_SUPPORTED_PRIM_MODES_WITH_RESTART``: A bitmask of the ``mesa_prim`` enum values that the driver can natively support for primitive restart. Only useful if ``PIPE_CAP_PRIMITIVE_RESTART`` is also exported.
* ``PIPE_CAP_PREFER_BACK_BUFFER_REUSE``: Only applies to DRI_PRIME. If 1, the driver prefers that DRI3 tries to use the same back buffer each frame. If 0, this means DRI3 will at least use 2 back buffers and ping-pong between them to allow the tiled->linear copy to run in parallel.
* ``PIPE_CAP_DRAW_VERTEX_STATE``: Driver supports ``pipe_screen::create_vertex_state/vertex_state_destroy`` and ``pipe_context::draw_vertex_state``. Only used by display lists and designed to serve vbo_save.
* ``PIPE_CAP_PREFER_POT_ALIGNED_VARYINGS``: Driver prefers varyings to be aligned to power of two in a slot. If this cap is enabled, vec4 varying will be placed in .xyzw components of the varying slot, vec3 in .xyz and vec2 in .xy or .zw
* ``PIPE_CAP_MAX_SPARSE_TEXTURE_SIZE``: Maximum 1D/2D/rectangle texture image dimension for a sparse texture.
* ``PIPE_CAP_MAX_SPARSE_3D_TEXTURE_SIZE``: Maximum 3D texture image dimension for a sparse texture.
* ``PIPE_CAP_MAX_SPARSE_ARRAY_TEXTURE_LAYERS``: Maximum number of layers in a sparse array texture.
* ``PIPE_CAP_SPARSE_TEXTURE_FULL_ARRAY_CUBE_MIPMAPS``: TRUE if there are no restrictions on the allocation of mipmaps in sparse textures and FALSE otherwise. See SPARSE_TEXTURE_FULL_ARRAY_CUBE_MIPMAPS_ARB description in :ext:`GL_ARB_sparse_texture` extension spec.
* ``PIPE_CAP_QUERY_SPARSE_TEXTURE_RESIDENCY``: TRUE if shader sparse texture sample instruction could also return the residency information.
* ``PIPE_CAP_CLAMP_SPARSE_TEXTURE_LOD``: TRUE if shader sparse texture sample instruction support clamp the minimal lod to prevent read from uncommitted pages.
* ``PIPE_CAP_ALLOW_DRAW_OUT_OF_ORDER``: TRUE if the driver allows the "draw out of order" optimization to be enabled. See _mesa_update_allow_draw_out_of_order for more details.
* ``PIPE_CAP_MAX_CONSTANT_BUFFER_SIZE_UINT``: Maximum bound constant buffer size in bytes. This is unsigned integer with the maximum of 4GB - 1. This applies to all constant buffers used by UBOs, unlike ``PIPE_SHADER_CAP_MAX_CONST_BUFFER0_SIZE``, which is specifically for GLSL uniforms.
* ``PIPE_CAP_HARDWARE_GL_SELECT``: Enable hardware accelerated GL_SELECT for this driver.
* ``PIPE_CAP_DEVICE_PROTECTED_CONTEXT``: Whether the device supports protected / encrypted context which can manipulate protected / encrypted content (some devices might need protected contexts to access protected content, whereas ``PIPE_CAP_DEVICE_PROTECTED_SURFACE`` does not require any particular context to do so).
* ``PIPE_CAP_ALLOW_GLTHREAD_BUFFER_SUBDATA_OPT``: Whether to allow glthread to convert glBufferSubData to glCopyBufferSubData. This may improve or worsen performance depending on your driver.
* ``PIPE_CAP_NULL_TEXTURES`` : Whether the driver supports sampling from NULL textures.
* ``PIPE_CAP_ASTC_VOID_EXTENTS_NEED_DENORM_FLUSH`` : True if the driver/hardware needs denormalized values in ASTC void extent blocks flushed to zero.
* ``PIPE_CAP_VALIDATE_ALL_DIRTY_STATES`` : Whether state validation must also validate the state changes for resources types used in the previous shader but not in the current shader.
* ``PIPE_CAP_HAS_CONST_BW``: Whether the driver only supports non-data-dependent layouts (ie. not bandwidth compressed formats like AFBC, UBWC, etc), or supports ``PIPE_BIND_CONST_BW`` to disable data-dependent layouts on requested resources.
* ``PIPE_CAP_PERFORMANCE_MONITOR``: Whether GL_AMD_performance_monitor should be exposed.


.. _pipe_capf:

PIPE_CAPF_*
^^^^^^^^^^^^^^^^

The floating-point capabilities are:

* ``PIPE_CAPF_MIN_LINE_WIDTH``: The minimum width of a regular line.
* ``PIPE_CAPF_MIN_LINE_WIDTH_AA``: The minimum width of a smoothed line.
* ``PIPE_CAPF_MAX_LINE_WIDTH``: The maximum width of a regular line.
* ``PIPE_CAPF_MAX_LINE_WIDTH_AA``: The maximum width of a smoothed line.
* ``PIPE_CAPF_LINE_WIDTH_GRANULARITY``: The line width is rounded to a multiple of this number.
* ``PIPE_CAPF_MIN_POINT_SIZE``: The minimum width and height of a point.
* ``PIPE_CAPF_MIN_POINT_SIZE_AA``: The minimum width and height of a smoothed point.
* ``PIPE_CAPF_MAX_POINT_SIZE``: The maximum width and height of a point.
* ``PIPE_CAPF_MAX_POINT_SIZE_AA``: The maximum width and height of a smoothed point.
* ``PIPE_CAPF_POINT_SIZE_GRANULARITY``: The point size is rounded to a multiple of this number.
* ``PIPE_CAPF_MAX_TEXTURE_ANISOTROPY``: The maximum level of anisotropy that can be
  applied to anisotropically filtered textures.
* ``PIPE_CAPF_MAX_TEXTURE_LOD_BIAS``: The maximum :term:`LOD` bias that may be applied
  to filtered textures.
* ``PIPE_CAPF_MIN_CONSERVATIVE_RASTER_DILATE``: The minimum conservative rasterization
  dilation.
* ``PIPE_CAPF_MAX_CONSERVATIVE_RASTER_DILATE``: The maximum conservative rasterization
  dilation.
* ``PIPE_CAPF_CONSERVATIVE_RASTER_DILATE_GRANULARITY``: The conservative rasterization
  dilation granularity for values relative to the minimum dilation.


.. _pipe_shader_cap:

PIPE_SHADER_CAP_*
^^^^^^^^^^^^^^^^^

These are per-shader-stage capabitity queries. Different shader stages may
support different features.

* ``PIPE_SHADER_CAP_MAX_INSTRUCTIONS``: The maximum number of instructions.
* ``PIPE_SHADER_CAP_MAX_ALU_INSTRUCTIONS``: The maximum number of arithmetic instructions.
* ``PIPE_SHADER_CAP_MAX_TEX_INSTRUCTIONS``: The maximum number of texture instructions.
* ``PIPE_SHADER_CAP_MAX_TEX_INDIRECTIONS``: The maximum number of texture indirections.
* ``PIPE_SHADER_CAP_MAX_CONTROL_FLOW_DEPTH``: The maximum nested control flow depth.
* ``PIPE_SHADER_CAP_MAX_INPUTS``: The maximum number of input registers.
* ``PIPE_SHADER_CAP_MAX_OUTPUTS``: The maximum number of output registers.
  This is valid for all shaders except the fragment shader.
* ``PIPE_SHADER_CAP_MAX_CONST_BUFFER0_SIZE``: The maximum size of constant buffer 0 in bytes.
* ``PIPE_SHADER_CAP_MAX_CONST_BUFFERS``: Maximum number of constant buffers that can be bound
  to any shader stage using ``set_constant_buffer``. If 0 or 1, the pipe will
  only permit binding one constant buffer per shader.

  If a value greater than 0 is returned, the driver can have multiple
  constant buffers bound to shader stages. The CONST register file is
  accessed with two-dimensional indices, like in the example below.

  ::

    DCL CONST[0][0..7]       # declare first 8 vectors of constbuf 0
    DCL CONST[3][0]          # declare first vector of constbuf 3
    MOV OUT[0], CONST[0][3]  # copy vector 3 of constbuf 0

* ``PIPE_SHADER_CAP_MAX_TEMPS``: The maximum number of temporary registers.
* ``PIPE_SHADER_CAP_CONT_SUPPORTED``: Whether continue is supported.
* ``PIPE_SHADER_CAP_INDIRECT_INPUT_ADDR``: Whether indirect addressing
  of the input file is supported.
* ``PIPE_SHADER_CAP_INDIRECT_OUTPUT_ADDR``: Whether indirect addressing
  of the output file is supported.
* ``PIPE_SHADER_CAP_INDIRECT_TEMP_ADDR``: Whether indirect addressing
  of the temporary file is supported.
* ``PIPE_SHADER_CAP_INDIRECT_CONST_ADDR``: Whether indirect addressing
  of the constant file is supported.
* ``PIPE_SHADER_CAP_SUBROUTINES``: Whether subroutines are supported, i.e.
  BGNSUB, ENDSUB, CAL, and RET, including RET in the main block.
* ``PIPE_SHADER_CAP_INTEGERS``: Whether integer opcodes are supported.
  If unsupported, only float opcodes are supported.
* ``PIPE_SHADER_CAP_INT64_ATOMICS``: Whether int64 atomic opcodes are supported. The device needs to support add, sub, swap, cmpswap, and, or, xor, min, and max.
* ``PIPE_SHADER_CAP_FP16``: Whether half precision floating-point opcodes are supported.
   If unsupported, half precision ops need to be lowered to full precision.
* ``PIPE_SHADER_CAP_FP16_DERIVATIVES``: Whether half precision floating-point
  DDX and DDY opcodes are supported.
* ``PIPE_SHADER_CAP_FP16_CONST_BUFFERS``: Whether half precision floating-point
  constant buffer loads are supported. Drivers are recommended to report 0
  if x86 F16C is not supported by the CPU (or an equivalent instruction set
  on other CPU architectures), otherwise they could be impacted by emulated
  FP16 conversions in glUniform.
* ``PIPE_SHADER_CAP_INT16``: Whether 16-bit signed and unsigned integer types
  are supported.
* ``PIPE_SHADER_CAP_GLSL_16BIT_CONSTS``: Lower mediump constants to 16-bit.
  Note that 16-bit constants are not lowered to uniforms in GLSL.
* ``PIPE_SHADER_CAP_MAX_TEXTURE_SAMPLERS``: The maximum number of texture
  samplers.
* ``PIPE_SHADER_CAP_MAX_SAMPLER_VIEWS``: The maximum number of texture
  sampler views. Must not be lower than PIPE_SHADER_CAP_MAX_TEXTURE_SAMPLERS.
* ``PIPE_SHADER_CAP_TGSI_ANY_INOUT_DECL_RANGE``: Whether the driver doesn't
  ignore tgsi_declaration_range::Last for shader inputs and outputs.
* ``PIPE_SHADER_CAP_MAX_SHADER_BUFFERS``: Maximum number of memory buffers
  (also used to implement atomic counters). Having this be non-0 also
  implies support for the ``LOAD``, ``STORE``, and ``ATOM*`` TGSI
  opcodes.
* ``PIPE_SHADER_CAP_SUPPORTED_IRS``: Supported representations of the
  program.  It should be a mask of ``pipe_shader_ir`` bits.
* ``PIPE_SHADER_CAP_MAX_SHADER_IMAGES``: Maximum number of image units.
* ``PIPE_SHADER_CAP_MAX_HW_ATOMIC_COUNTERS``: If atomic counters are separate,
  how many HW counters are available for this stage. (0 uses SSBO atomics).
* ``PIPE_SHADER_CAP_MAX_HW_ATOMIC_COUNTER_BUFFERS``: If atomic counters are
  separate, how many atomic counter buffers are available for this stage.

.. _pipe_compute_cap:

PIPE_COMPUTE_CAP_*
^^^^^^^^^^^^^^^^^^

Compute-specific capabilities. They can be queried using
pipe_screen::get_compute_param.

* ``PIPE_COMPUTE_CAP_IR_TARGET``: A description of the target of the form
  ``processor-arch-manufacturer-os`` that will be passed on to the compiler.
  This CAP is only relevant for drivers that specify PIPE_SHADER_IR_NATIVE for
  their preferred IR.
  Value type: null-terminated string. Shader IR type dependent.
* ``PIPE_COMPUTE_CAP_GRID_DIMENSION``: Number of supported dimensions
  for grid and block coordinates.  Value type: ``uint64_t``. Shader IR type dependent.
* ``PIPE_COMPUTE_CAP_MAX_GRID_SIZE``: Maximum grid size in block
  units.  Value type: ``uint64_t []``.  Shader IR type dependent.
* ``PIPE_COMPUTE_CAP_MAX_BLOCK_SIZE``: Maximum block size in thread
  units.  Value type: ``uint64_t []``. Shader IR type dependent.
* ``PIPE_COMPUTE_CAP_MAX_THREADS_PER_BLOCK``: Maximum number of threads that
  a single block can contain.  Value type: ``uint64_t``. Shader IR type dependent.
  This may be less than the product of the components of MAX_BLOCK_SIZE and is
  usually limited by the number of threads that can be resident simultaneously
  on a compute unit.
* ``PIPE_COMPUTE_CAP_MAX_GLOBAL_SIZE``: Maximum size of the GLOBAL
  resource.  Value type: ``uint64_t``. Shader IR type dependent.
* ``PIPE_COMPUTE_CAP_MAX_LOCAL_SIZE``: Maximum size of the LOCAL
  resource.  Value type: ``uint64_t``. Shader IR type dependent.
* ``PIPE_COMPUTE_CAP_MAX_PRIVATE_SIZE``: Maximum size of the PRIVATE
  resource.  Value type: ``uint64_t``. Shader IR type dependent.
* ``PIPE_COMPUTE_CAP_MAX_INPUT_SIZE``: Maximum size of the INPUT
  resource.  Value type: ``uint64_t``. Shader IR type dependent.
* ``PIPE_COMPUTE_CAP_MAX_MEM_ALLOC_SIZE``: Maximum size of a memory object
  allocation in bytes.  Value type: ``uint64_t``.
* ``PIPE_COMPUTE_CAP_MAX_CLOCK_FREQUENCY``: Maximum frequency of the GPU
  clock in MHz. Value type: ``uint32_t``
* ``PIPE_COMPUTE_CAP_MAX_COMPUTE_UNITS``: Maximum number of compute units
  Value type: ``uint32_t``
* ``PIPE_COMPUTE_CAP_MAX_SUBGROUPS``: The max amount of subgroups there can be
  inside a block. Non 0 indicates support for OpenCL subgroups including
  implementing ``get_compute_state_subgroup_size`` if multiple subgroup sizes
  are supported.
* ``PIPE_COMPUTE_CAP_IMAGES_SUPPORTED``: Whether images are supported
  non-zero means yes, zero means no. Value type: ``uint32_t``
* ``PIPE_COMPUTE_CAP_SUBGROUP_SIZES``: Ored power of two sizes of a basic execution
  unit in threads. Also known as wavefront size, warp size or SIMD width.
  E.g. `64 | 32`.
* ``PIPE_COMPUTE_CAP_ADDRESS_BITS``: The default compute device address space
  size specified as an unsigned integer value in bits.
* ``PIPE_COMPUTE_CAP_MAX_VARIABLE_THREADS_PER_BLOCK``: Maximum variable number
  of threads that a single block can contain. This is similar to
  PIPE_COMPUTE_CAP_MAX_THREADS_PER_BLOCK, except that the variable size is not
  known a compile-time but at dispatch-time.

.. _pipe_bind:

PIPE_BIND_*
^^^^^^^^^^^

These flags indicate how a resource will be used and are specified at resource
creation time. Resources may be used in different roles
during their life cycle. Bind flags are cumulative and may be combined to create
a resource which can be used for multiple things.
Depending on the pipe driver's memory management and these bind flags,
resources might be created and handled quite differently.

* ``PIPE_BIND_RENDER_TARGET``: A color buffer or pixel buffer which will be
  rendered to.  Any surface/resource attached to pipe_framebuffer_state::cbufs
  must have this flag set.
* ``PIPE_BIND_DEPTH_STENCIL``: A depth (Z) buffer and/or stencil buffer. Any
  depth/stencil surface/resource attached to pipe_framebuffer_state::zsbuf must
  have this flag set.
* ``PIPE_BIND_BLENDABLE``: Used in conjunction with PIPE_BIND_RENDER_TARGET to
  query whether a device supports blending for a given format.
  If this flag is set, surface creation may fail if blending is not supported
  for the specified format. If it is not set, a driver may choose to ignore
  blending on surfaces with formats that would require emulation.
* ``PIPE_BIND_DISPLAY_TARGET``: A surface that can be presented to screen. Arguments to
  pipe_screen::flush_front_buffer must have this flag set.
* ``PIPE_BIND_SAMPLER_VIEW``: A texture that may be sampled from in a fragment
  or vertex shader.
* ``PIPE_BIND_VERTEX_BUFFER``: A vertex buffer.
* ``PIPE_BIND_INDEX_BUFFER``: An vertex index/element buffer.
* ``PIPE_BIND_CONSTANT_BUFFER``: A buffer of shader constants.
* ``PIPE_BIND_STREAM_OUTPUT``: A stream output buffer.
* ``PIPE_BIND_CUSTOM``:
* ``PIPE_BIND_SCANOUT``: A front color buffer or scanout buffer.
* ``PIPE_BIND_SHARED``: A shareable buffer that can be given to another
  process.
* ``PIPE_BIND_GLOBAL``: A buffer that can be mapped into the global
  address space of a compute program.
* ``PIPE_BIND_SHADER_BUFFER``: A buffer without a format that can be bound
  to a shader and can be used with load, store, and atomic instructions.
* ``PIPE_BIND_SHADER_IMAGE``: A buffer or texture with a format that can be
  bound to a shader and can be used with load, store, and atomic instructions.
* ``PIPE_BIND_COMPUTE_RESOURCE``: A buffer or texture that can be
  bound to the compute program as a shader resource.
* ``PIPE_BIND_COMMAND_ARGS_BUFFER``: A buffer that may be sourced by the
  GPU command processor. It can contain, for example, the arguments to
  indirect draw calls.

.. _pipe_usage:

PIPE_USAGE_*
^^^^^^^^^^^^

The PIPE_USAGE enums are hints about the expected usage pattern of a resource.
Note that drivers must always support read and write CPU access at any time
no matter which hint they got.

* ``PIPE_USAGE_DEFAULT``: Optimized for fast GPU access.
* ``PIPE_USAGE_IMMUTABLE``: Optimized for fast GPU access and the resource is
  not expected to be mapped or changed (even by the GPU) after the first upload.
* ``PIPE_USAGE_DYNAMIC``: Expect frequent write-only CPU access. What is
  uploaded is expected to be used at least several times by the GPU.
* ``PIPE_USAGE_STREAM``: Expect frequent write-only CPU access. What is
  uploaded is expected to be used only once by the GPU.
* ``PIPE_USAGE_STAGING``: Optimized for fast CPU access.


Methods
-------

XXX to-do

get_name
^^^^^^^^

Returns an identifying name for the screen.

The returned string should remain valid and immutable for the lifetime of
pipe_screen.

get_vendor
^^^^^^^^^^

Returns the screen vendor.

The returned string should remain valid and immutable for the lifetime of
pipe_screen.

get_device_vendor
^^^^^^^^^^^^^^^^^

Returns the actual vendor of the device driving the screen
(as opposed to the driver vendor).

The returned string should remain valid and immutable for the lifetime of
pipe_screen.

.. _get_param:

get_param
^^^^^^^^^

Get an integer/boolean screen parameter.

**param** is one of the :ref:`PIPE_CAP` names.

.. _get_paramf:

get_paramf
^^^^^^^^^^

Get a floating-point screen parameter.

**param** is one of the :ref:`PIPE_CAPF` names.

context_create
^^^^^^^^^^^^^^

Create a pipe_context.

**priv** is private data of the caller, which may be put to various
unspecified uses, typically to do with implementing swapbuffers
and/or front-buffer rendering.

is_format_supported
^^^^^^^^^^^^^^^^^^^

Determine if a resource in the given format can be used in a specific manner.

**format** the resource format

**target** one of the PIPE_TEXTURE_x flags

**sample_count** the number of samples. 0 and 1 mean no multisampling,
the maximum allowed legal value is 32.

**storage_sample_count** the number of storage samples. This must be <=
sample_count. See the documentation of ``pipe_resource::nr_storage_samples``.

**bindings** is a bitmask of :ref:`PIPE_BIND` flags.

Returns TRUE if all usages can be satisfied.


can_create_resource
^^^^^^^^^^^^^^^^^^^

Check if a resource can actually be created (but don't actually allocate any
memory).  This is used to implement OpenGL's proxy textures.  Typically, a
driver will simply check if the total size of the given resource is less than
some limit.

For PIPE_TEXTURE_CUBE, the pipe_resource::array_size field should be 6.


.. _resource_create:

resource_create
^^^^^^^^^^^^^^^

Create a new resource from a template.
The following fields of the pipe_resource must be specified in the template:

**target** one of the pipe_texture_target enums.
Note that PIPE_BUFFER and PIPE_TEXTURE_X are not really fundamentally different.
Modern APIs allow using buffers as shader resources.

**format** one of the pipe_format enums.

**width0** the width of the base mip level of the texture or size of the buffer.

**height0** the height of the base mip level of the texture
(1 for 1D or 1D array textures).

**depth0** the depth of the base mip level of the texture
(1 for everything else).

**array_size** the array size for 1D and 2D array textures.
For cube maps this must be 6, for other textures 1.

**last_level** the last mip map level present.

**nr_samples**: Number of samples determining quality, driving the rasterizer,
shading, and framebuffer. It is the number of samples seen by the whole
graphics pipeline. 0 and 1 specify a resource which isn't multisampled.

**nr_storage_samples**: Only color buffers can set this lower than nr_samples.
Multiple samples within a pixel can have the same color. ``nr_storage_samples``
determines how many slots for different colors there are per pixel.
If there are not enough slots to store all sample colors, some samples will
have an undefined color (called "undefined samples").

The resolve blit behavior is driver-specific, but can be one of these two:

1. Only defined samples will be averaged. Undefined samples will be ignored.
2. Undefined samples will be approximated by looking at surrounding defined
   samples (even in different pixels).

Blits and MSAA texturing: If the sample being fetched is undefined, one of
the defined samples is returned instead.

Sample shading (``set_min_samples``) will operate at a sample frequency that
is at most ``nr_storage_samples``. Greater ``min_samples`` values will be
replaced by ``nr_storage_samples``.

**usage** one of the :ref:`PIPE_USAGE` flags.

**bind** bitmask of the :ref:`PIPE_BIND` flags.

**flags** bitmask of PIPE_RESOURCE_FLAG flags.

**next**: Pointer to the next plane for resources that consist of multiple
memory planes.

As a corollary, this mean resources for an image with multiple planes have
to be created starting from the highest plane.

resource_changed
^^^^^^^^^^^^^^^^

Mark a resource as changed so derived internal resources will be recreated
on next use.

When importing external images that can't be directly used as texture sampler
source, internal copies may have to be created that the hardware can sample
from. When those resources are reimported, the image data may have changed, and
the previously derived internal resources must be invalidated to avoid sampling
from old copies.



resource_destroy
^^^^^^^^^^^^^^^^

Destroy a resource. A resource is destroyed if it has no more references.



get_timestamp
^^^^^^^^^^^^^

Query a timestamp in nanoseconds. The returned value should match
PIPE_QUERY_TIMESTAMP. This function returns immediately and doesn't
wait for rendering to complete (which cannot be achieved with queries).



get_driver_query_info
^^^^^^^^^^^^^^^^^^^^^

Return a driver-specific query. If the **info** parameter is NULL,
the number of available queries is returned.  Otherwise, the driver
query at the specified **index** is returned in **info**.
The function returns non-zero on success.
The driver-specific query is described with the pipe_driver_query_info
structure.

get_driver_query_group_info
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Return a driver-specific query group. If the **info** parameter is NULL,
the number of available groups is returned.  Otherwise, the driver
query group at the specified **index** is returned in **info**.
The function returns non-zero on success.
The driver-specific query group is described with the
pipe_driver_query_group_info structure.



get_disk_shader_cache
^^^^^^^^^^^^^^^^^^^^^

Returns a pointer to a driver-specific on-disk shader cache. If the driver
failed to create the cache or does not support an on-disk shader cache NULL is
returned. The callback itself may also be NULL if the driver doesn't support
an on-disk shader cache.


is_dmabuf_modifier_supported
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Query whether the driver supports a **modifier** in combination with a
**format**, and whether it is only supported with "external" texture targets.
If the combination is supported in any fashion, true is returned.  If the
**external_only** parameter is not NULL, the bool it points to is set to
false if non-external texture targets are supported with the specified modifier+
format, or true if only external texture targets are supported.


get_dmabuf_modifier_planes
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Query the number of planes required by the image layout specified by the
**modifier** and **format** parameters.  The value returned includes both planes
dictated by **format** and any additional planes required for driver-specific
auxiliary data necessary for the layout defined by **modifier**.
If the proc is NULL, no auxiliary planes are required for any layout supported by
**screen** and the number of planes can be derived directly from **format**.


Thread safety
-------------

Screen methods are required to be thread safe. While gallium rendering
contexts are not required to be thread safe, it is required to be safe to use
different contexts created with the same screen in different threads without
locks. It is also required to be safe using screen methods in a thread, while
using one of its contexts in another (without locks).
