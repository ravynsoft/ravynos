Introduction
============

The Mesa project began as an open-source implementation of the
`OpenGL`_ specification - a system for rendering interactive 3D graphics.

Over the years the project has grown to implement more graphics APIs,
including `OpenGL ES`_, `OpenCL`_, `OpenMAX`_, `VDPAU`_, `VA-API`_,
`Vulkan`_ and `EGL`_.

A variety of device drivers allows the Mesa libraries to be used in many
different environments ranging from software emulation to complete
hardware acceleration for modern GPUs.

Mesa ties into several other open-source projects: the `Direct Rendering
Infrastructure`_, `X.org`_, and `Wayland`_ to provide OpenGL support on
Linux, FreeBSD, and other operating systems.

.. _OpenGL: https://www.opengl.org/
.. _OpenGL ES: https://www.khronos.org/opengles/
.. _OpenCL: https://www.khronos.org/opencl/
.. _OpenMAX: https://www.khronos.org/openmax/
.. _VDPAU: https://en.wikipedia.org/wiki/VDPAU
.. _VA-API: https://en.wikipedia.org/wiki/Video_Acceleration_API
.. _Vulkan: https://www.vulkan.org/
.. _EGL: https://www.khronos.org/egl/
.. _Direct Rendering Infrastructure: https://dri.freedesktop.org/
.. _X.org: https://x.org
.. _Wayland: https://wayland.freedesktop.org

.. toctree::
   :maxdepth: 1
   :caption: Documentation
   :hidden:

   self
   history
   amber
   systems
   license
   faq
   relnotes

.. toctree::
   :maxdepth: 2
   :caption: Download and Install
   :hidden:

   download
   install
   precompiled

.. toctree::
   :maxdepth: 1
   :caption: Need help?
   :hidden:

   lists
   bugs

.. toctree::
   :maxdepth: 1
   :caption: User Topics
   :hidden:

   shading
   egl
   opengles
   envvars
   osmesa
   debugging
   perf
   gpu-perf-tracing
   extensions
   application-issues
   gallium-nine
   viewperf
   xlibdriver

.. toctree::
   :maxdepth: 1
   :caption: Drivers
   :hidden:

   drivers/anv
   drivers/asahi
   drivers/d3d12
   drivers/freedreno
   drivers/lima
   drivers/llvmpipe
   drivers/nvk
   drivers/panfrost
   drivers/powervr
   drivers/radv
   drivers/svga3d
   drivers/v3d
   drivers/vc4
   drivers/venus
   drivers/virgl
   drivers/zink

.. toctree::
   :maxdepth: 1
   :caption: Developer Topics
   :hidden:

   repository
   sourcetree
   utilities
   helpwanted
   devinfo
   codingstyle
   submittingpatches
   releasing
   release-calendar
   dispatch
   gallium/index
   vulkan/index
   nir/index
   isl/index
   isaspec
   rusticl
   android
   macos
   Linux Kernel Drivers <https://www.kernel.org/doc/html/latest/gpu/>

.. toctree::
   :maxdepth: 1
   :caption: Testing
   :hidden:

   conform
   ci/index

.. toctree::
   :maxdepth: 1
   :caption: Links
   :hidden:

   OpenGL Website <https://www.opengl.org>
   DRI Website <https://dri.freedesktop.org>
   Developer Blogs <https://planet.freedesktop.org>
