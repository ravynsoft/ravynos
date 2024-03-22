EGL
===

The current version of EGL in Mesa implements EGL 1.4. More information
about EGL can be found at https://www.khronos.org/egl/.

The Mesa's implementation of EGL uses a driver architecture. The main
library (``libEGL``) is window system neutral. It provides the EGL API
entry points and helper functions for use by the drivers. Drivers are
dynamically loaded by the main library and most of the EGL API calls are
directly dispatched to the drivers.

The driver in use decides the window system to support.

Build EGL
---------

#. Configure your build with the desired client APIs and enable the
   driver for your hardware. For example:

   .. code-block:: console

      $ meson configure \
              -D egl=enabled \
              -D gles1=enabled \
              -D gles2=enabled \
              -D gallium-drivers=...

   The main library and OpenGL is enabled by default. The first two
   options above enables :doc:`OpenGL ES 1.x and 2.x <opengles>`. The
   last two options enables the listed classic and Gallium drivers
   respectively.

#. Build and install Mesa as usual.

In the given example, it will build and install ``libEGL``, ``libGL``,
``libGLESv1_CM``, ``libGLESv2``, and one or more EGL drivers.

Configure Options
~~~~~~~~~~~~~~~~~

There are several options that control the build of EGL at configuration
time

``-D egl=enabled``
   By default, EGL is enabled. When disabled, the main library and the
   drivers will not be built.

``-D platforms=...``
   List the platforms (window systems) to support. Its argument is a
   comma separated string such as ``-D platforms=x11,wayland``. It decides
   the platforms a driver may support. The first listed platform is also
   used by the main library to decide the native platform.

   The available platforms are ``x11``, ``wayland``,
   ``android``, and ``haiku``. The ``android`` platform
   can either be built as a system component, part of AOSP, using
   ``Android.mk`` files, or cross-compiled using appropriate options.
   Unless for special needs, the build system should select the right
   platforms automatically.

``-D gles1=enabled`` and ``-D gles2=enabled``
   These options enable OpenGL ES support in OpenGL. The result is one
   big internal library that supports multiple APIs.

``-D shared-glapi=enabled``
   By default, ``libGL`` has its own copy of ``libglapi``. This options
   makes ``libGL`` use the shared ``libglapi``. This is required if
   applications mix OpenGL and OpenGL ES.

Use EGL
-------

Demos
~~~~~

There are demos for the client APIs supported by EGL. They can be found
in mesa/demos repository.

Environment Variables
~~~~~~~~~~~~~~~~~~~~~

There are several environment variables that control the behavior of EGL
at runtime

``EGL_PLATFORM``
   This variable specifies the native platform. The valid values are the
   same as those for ``-D platforms=...``. When the variable is not set,
   the main library uses the first platform listed in
   ``-D platforms=...`` as the native platform.

``EGL_LOG_LEVEL``
   This changes the log level of the main library and the drivers. The
   valid values are: ``debug``, ``info``, ``warning``, and ``fatal``.

Packaging
---------

The ABI between the main library and its drivers are not stable. Nor is
there a plan to stabilize it at the moment.

Developers
----------

The sources of the main library and drivers can be found at
``src/egl/``.

The code basically consists of two things:

1. An EGL API dispatcher. This directly routes all the ``eglFooBar()``
   API calls into driver-specific functions.

2. Two EGL drivers (``dri2`` and ``haiku``), implementing the API
   functions handling the platforms' specifics.

Two of API functions are optional (``eglQuerySurface()`` and
``eglSwapInterval()``); the former provides fallback for all the
platform-agnostic attributes (i.e. everything except ``EGL_WIDTH``
& ``EGL_HEIGHT``), and the latter just silently pretends the API call
succeeded (as per EGL spec).

A driver _could_ implement all the other EGL API functions, but several of
them are only needed for extensions, like ``eglSwapBuffersWithDamageEXT()``.
See ``src/egl/main/egldriver.h`` to see which driver hooks are only
required by extensions.

Bootstrapping
~~~~~~~~~~~~~

When the apps calls ``eglInitialize()``, the driver's ``Initialize()``
function is called. If the first driver initialization attempt fails,
a second one is tried using only software components (this can be forced
using the ``LIBGL_ALWAYS_SOFTWARE`` environment variable). Typically,
this function takes care of setting up visual configs, creating EGL
devices, etc.

Teardown
~~~~~~~~

When ``eglTerminate()`` is called, the ``driver->Terminate()`` function
is called. The driver should clean up after itself.

Subclassing
~~~~~~~~~~~

The internal libEGL data structures such as ``_EGLDisplay``,
``_EGLContext``, ``_EGLSurface``, etc. should be considered base classes
from which drivers will derive subclasses.

EGL Drivers
-----------

``egl_dri2``
   This driver supports several platforms: ``android``, ``device``,
   ``drm``, ``surfaceless``, ``wayland`` and ``x11``. It functions as
   a DRI driver loader. For ``x11`` support, it talks to the X server
   directly using (XCB-)DRI3 protocol when available, and falls back to
   DRI2 if necessary (can be forced with ``LIBGL_DRI3_DISABLE``).

   This driver can share DRI drivers with ``libGL``.

``haiku``
   This driver supports only the `Haiku <https://www.haiku-os.org/>`__
   platform. It is also much less feature-complete than ``egl_dri2``,
   supporting only part of EGL 1.4 and none of the extensions beyond it.

Lifetime of Display Resources
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Contexts and surfaces are examples of display resources. They might live
longer than the display that creates them.

In EGL, when a display is terminated through ``eglTerminate``, all
display resources should be destroyed. Similarly, when a thread is
released through ``eglReleaseThread``, all current display resources
should be released. Another way to destroy or release resources is
through functions such as ``eglDestroySurface`` or ``eglMakeCurrent``.

When a resource that is current to some thread is destroyed, the
resource should not be destroyed immediately. EGL requires the resource
to live until it is no longer current. A driver usually calls
``eglIs<Resource>Bound`` to check if a resource is bound (current) to
any thread in the destroy callbacks. If it is still bound, the resource
is not destroyed.

The main library will mark destroyed current resources as unlinked. In a
driver's ``MakeCurrent`` callback, ``eglIs<Resource>Linked`` can then be
called to check if a newly released resource is linked to a display. If
it is not, the last reference to the resource is removed and the driver
should destroy the resource. But it should be careful here because
``MakeCurrent`` might be called with an uninitialized display.

This is the only mechanism provided by the main library to help manage
the resources. The drivers are responsible to the correct behavior as
defined by EGL.

``EGL_RENDER_BUFFER``
~~~~~~~~~~~~~~~~~~~~~

In EGL, the color buffer a context should try to render to is decided by
the binding surface. It should try to render to the front buffer if the
binding surface has ``EGL_RENDER_BUFFER`` set to ``EGL_SINGLE_BUFFER``;
If the same context is later bound to a surface with
``EGL_RENDER_BUFFER`` set to ``EGL_BACK_BUFFER``, the context should try
to render to the back buffer. However, the context is allowed to make
the final decision as to which color buffer it wants to or is able to
render to.

For pbuffer surfaces, the render buffer is always ``EGL_BACK_BUFFER``.
And for pixmap surfaces, the render buffer is always
``EGL_SINGLE_BUFFER``. Unlike window surfaces, EGL spec requires their
``EGL_RENDER_BUFFER`` values to be honored. As a result, a driver should
never set ``EGL_PIXMAP_BIT`` or ``EGL_PBUFFER_BIT`` bits of a config if
the contexts created with the config won't be able to honor the
``EGL_RENDER_BUFFER`` of pixmap or pbuffer surfaces.

It should also be noted that pixmap and pbuffer surfaces are assumed to
be single-buffered, in that ``eglSwapBuffers`` has no effect on them. It
is desirable that a driver allocates a private color buffer for each
pbuffer surface created. If the window system the driver supports has
native pbuffers, or if the native pixmaps have more than one color
buffers, the driver should carefully attach the native color buffers to
the EGL surfaces, re-route them if required.

There is no defined behavior as to, for example, how ``glDrawBuffer``
interacts with ``EGL_RENDER_BUFFER``. Right now, it is desired that the
draw buffer in a client API be fixed for pixmap and pbuffer surfaces.
Therefore, the driver is responsible to guarantee that the client API
renders to the specified render buffer for pixmap and pbuffer surfaces.

``EGLDisplay`` Mutex
~~~~~~~~~~~~~~~~~~~~

The ``EGLDisplay`` will be locked before calling any of the dispatch
functions (well, except for GetProcAddress which does not take an
``EGLDisplay``). This guarantees that the same dispatch function will
not be called with the same display at the same time. If a driver has
access to an ``EGLDisplay`` without going through the EGL APIs, the
driver should as well lock the display before using it.
