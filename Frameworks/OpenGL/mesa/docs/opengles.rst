OpenGL ES
=========

Mesa implements OpenGL ES 1.1 and OpenGL ES 2.0. More information about
OpenGL ES can be found at https://www.khronos.org/opengles/.

OpenGL ES depends on a working EGL implementation. Please refer to
:doc:`Mesa EGL <egl>` for more information about EGL.

Build the Libraries
-------------------

#. Run ``meson configure`` with ``-D gles1=enabled -D gles2=enabled`` and
   enable the Gallium driver for your hardware.
#. Build and install Mesa as usual.

Alternatively, if XCB-DRI2 is installed on the system, one can use
``egl_dri2`` EGL driver with OpenGL|ES-enabled DRI drivers

#. Run ``meson configure`` with ``-D gles1=enabled -D gles2=enabled``.
#. Build and install Mesa as usual.

Both methods will install libGLESv1_CM, libGLESv2, libEGL, and one or
more EGL drivers for your hardware.

Run the Demos
-------------

There are some demos in ``mesa/demos`` repository.
