.. _resource:

Resources and derived objects
=============================

Resources represent objects that hold data: textures and buffers.

They are mostly modelled after the resources in Direct3D 10/11, but with a
different transfer/update mechanism, and more features for OpenGL support.

Resources can be used in several ways, and it is required to specify all planned uses through an appropriate set of bind flags.

TODO: write much more on resources

Transfers
---------

Transfers are the mechanism used to access resources with the CPU.

OpenGL: OpenGL supports mapping buffers and has inline transfer functions for both buffers and textures

D3D11: D3D11 lacks transfers, but has special resource types that are mappable to the CPU address space

TODO: write much more on transfers

Resource targets
----------------

Resource targets determine the type of a resource.

Note that drivers may not actually have the restrictions listed regarding
coordinate normalization and wrap modes, and in fact efficient OpenCL
support will probably require drivers that don't have any of them, which
will probably be advertised with an appropriate cap.

TODO: document all targets. Note that both 3D and cube have restrictions
that depend on the hardware generation.


PIPE_BUFFER
^^^^^^^^^^^

Buffer resource: can be used as a vertex, index, constant buffer
(appropriate bind flags must be requested).

Buffers do not really have a format, it's just bytes, but they are required
to have their type set to a R8 format (without a specific "just byte" format,
R8_UINT would probably make the most sense, but for historic reasons R8_UNORM
is OK too). (This is just to make some shared buffer/texture code easier so
format size can be queried.)
width0 serves as size, most other resource properties don't apply but must be
set appropriately (depth0/height0/array_size must be 1, last_level 0).

They can be bound to stream output if supported.
TODO: what about the restrictions lifted by the several later GL transform feedback extensions? How does one advertise that in Gallium?

They can be also be bound to a shader stage (for sampling) as usual by
creating an appropriate sampler view, if the driver supports PIPE_CAP_TEXTURE_BUFFER_OBJECTS.
This supports larger width than a 1d texture would
(TODO limit currently unspecified, minimum must be at least 65536).
Only the "direct fetch" sample opcodes are supported (TGSI_OPCODE_TXF,
TGSI_OPCODE_SAMPLE_I) so the sampler state (coord wrapping etc.)
is mostly ignored (with SAMPLE_I there's no sampler state at all).

They can be also be bound to the framebuffer (only as color render target, not
depth buffer, also there cannot be a depth buffer bound at the same time) as usual
by creating an appropriate view (this is not usable in OpenGL).
TODO there's no CAP bit currently for this, there's also unspecified size etc. limits
TODO: is there any chance of supporting GL pixel buffer object acceleration with this?


OpenGL: vertex buffers in GL 1.5 or :ext:`GL_ARB_vertex_buffer_object`

- Binding to stream out requires GL 3.0 or :ext:`GL_NV_transform_feedback`
- Binding as constant buffers requires GL 3.1 or :ext:`GL_ARB_uniform_buffer_object`
- Binding to a sampling stage requires GL 3.1 or :ext:`GL_ARB_texture_buffer_object`

D3D11: buffer resources
- Binding to a render target requires D3D_FEATURE_LEVEL_10_0

PIPE_TEXTURE_1D / PIPE_TEXTURE_1D_ARRAY
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
1D surface accessed with normalized coordinates.
1D array textures are supported depending on PIPE_CAP_MAX_TEXTURE_ARRAY_LAYERS.

- If PIPE_CAP_NPOT_TEXTURES is not supported,
      width must be a power of two
- height0 must be 1
- depth0 must be 1
- array_size must be 1 for PIPE_TEXTURE_1D
- Mipmaps can be used
- Must use normalized coordinates

OpenGL: GL_TEXTURE_1D in GL 1.0

- PIPE_CAP_NPOT_TEXTURES is equivalent to GL 2.0 or :ext:`GL_ARB_texture_non_power_of_two`

D3D11: 1D textures in D3D_FEATURE_LEVEL_10_0

PIPE_TEXTURE_RECT
^^^^^^^^^^^^^^^^^
2D surface with OpenGL GL_TEXTURE_RECTANGLE semantics.

- depth0 must be 1
- array_size must be 1
- last_level must be 0
- Must use unnormalized coordinates
- Must use a clamp wrap mode

OpenGL: GL_TEXTURE_RECTANGLE in GL 3.1 or :ext:`GL_ARB_texture_rectangle` or
:ext:`GL_NV_texture_rectangle`

OpenCL: can create OpenCL images based on this, that can then be sampled arbitrarily

D3D11: not supported (only PIPE_TEXTURE_2D with normalized coordinates is supported)

PIPE_TEXTURE_2D / PIPE_TEXTURE_2D_ARRAY
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
2D surface accessed with normalized coordinates.
2D array textures are supported depending on PIPE_CAP_MAX_TEXTURE_ARRAY_LAYERS.

- If PIPE_CAP_NPOT_TEXTURES is not supported,
      width and height must be powers of two
- depth0 must be 1
- array_size must be 1 for PIPE_TEXTURE_2D
- Mipmaps can be used
- Must use normalized coordinates
- No special restrictions on wrap modes

OpenGL: GL_TEXTURE_2D in GL 1.0

- PIPE_CAP_NPOT_TEXTURES is equivalent to GL 2.0 or :ext:`GL_ARB_texture_non_power_of_two`

OpenCL: can create OpenCL images based on this, that can then be sampled arbitrarily

D3D11: 2D textures

- PIPE_CAP_NPOT_TEXTURES is equivalent to D3D_FEATURE_LEVEL_9_3

PIPE_TEXTURE_3D
^^^^^^^^^^^^^^^

3-dimensional array of texels.
Mipmap dimensions are reduced in all 3 coordinates.

- If PIPE_CAP_NPOT_TEXTURES is not supported,
      width, height and depth must be powers of two
- array_size must be 1
- Must use normalized coordinates

OpenGL: GL_TEXTURE_3D in GL 1.2 or :ext:`GL_EXT_texture3D`

- PIPE_CAP_NPOT_TEXTURES is equivalent to GL 2.0 or :ext:`GL_ARB_texture_non_power_of_two`

D3D11: 3D textures

- PIPE_CAP_NPOT_TEXTURES is equivalent to D3D_FEATURE_LEVEL_10_0

PIPE_TEXTURE_CUBE / PIPE_TEXTURE_CUBE_ARRAY
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Cube maps consist of 6 2D faces.
The 6 surfaces form an imaginary cube, and sampling happens by mapping an
input 3-vector to the point of the cube surface in that direction.
Cube map arrays are supported depending on PIPE_CAP_CUBE_MAP_ARRAY.

Sampling may be optionally seamless if a driver supports it (PIPE_CAP_SEAMLESS_CUBE_MAP),
resulting in filtering taking samples from multiple surfaces near to the edge.

- Width and height must be equal
- depth0 must be 1
- array_size must be a multiple of 6
- If PIPE_CAP_NPOT_TEXTURES is not supported,
      width and height must be powers of two
- Must use normalized coordinates

OpenGL: GL_TEXTURE_CUBE_MAP in GL 1.3 or :ext:`GL_EXT_texture_cube_map`

- PIPE_CAP_NPOT_TEXTURES is equivalent to GL 2.0 or :ext:`GL_ARB_texture_non_power_of_two`
- Seamless cube maps require GL 3.2 or :ext:`GL_ARB_seamless_cube_map` or :ext:`GL_AMD_seamless_cubemap_per_texture`
- Cube map arrays require GL 4.0 or :ext:`GL_ARB_texture_cube_map_array`

D3D11: 2D array textures with the D3D11_RESOURCE_MISC_TEXTURECUBE flag

- PIPE_CAP_NPOT_TEXTURES is equivalent to D3D_FEATURE_LEVEL_10_0
- Cube map arrays require D3D_FEATURE_LEVEL_10_1

Surfaces
--------

Surfaces are views of a resource that can be bound as a framebuffer to serve as the render target or depth buffer.

TODO: write much more on surfaces

OpenGL: FBOs are collections of surfaces in GL 3.0 or :ext:`GL_ARB_framebuffer_object`

D3D11: render target views and depth/stencil views

Sampler views
-------------

Sampler views are views of a resource that can be bound to a pipeline stage to be sampled from shaders.

TODO: write much more on sampler views

OpenGL: texture objects are actually sampler view and resource in a single unit

D3D11: shader resource views
