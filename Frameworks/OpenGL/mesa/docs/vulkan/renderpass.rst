Render Passes
=============

The Vulkan runtime code in Mesa provides several helpful utilities to make
managing render passes easier.


:ext:`VK_KHR_create_renderpass2`
--------------------------------

It is strongly recommended that drivers implement
:ext:`VK_KHR_create_renderpass2` directly and not bother implementing the
old Vulkan 1.0 entrypoints.  If a driver does not implement them, the
following will be implemented in common code in terms of their
:ext:`VK_KHR_create_renderpass2` counterparts:

 - :c:func:`vkCreateRenderPass`
 - :c:func:`vkCmdBeginRenderPass`
 - :c:func:`vkCmdNextSubpass`
 - :c:func:`vkCmdEndRenderPass`


Common VkRenderPass implementation
----------------------------------

The Vulkan runtime code in Mesa provides a common implementation of
:c:type:`VkRenderPass` called :c:struct:`vk_render_pass` which drivers
can optionally use.  Unlike most Vulkan runtime structs, it's not really
designed to be used as a base for a driver-specific struct.  It does,
however, contain all the information passed to
:c:func:`vkCreateRenderPass2` so it can be used in a driver so long as
that driver doesn't need to do any additional compilation at
:c:func:`vkCreateRenderPass2` time.  If a driver chooses to use
:c:struct:`vk_render_pass`, the Vulkan runtime provides implementations
of :c:func:`vkCreateRenderPass2` and :c:func:`vkDestroyRenderPass`.


:ext:`VK_KHR_dynamic_rendering`
-------------------------------

For drivers which don't need to do subpass combining, it is recommended
that they skip implementing render passes entirely and implement
:ext:`VK_KHR_dynamic_rendering` instead.  If they choose to do so, the runtime
will provide the following, implemented in terms of
:c:func:`vkCmdBeginRendering` and :c:func:`vkCmdEndRendering`:

 - :c:func:`vkCmdBeginRenderPass2`
 - :c:func:`vkCmdNextSubpass2`
 - :c:func:`vkCmdEndRenderPass2`

We also provide a no-op implementation of
:c:func:`vkGetRenderAreaGranularity` which returns a render area
granularity of 1x1.

Drivers which wish to use the common render pass implementation in this way
**must** also support a Mesa-specific pseudo-extension which optionally
provides an initial image layout for each attachment at
:c:func:`vkCmdBeginRendering` time.  This is required for us to combine
render pass clears with layout transitions, often from
:c:enum:`VK_IMAGE_LAYOUT_UNDEFINED`.  On at least Intel and AMD,
combining these transitions with clears is important for performance.

.. c:autostruct:: VkRenderingAttachmentInitialLayoutInfoMESA
   :file: src/vulkan/runtime/vk_render_pass.h
   :members:

Because render passes and subpass indices are also passed into
:c:func:`vkCmdCreateGraphicsPipelines` and
:c:func:`vkCmdExecuteCommands` which we can't implement on the driver's
behalf, we provide a couple of helpers for getting the render pass
information in terms of the relevant :ext:`VK_KHR_dynamic_rendering`:

.. c:autofunction:: vk_get_pipeline_rendering_create_info
   :file: src/vulkan/runtime/vk_render_pass.h

.. c:autofunction:: vk_get_command_buffer_inheritance_rendering_info
   :file: src/vulkan/runtime/vk_render_pass.h

Apart from handling layout transitions, the common render pass
implementation mostly ignores input attachments.  It is expected that the
driver call :c:func:`nir_lower_input_attachments` to turn them into
texturing operations.  The driver **must** support texturing from an input
attachment at the same time as rendering to it in order to support Vulkan
subpass self-dependencies. ``VK_EXT_attachment_feedback_loop_layout`` provides
information on when these self dependencies are present.

vk_render_pass reference
------------------------

The following is a reference for the :c:struct:`vk_render_pass` structure
and its substructures.

.. c:autostruct:: vk_render_pass
   :file: src/vulkan/runtime/vk_render_pass.h
   :members:

.. c:autostruct:: vk_render_pass_attachment
   :file: src/vulkan/runtime/vk_render_pass.h
   :members:

.. c:autostruct:: vk_subpass
   :file: src/vulkan/runtime/vk_render_pass.h
   :members:

.. c:autostruct:: vk_subpass_attachment
   :file: src/vulkan/runtime/vk_render_pass.h
   :members:

.. c:autostruct:: vk_subpass_dependency
   :file: src/vulkan/runtime/vk_render_pass.h
   :members:
