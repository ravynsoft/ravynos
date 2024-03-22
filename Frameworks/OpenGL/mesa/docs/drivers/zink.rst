Zink
====

Overview
--------

The Zink driver is a Gallium driver that emits Vulkan API calls instead
of targeting a specific GPU architecture. This can be used to get full
desktop OpenGL support on devices that only support Vulkan.

Features
--------

The feature-level of Zink depends on two things; what's implemented in Zink,
as well as the capabilities of the Vulkan driver.

The feature-levels implemented by Zink are exposed by `Vulkan Profiles
<https://dev.vulkan.org/tools#vulkan-profiles>`__ in the
:file:`VP_ZINK_requirements.json` profiles file.

Used with the `Vulkan Profiles tools <https://github.com/KhronosGroup/Vulkan-Profiles>`__,
we can compare the ZINK profiles with Vulkan devices profiles generated with
`Vulkaninfo <https://vulkan.lunarg.com/doc/view/latest/windows/vulkaninfo.html>`__
or `downloaded from GPUinfo.org`_
to establish the feature-levels supported by these drivers.

OpenGL 2.1
^^^^^^^^^^

OpenGL 2.1 is the minimum version Zink can support, and will always be
exposed, given Vulkan support. There's a few features that are required
for correct behavior, but not all of these are validated; instead you'll
see rendering-issues and likely validation error, or even crashes.

Here's a list of those requirements:

* Vulkan 1.0
* ``VkPhysicalDeviceFeatures``:

  * :vk-feat:`logicOp`
  * :vk-feat:`fillModeNonSolid`
  * :vk-feat:`alphaToOne`
  * :vk-feat:`shaderClipDistance`

* Device extensions:

  * :ext:`VK_KHR_maintenance1`
  * :ext:`VK_KHR_create_renderpass2`
  * :ext:`VK_KHR_imageless_framebuffer`
  * :ext:`VK_KHR_timeline_semaphore`
  * :ext:`VK_EXT_custom_border_color` with ``customBorderColorWithoutFormat``
  * :ext:`VK_EXT_provoking_vertex`
  * :ext:`VK_EXT_line_rasterization`, with the following ``VkPhysicalDeviceLineRasterizationFeaturesEXT``:

    * :vk-feat:`rectangularLines`
    * :vk-feat:`bresenhamLines`
    * :vk-feat:`smoothLines`
    * :vk-feat:`stippledRectangularLines`
    * :vk-feat:`stippledBresenhamLines`
    * :vk-feat:`stippledSmoothLines`

  * :ext:`VK_KHR_swapchain_mutable_format`
  * :ext:`VK_EXT_border_color_swizzle`
  * :ext:`VK_KHR_descriptor_update_template`

In addition to this, :ext:`VK_KHR_external_memory` is required to support the
DRI code-path.

We also require either the :ext:`VK_EXT_scalar_block_layout` extension or
Vulkan 1.2, with the :vk-feat:`scalarBlockLayout` feature.

OpenGL 3.0
^^^^^^^^^^


For OpenGL 3.0 support, the following additional requirements must be
met:

* ``VkPhysicalDeviceFeatures``:

  * :vk-feat:`independentBlend`

* Device extensions:

  * :ext:`VK_EXT_transform_feedback`
  * :ext:`VK_EXT_conditional_rendering`

OpenGL 3.1
^^^^^^^^^^

For OpenGL 3.1 support, the following additional ``VkPhysicalDeviceLimits``
are required:

* ``maxPerStageDescriptorSamplers`` ≥ 16

OpenGL 3.2
^^^^^^^^^^

For OpenGL 3.2 support, the following additional requirements must be
met, although some of these might not actually get verified:

* ``VkPhysicalDeviceFeatures``:

  * :vk-feat:`depthClamp`
  * :vk-feat:`geometryShader`
  * :vk-feat:`shaderTessellationAndGeometryPointSize`

* Device extensions:

  * :ext:`VK_EXT_depth_clip_enable`

OpenGL 3.3
^^^^^^^^^^

For OpenGL 3.3 support, the following additional requirements must be
met, although some of these might not actually get verified:

* ``VkPhysicalDeviceFeatures``:

  * :vk-feat:`dualSrcBlend`

* Device extensions:

  * :ext:`VK_EXT_vertex_attribute_divisor`

OpenGL 4.0
^^^^^^^^^^

For OpenGL 4.0 support, the following additional requirements must be
met:

* ``VkPhysicalDeviceFeatures``:

  * :vk-feat:`sampleRateShading`
  * :vk-feat:`tessellationShader`
  * :vk-feat:`imageCubeArray`

* Device extensions:

  * :ext:`VK_KHR_maintenance2`

* Formats requiring ``VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT``:

      * ``VK_FORMAT_R32G32B32_SFLOAT``
      * ``VK_FORMAT_R32G32B32_SINT``
      * ``VK_FORMAT_R32G32B32_UINT``

OpenGL 4.1
^^^^^^^^^^

For OpenGL 4.1 support, the following additional requirements must be
met:

* ``VkPhysicalDeviceFeatures``:

  * :vk-feat:`multiViewport`

* ``VkPhysicalDeviceLimits``

  * ``maxImageDimension1D`` ≥ 16384
  * ``maxImageDimension2D`` ≥ 16384
  * ``maxImageDimension3D`` ≥ 2048
  * ``maxImageDimensionCube`` ≥ 16384
  * ``maxImageArrayLayers`` ≥ 2048
  * ``maxViewports`` ≥ 16

OpenGL 4.2
^^^^^^^^^^

For OpenGL 4.2 support, the following additional requirements must be
met:

* Device extensions:
    * :ext:`VK_EXT_image_2d_view_of_3d`

* ``VkPhysicalDeviceLimits``:

  * ``shaderStorageImageExtendedFormats``
  * ``shaderStorageImageWriteWithoutFormat``
  * ``vertexPipelineStoresAndAtomics``
  * ``fragmentStoresAndAtomics``

* For Vulkan 1.2 and above:

  * ``VkPhysicalDeviceVulkan11Features``:

    * :vk-feat:`shaderDrawParameters`

* For Vulkan 1.1 and below:

  * Device extensions:

    * :ext:`VK_KHR_shader_draw_parameters`

OpenGL 4.3
^^^^^^^^^^

For OpenGL 4.3 support, the following additional requirements must be
met:

* ``VkPhysicalDeviceFeatures``:

  * :vk-feat:`robustBufferAccess`

* Formats requiring ``VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT``:

   * ``VK_FORMAT_R8G8B8A8_UNORM``
   * ``VK_FORMAT_R8G8B8A8_SRGB``
   * ``VK_FORMAT_R16_UNORM``
   * ``VK_FORMAT_R16G16_UNORM``
   * ``VK_FORMAT_R16_SNORM``
   * ``VK_FORMAT_R16G16_SNORM``
   * ``VK_FORMAT_D32_SFLOAT_S8_UINT``

OpenGL 4.4
^^^^^^^^^^

For OpenGL 4.4 support, the following additional requirements must be
met:

* Formats requiring ``VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT``:

  * ``VK_FORMAT_B10G11R11_UFLOAT_PACK32``

* For Vulkan 1.2 and above:

  * ``VkPhysicalDeviceVulkan12Features``:

    * ``samplerMirrorClampToEdge``

* For Vulkan 1.1 and below:

  * Device extensions:

    * :ext:`VK_KHR_sampler_mirror_clamp_to_edge`

OpenGL 4.5
^^^^^^^^^^

For OpenGL 4.5 support, the following additional ``VkPhysicalDeviceFeatures``
are required to be supported

* :vk-feat:`shaderCullDistance`

OpenGL 4.6
^^^^^^^^^^

For OpenGL 4.6 support, the following additional requirements must be
met:

* ``VkPhysicalDeviceFeatures``:

  * :vk-feat:`samplerAnisotropy`
  * :vk-feat:`depthBiasClamp`

* Device extensions:

  * :ext:`VK_KHR_draw_indirect_count`

Performance
-----------

If you notice poor performance and high CPU usage while running an application,
changing the descriptor manager may improve performance:

.. envvar:: ZINK_DESCRIPTORS <mode> ("auto")

``auto``
   Automatically detect best mode. This is the default.
``lazy``
   Attempt to use the least amount of CPU by binding descriptors opportunistically.
``db``
   Use EXT_descriptor_buffer when possible.

Debugging
---------

There's a few tools that are useful for debugging Zink, like this environment
variable:

.. envvar:: ZINK_DEBUG

  Accepts the following comma-separated list of flags:

  ``nir``
    Print the NIR form of all shaders to stderr.
  ``spirv``
    Write the binary SPIR-V form of all compiled shaders to a file in the
    current directory, and print a message with the filename to stderr.
  ``tgsi``
    Print the TGSI form of TGSI shaders to stderr.
  ``validation``
    Dump Validation layer output.
  ``sync``
    Emit full synchronization barriers before every draw and dispatch.
  ``compact``
    Use a maximum of 4 descriptor sets
  ``noreorder``
    Do not reorder or optimize GL command streams
  ``gpl``
    Force using Graphics Pipeline Library for all shaders
  ``rp``
    Enable renderpass optimizations (for tiling GPUs)
  ``norp``
    Disable renderpass optimizations (for tiling GPUs)
  ``map``
    Print info about mapped VRAM
  ``flushsync``
    Force synchronous flushes/presents
  ``noshobj``
    Disable EXT_shader_object
  ``optimal_keys``
    Debug/use optimal_keys
  ``noopt``
    Disable async optimized pipeline compiles
  ``nobgc``
    Disable all async pipeline compiles
  ``mem``
    Enable memory allocation debugging
  ``quiet``
    Suppress probably-harmless warnings

Vulkan Validation Layers
^^^^^^^^^^^^^^^^^^^^^^^^

Another useful tool for debugging is the `Vulkan Validation Layers
<https://github.com/KhronosGroup/Vulkan-ValidationLayers/blob/main/README.md>`__.

The validation layers effectively insert extra checking between Zink and the
Vulkan driver, pointing out incorrect usage of the Vulkan API. The layers can
be enabled by setting the environment variable :envvar:`VK_INSTANCE_LAYERS` to
"VK_LAYER_KHRONOS_validation". You can read more about the Validation Layers
in the link above.

IRC
---

In order to make things a bit easier to follow, we have decided to create our
own IRC channel. If you're interested in contributing, or have any technical
questions, don't hesitate to visit `#zink on OFTC
<irc://irc.oftc.net/zink>`__ and say hi!


.. _downloaded from GPUinfo.org: https://www.saschawillems.de/blog/2022/03/12/vulkan-profiles-support-for-the-vulkan-hardware-capability-viewer-and-database/
