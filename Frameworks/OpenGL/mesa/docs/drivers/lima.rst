Lima
====

Lima is an open source graphics driver which supports Mali Utgard
(Mali-4xx) embedded GPUs from ARM. It’s a reverse-engineered,
community-developed driver, and is not endorsed by ARM. Lima was
upstreamed in Mesa 19.1 and Linux kernel 5.2.

========  ============ ===========
Product   Architecture   Status
========  ============ ===========
Mali-400     Utgard     Supported
Mali-450     Utgard     Supported
Mali-470     Utgard    Unsupported
========  ============ ===========

Newer Mali chips based on the Midgard/Bifrost architectures (Mali T or G
series) are handled by the :doc:`Panfrost <panfrost>` driver, not Lima.

Note that the Mali GPU is only for rendering: the GPU does not control a
display and has little to do with display-related issues.
Each SoC has its own separate display engine to control the display
output. To display the contents rendered by the Mali GPU to a screen, a
separate `display driver <#display-drivers>`__ is also required, which
is able to share buffers with the GPU. In Mesa, this is handled by
``kmsro``.

Supported APIs
--------------

Lima mainly targets **OpenGL ES 2.0**, as well as **OpenGL 2.1**
(desktop) to some extent.

The OpenGL (desktop) implementation is enabled by Mesa and Gallium,
where it is possible to reuse the same implementation backend. That way,
it is possible to support running a majority of Linux desktop
applications designed for OpenGL. It is not possible to fully support
OpenGL (desktop), though, due to hardware limitations. Some (but not
all) features of OpenGL 2.1 that are not supported directly in hardware
are enabled by internal shader transformations.
Check the `known hardware limitations <#known-hardware-limitations>`__
list for additional information.

**OpenGL ES 1.1** and **OpenGL 1.x** are also provided by Mesa and
similarly supported to some extent in Lima.

Display drivers
---------------

These are some display drivers that have been tested with Lima:

- Allwinner: ``sun4i-drm``
- Amlogic: ``meson``
- Ericsson MCDE: ``mcde``
- Exynos: ``exynos``
- Rockchip: ``rockchip``
- Tiny DRM: ``tinydrm``

Environment variables
---------------------

These are some Lima-specific environment variables that may aid in
debugging. None of this is required for normal use.

.. envvar:: LIMA_DEBUG

  accepts the following comma-separated list of flags:

  ``bocache``
    print debug info for BO cache
  ``diskcache``
    print debug info for shader disk cache
  ``dump``
    dump GPU command stream to ``$PWD/lima.dump``
  ``gp``
    print GP shader compiler result of each stage
  ``noblit``
    use generic u_blitter instead of Lima-specific
  ``nobocache``
    disable BO cache
  ``nogrowheap``
    disable growable heap buffer
  ``notiling``
    don’t use tiled buffers
  ``pp``
    print PP shader compiler result of each stage
  ``precompile``
    precompile shaders for shader-db
  ``shaderdb``
    print shader information for shaderdb
  ``singlejob``
    disable multi job optimization


.. envvar:: LIMA_CTX_NUM_PLB

  set number of PLB per context (used for development purposes)

.. envvar:: LIMA_PLB_MAX_BLK

  set PLB max block (used for development purposes)

.. envvar:: LIMA_PPIR_FORCE_SPILLING

  force spilling of variables in PPIR (used for development purposes)

.. envvar:: LIMA_PLB_PP_STREAM_CACHE_SIZE

  set PP stream cache size (used for development purposes)

Known hardware limitations
--------------------------

Here are some known caveats in OpenGL support:

- ``glPolygonMode()`` with ``GL_LINE`` is not supported. This is not part of
  OpenGL ES 2.0 and so it is not possible to reverse engineer.

- Precision limitations in fragment shaders:

  - In general, only
    `FP16 <https://en.wikipedia.org/wiki/Half-precision_floating-point_format>`__
    precision is supported in fragment shaders. Specifying ``highp``
    will have no effect.
  - Integers are not supported in hardware, they are lowered down to
    FP16.
  - There is a higher precision (FP24) path for texture lookups, if
    there is *no* math performed on texture coordinates obtained from
    varyings. If there is *any* calculation done in the texture
    coordinates, the texture coordinates will fall back to FP16 and
    that may affect the quality of the texture lookup.

- Lima supports FP16 textures in OpenGL ES (through
  :ext:`GL_OES_texture_half_float<GL_OES_texture_float>`), but not in OpenGL.
  This is because it would require :ext:`GL_ARB_texture_float` which would
  also require 32-bit float textures, that the Mali-4xx does not support.
- Rendering to FP16 is possible, but the result is clamped to the
  [0.0,1.0] range.

Bug Reporting
-------------

Please try the latest Mesa development branch or at least Mesa latest
release before reporting issues. Please review the
:doc:`Mesa bug report guidelines <../bugs>`.

Issues should be filed as a `Mesa issue`_.
Lima tags will be added accordingly by the developers.

`apitrace <https://github.com/apitrace/apitrace>`__ traces are very
welcome in issue reports and significantly ease the debug and fix
process.

FAQ
---

Will Lima support OpenGL 3.x+ / OpenGL ES 3.x+ / OpenCL / Vulkan ?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**No.** The Mali-4xx was designed to implement OpenGL ES 2.0 and OpenGL
ES 1.1. The hardware lacks features to properly implement some features
required by newer APIs.

How complete is Lima? Is reverse engineering complete?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

At the time of writing, with local runs of the
`OpenGL ES Conformance Tests <https://github.com/KhronosGroup/VK-GL-CTS/>`__
(dEQP) for OpenGL ES 2.0, Lima reports **97%** pass rate.
This coverage is on par with coverage provided by the ARM Mali driver.
Some tests that pass with Lima fail on Mali and vice versa. Some of
these issues are related to precision limitations which likely don’t
affect end user applications.

The work being done in Lima at this stage is largely decoupled from
reverse engineering. Reverse engineering is still useful sometimes to
obtain details on how to implement low level features (e.g. how to
enable some missing legacy OpenGL ES 1.1 feature to support an
additional application), but with the current information Lima is
already able to cover most of OpenGL ES 2.0.

Much of the work to be done is related to plumbing features within the
frameworks provided by Mesa, fixing bugs (e.g. artifacts or crashes in
specific applications), shader compiler improvements, which are not
necessarily related to new hardware bits and not related at all to the
Mali driver.

When will Feature XYZ be supported? Is there a roadmap for features implementation?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

There is no established roadmap for features implementation.
Development is driven by improving coverage in existing OpenGL test
frameworks, adding support to features that enable more existing Linux
applications, and fixing issues reported by users in their applications.
Development is fully based on community contributions.

If some desired feature is missing or there is an OpenGL-related bug
while running some application, please do file a `Mesa issue`_.
Issues that are not reproduced by an existing test suite or common
application and are also not reported by users are just likely not going
to be noticed and fixed.

How does Lima compare to Mali (blob)? How is performance?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

By the fact that Lima is a fully open source driver and leverages a lot
of Mesa and Linux functionality, feature-wise Lima is able to support
many things that Mali does not. As already mentioned, supporting OpenGL
2.1 is one of them. This allows Lima to support many more Linux desktop
applications out of the box. Through the abstractions implemented in
Mesa, Lima supports a number of OpenGL and OpenGL ES extensions that
originally the Mali did not support. Lima is also aligned with the
current status of the Linux graphics stack and is therefore able to
leverage modern features (such as zero copy pipelines) much more
seamlessly. Finally, Lima continues to gain improvements as the Linux
graphics ecosystem evolves.

The entire software stack of the Mali driver and the software stack with
Lima are significantly different which makes it hard to offer a single
number comparison for performance of the GPU driver. The difference
really depends on the type of application. Keep in mind that hardware
containing a Mali-4xx is usually quite limited for modern standards and
it might not perform as well as hoped. For example: while it is now
technically possible to run full GL modern desktop environments at 1080p
(which might not have been even possible before due to limited GL
support), that might not be very performant due to memory bandwidth, CPU
and GPU limitations of the SoC with a Mali-4xx.

Overall performance with Lima is good for many applications where the
Mali-4xx would be a suitable target GPU.
But bottom line for a performance evaluation, you need to try with your
target application. If performance with Lima does not seem right in some
application where it should reasonably perform better, please file a
`Mesa issue`_ (in which case some indication on why Lima in particular
seems to be the bottleneck would also be helpful).

Communication channels
----------------------

- `#lima channel <irc://irc.oftc.net/lima>`__ on `irc.oftc.net <https://webchat.oftc.net/>`__
- `lima mailing list <https://lists.freedesktop.org/mailman/listinfo/lima>`__
- `dri-devel mailing list <https://lists.freedesktop.org/mailman/listinfo/dri-devel>`__

Dump tool
---------

A tool to dump the runtime of the closed source Mali driver for
reverse engineering is available at:
https://gitlab.freedesktop.org/lima/mali-syscall-tracker

Reference
---------

Luc Verhaegen’s original Lima site:
https://web.archive.org/web/20180101212947/http://limadriver.org/

.. _Mesa issue: https://gitlab.freedesktop.org/mesa/mesa/-/issues?label_name%5B%5D=lima
