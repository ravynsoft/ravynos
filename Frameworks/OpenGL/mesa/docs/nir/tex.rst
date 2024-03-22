NIR Texture Instructions
========================

Even though texture instructions *could* be supported as intrinsics, the vast
number of combinations mean that doing so is practically impossible. Instead,
NIR has a dedicated texture instruction.  There are several texture operations:

.. c:autoenum:: nir_texop
   :file: src/compiler/nir/nir.h
   :members:

As with other instruction types, there is still an array of sources, except
that each source also has a *type* associated with it.  There are various
source types, each corresponding to a piece of information that the different
texture operations require.

.. c:autoenum:: nir_tex_src_type
   :members:

Of particular interest are the texture/sampler deref/index/handle source types.
First, note that textures and samplers are specified separately in NIR.  While
not required for OpenGL, this is required for Vulkan and OpenCL.  Some
OpenGL [ES] drivers have to deal with hardware that does not have separate
samplers and textures.  While not recommended, an OpenGL-only driver may assume
that the texture and sampler derefs will always point to the same resource, if
needed.  Note that this pretty well paints your compiler into a corner and
makes any future port to Vulkan or OpenCL harder, so such assumptions should
really only be made if targeting OpenGL ES 2.0 era hardware.

Also, like a lot of other resources, there are multiple ways to represent a
texture in NIR. It can be referenced by a variable dereference, an index, or a
bindless handle. When using an index or a bindless handle, the texture type
information is generally not available.  To handle this, various information
from the type is redundantly stored in the :c:struct:`nir_tex_instr` itself.

.. c:autostruct:: nir_tex_instr
   :members:

.. c:autostruct:: nir_tex_src
   :members:

Texture instruction helpers
---------------------------

There are a number of helper functions for working with NIR texture
instructions.  They are documented here in no particular order.

.. c:autofunction:: nir_tex_instr_create

.. c:autofunction:: nir_tex_instr_need_sampler

.. c:autofunction:: nir_tex_instr_result_size

.. c:autofunction:: nir_tex_instr_dest_size

.. c:autofunction:: nir_tex_instr_is_query

.. c:autofunction:: nir_tex_instr_has_implicit_derivative

.. c:autofunction:: nir_tex_instr_src_type

.. c:autofunction:: nir_tex_instr_src_size

.. c:autofunction:: nir_tex_instr_src_index

.. c:autofunction:: nir_tex_instr_add_src

.. c:autofunction:: nir_tex_instr_remove_src

Texture instruction lowering
----------------------------

Because most hardware only supports some subset of all possible GLSL/SPIR-V
texture operations, NIR provides a quite powerful lowering pass which is able
to implement more complex texture operations in terms of simpler ones.

.. c:autofunction:: nir_lower_tex

.. c:autostruct:: nir_lower_tex_options
   :members:

.. c:autoenum:: nir_lower_tex_packing
   :members:
