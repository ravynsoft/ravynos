Graphics state
==============

The Mesa Vulkan runtime provides helpers for managing the numerous pieces
of graphics state associated with a ``VkPipeline`` or set dynamically on a
command buffer.  No such helpers are provided for compute or ray-tracing
because they have little or no state besides the shaders themselves.


Pipeline state
--------------

All (possibly dynamic) Vulkan graphics pipeline state is encapsulated into
a single :c:struct:`vk_graphics_pipeline_state` structure which contains
pointers to sub-structures for each of the different state categories.
Unlike :c:type:`VkGraphicsPipelineCreateInfo`, the pointers in
:c:struct:`vk_graphics_pipeline_state` are guaranteed to be either be
NULL or point to valid and properly populated memory.

When creating a pipeline, the
:c:func:`vk_graphics_pipeline_state_fill()` function can be used to
gather all of the state from the core structures as well as various ``pNext``
chains into a single state structure.  Whenever an extension struct is
missing, a reasonable default value is provided whenever possible.


:c:func:`vk_graphics_pipeline_state_fill()` automatically handles both
the render pass and dynamic rendering.  For drivers which use
:c:struct:`vk_render_pass`, the :c:struct:`vk_render_pass_state`
structure will be populated as if for dynamic rendering, regardless of
which path is used.  Drivers which use their own render pass structure
should parse the render pass, if available, and pass a
:c:struct:`vk_render_pass_state` to the `driver_rp` argument of
:c:func:`vk_graphics_pipeline_state_fill()` with the relevant information
from the specified subpass.  If a render pass is available,
:c:struct:`vk_render_pass_state` will be populated with the
the information from the :c:struct:`driver_rp`.  If dynamic
rendering is used or the driver provides a `NULL`
:c:struct:`driver_rp`, the :c:struct:`vk_render_pass_state`
structure will be populated for dynamic rendering, including color, depth,
and stencil attachment formats.

The usual flow for creating a full graphics pipeline (not library) looks
like this:

.. code-block:: c

   struct vk_graphics_pipeline_state state = { };
   struct vk_graphics_pipeline_all_state all;
   vk_graphics_pipeline_state_fill(&device->vk, &state, pCreateInfo,
                                   NULL, &all, NULL, 0, NULL);

   /* Emit stuff using the state in `state` */

The :c:struct:`vk_graphics_pipeline_all_state` structure exists to allow
the state to sit on the stack instead of requiring a heap allocation.  This
is useful if you intend to use the state right away and don't need to store
it.  For pipeline libraries, it's likely more useful to use the dynamically
allocated version and store the dynamically allocated memory in the
library pipeline.

.. code-block:: c

   /* Assuming we have a vk_graphics_pipeline_state in pipeline */
   memset(&pipeline->state, 0, sizeof(pipeline->state));

   for (uint32_t i = 0; i < lib_info->libraryCount; i++) {
      VK_FROM_HANDLE(drv_graphics_pipeline_library, lib, lib_info->pLibraries[i]);
      vk_graphics_pipeline_state_merge(&pipeline->state, &lib->state);
   }

   /* This assumes you have a void **state_mem in pipeline */
   result = vk_graphics_pipeline_state_fill(&device->vk, &pipeline->state,
                                            pCreateInfo, NULL, NULL, pAllocator,
                                            VK_SYSTEM_ALLOCATION_SCOPE_OBJECT,
                                            &pipeline->state_mem);
   if (result != VK_SUCCESS)
      return result;

State from dependent libraries can be merged together using
:c:func:`vk_graphics_pipeline_state_merge`.
:c:func:`vk_graphics_pipeline_state_fill` will then only attempt to
populate missing fields.  You can also merge dependent pipeline libraries
together but store the final state on the stack for immediate consumption:

.. code-block:: c

   struct vk_graphics_pipeline_state state = { };

   for (uint32_t i = 0; i < lib_info->libraryCount; i++) {
      VK_FROM_HANDLE(drv_graphics_pipeline_library, lib, lib_info->pLibraries[i]);
      vk_graphics_pipeline_state_merge(&state, &lib->state);
   }

   struct vk_graphics_pipeline_all_state all;
   vk_graphics_pipeline_state_fill(&device->vk, &state, pCreateInfo,
                                   NULL, &all, NULL, 0, NULL);

.. c:autofunction:: vk_graphics_pipeline_state_fill
   :file: src/vulkan/runtime/vk_graphics_state.h

.. c:autofunction:: vk_graphics_pipeline_state_merge
   :file: src/vulkan/runtime/vk_graphics_state.h


Dynamic state
-------------

All dynamic states in Vulkan, regardless of which API version or extension
introduced them, are represented by the
:c:enum:`mesa_vk_dynamic_graphics_state` enum.  This corresponds to the
:c:type:`VkDynamicState` enum in the Vulkan API only it's compact (has no
holes due to extension namespacing) and a bit better organized.  Each
enumerant is named with the name of the state group to which the dynamic
state belongs as well as the name of the dynamic state itself.  The fact
that it's compact allows us to use to index bitsets.

.. c:autofunction:: vk_get_dynamic_graphics_states
   :file: src/vulkan/runtime/vk_graphics_state.h

We also provide a :c:struct:`vk_dynamic_graphics_state` structure which
contains all the dynamic graphics states, regardless of which API version
or extension introduced them.  This structure can be populated from a
:c:struct:`vk_graphics_pipeline_state` via
:c:func:`vk_dynamic_graphics_state_init`.

.. c:autofunction:: vk_dynamic_graphics_state_init
   :file: src/vulkan/runtime/vk_graphics_state.h

.. c:autofunction:: vk_dynamic_graphics_state_copy
   :file: src/vulkan/runtime/vk_graphics_state.h

There is also a :c:struct:`vk_dynamic_graphics_state` embedded in
:c:struct:`vk_command_buffer`.  Should you choose to use them, we provide
common implementations for all ``vkCmdSet*()`` functions.  Two additional
functions are provided for the driver to call in ``CmdBindPipeline()`` and
``CmdBindVertexBuffers2()``:

.. c:autofunction:: vk_cmd_set_dynamic_graphics_state
   :file: src/vulkan/runtime/vk_graphics_state.h

.. c:autofunction:: vk_cmd_set_vertex_binding_strides
   :file: src/vulkan/runtime/vk_graphics_state.h

To use the dynamic state framework, you will need the following in your
pipeline structure:

.. code-block:: c

   struct drv_graphics_pipeline {
      ....
      struct vk_vertex_input_state vi_state;
      struct vk_sample_locations_state sl_state;
      struct vk_dynamic_graphics_state dynamic;
      ...
   };

Then, in your pipeline create function,

.. code-block:: c

   memset(&pipeline->dynamic, 0, sizeof(pipeline->dynamic));
   pipeline->dynamic->vi = &pipeline->vi_state;
   pipeline->dynamic->ms.sample_locations = &pipeline->sl_state;
   vk_dynamic_graphics_state_init(&pipeline->dynamic, &state);

In your implementation of ``vkCmdBindPipeline()``,

.. code-block:: c

   vk_cmd_set_dynamic_graphics_state(&cmd->vk, &pipeline->dynamic_state);

And, finally, at ``vkCmdDraw*()`` time, the code to emit dynamic state into
your hardware command buffer will look something like this:

.. code-block:: c

   static void
   emit_dynamic_state(struct drv_cmd_buffer *cmd)
   {
      struct vk_dynamic_graphics_state *dyn = &cmd->vk.dynamic_graphics_state;

      if (!vk_dynamic_graphics_state_any_dirty(dyn))
         return;

      if (BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_VP_VIEWPORTS) |
          BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_VP_VIEWPORT_COUNT)) {
         /* Re-emit viewports */
      }

      if (BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_VP_SCISSORS) |
          BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_VP_SCISSOR_COUNT)) {
         /* Re-emit scissors */
      }

      /* etc... */

      vk_dynamic_graphics_state_clear_dirty(dyn);
   }

Any states used by the currently bound pipeline and attachments are always
valid in ``vk_command_buffer::dynamic_graphics_state`` so you can always
use a state even if it isn't dirty on this particular draw.

.. c:autofunction:: vk_dynamic_graphics_state_dirty_all
   :file: src/vulkan/runtime/vk_graphics_state.h

.. c:autofunction:: vk_dynamic_graphics_state_clear_dirty
   :file: src/vulkan/runtime/vk_graphics_state.h

.. c:autofunction:: vk_dynamic_graphics_state_any_dirty
   :file: src/vulkan/runtime/vk_graphics_state.h


Depth stencil state optimization
--------------------------------

.. c:autofunction:: vk_optimize_depth_stencil_state
   :file: src/vulkan/runtime/vk_graphics_state.h


Reference
---------

.. c:autostruct:: vk_graphics_pipeline_state
   :file: src/vulkan/runtime/vk_graphics_state.h
   :members:

.. c:autostruct:: vk_vertex_binding_state
   :file: src/vulkan/runtime/vk_graphics_state.h
   :members:

.. c:autostruct:: vk_vertex_attribute_state
   :file: src/vulkan/runtime/vk_graphics_state.h
   :members:

.. c:autostruct:: vk_vertex_input_state
   :file: src/vulkan/runtime/vk_graphics_state.h
   :members:

.. c:autostruct:: vk_input_assembly_state
   :file: src/vulkan/runtime/vk_graphics_state.h
   :members:

.. c:autostruct:: vk_tessellation_state
   :file: src/vulkan/runtime/vk_graphics_state.h
   :members:

.. c:autostruct:: vk_viewport_state
   :file: src/vulkan/runtime/vk_graphics_state.h
   :members:

.. c:autostruct:: vk_discard_rectangles_state
   :file: src/vulkan/runtime/vk_graphics_state.h
   :members:

.. c:autostruct:: vk_rasterization_state
   :file: src/vulkan/runtime/vk_graphics_state.h
   :members:

.. c:autostruct:: vk_fragment_shading_rate_state
   :file: src/vulkan/runtime/vk_graphics_state.h
   :members:

.. c:autostruct:: vk_sample_locations_state
   :file: src/vulkan/runtime/vk_graphics_state.h
   :members:

.. c:autostruct:: vk_multisample_state
   :file: src/vulkan/runtime/vk_graphics_state.h
   :members:

.. c:autostruct:: vk_stencil_test_face_state
   :file: src/vulkan/runtime/vk_graphics_state.h
   :members:

.. c:autostruct:: vk_depth_stencil_state
   :file: src/vulkan/runtime/vk_graphics_state.h
   :members:

.. c:autostruct:: vk_color_blend_state
   :file: src/vulkan/runtime/vk_graphics_state.h
   :members:

.. c:autostruct:: vk_render_pass_state
   :file: src/vulkan/runtime/vk_graphics_state.h
   :members:

.. c:autoenum:: mesa_vk_dynamic_graphics_state
   :file: src/vulkan/runtime/vk_graphics_state.h

.. c:autostruct:: vk_dynamic_graphics_state
   :file: src/vulkan/runtime/vk_graphics_state.h
   :members:
