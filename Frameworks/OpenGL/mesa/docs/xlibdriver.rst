Xlib Software Driver
====================

Mesa's Xlib driver provides an emulation of the GLX interface so that
OpenGL programs which use the GLX API can render to any X display, even
those that don't support the GLX extension. Effectively, the Xlib driver
converts all OpenGL rendering into Xlib calls.

The Xlib driver is the oldest Mesa driver and the most mature of Mesa's
software-only drivers.

Since the Xlib driver *emulates* the GLX extension, it's not totally
conformant with a true GLX implementation. The differences are fairly
obscure, however.

The unique features of the Xlib driver follows.

X Visual Selection
------------------

Mesa supports RGB(A) rendering into TrueColor and DirectColor visuals, for
any depth with a corresponding renderable OpenGL texture format.

The glXChooseVisual function tries to choose the best X visual for the
given attribute list. However, if this doesn't suit your needs you can
force Mesa to use any X visual you want (any supported by your X server
that is) by setting the **MESA_RGB_VISUAL** environment variable. When
a visual is requested, glXChooseVisual will first look if the
MESA_RGB_VISUAL variable is defined. If so, it will try to use the
specified visual.

The format of accepted values is: ``visual-class depth``

Here are some examples:

::

   using csh:
       % setenv MESA_RGB_VISUAL "TrueColor 8"      // 8-bit TrueColor
       % setenv MESA_RGB_VISUAL "DirectColor 30"   // 30-bit DirectColor

   using bash:
       $ export MESA_RGB_VISUAL="TrueColor 8"
       $ export MESA_RGB_VISUAL="DirectColor 30"

Double Buffering
----------------

Mesa can use either an X Pixmap or XImage as the back color buffer when
in double-buffer mode. The default is to use an XImage. The
**MESA_BACK_BUFFER** environment variable can override this. The valid
values for **MESA_BACK_BUFFER** are: **Pixmap** and **XImage** (only the
first letter is checked, case doesn't matter).

Using XImage is almost always faster than a Pixmap since it resides in
the application's address space. When glXSwapBuffers() is called,
XPutImage() or XShmPutImage() is used to transfer the XImage to the
on-screen window.

A Pixmap may be faster when doing remote rendering of a simple scene.
Some OpenGL features will be very slow with a Pixmap (for example,
blending will require a round-trip message for pixel readback.)

Experiment with the MESA_BACK_BUFFER variable to see which is faster for
your application.

Colormaps
---------

When using Mesa directly or with GLX, it's up to the application writer
to create a window with an appropriate colormap. The GLUT toolkit tries
to minimize colormap *flashing* by sharing colormaps when possible.
Specifically, if the visual and depth of the window matches that of the
root window, the root window's colormap will be shared by the Mesa
window. Otherwise, a new, private colormap will be allocated.

When sharing the root colormap, Mesa may be unable to allocate the
colors it needs, resulting in poor color quality. This can happen when a
large number of colorcells in the root colormap are already allocated.

Overlay Planes
--------------

Hardware overlay planes are supported by the Xlib driver. To determine
if your X server has overlay support you can test for the
SERVER_OVERLAY_VISUALS property:

.. code-block:: console

   xprop -root | grep SERVER_OVERLAY_VISUALS


Extensions
----------

The following Mesa-specific extensions are implemented in the Xlib
driver.

GLX_MESA_pixmap_colormap
~~~~~~~~~~~~~~~~~~~~~~~~

This extension adds the GLX function:

.. code-block:: c

   GLXPixmap glXCreateGLXPixmapMESA( Display *dpy, XVisualInfo *visual,
                                     Pixmap pixmap, Colormap cmap )

It is an alternative to the standard glXCreateGLXPixmap() function.
Since Mesa supports RGB rendering into any X visual, not just True-
Color or DirectColor, Mesa needs colormap information to convert RGB
values into pixel values. An X window carries this information but a
pixmap does not. This function associates a colormap to a GLX pixmap.
See the xdemos/glxpixmap.c file for an example of how to use this
extension.

`GLX_MESA_pixmap_colormap
specification <specs/MESA_pixmap_colormap.spec>`__

GLX_MESA_release_buffers
~~~~~~~~~~~~~~~~~~~~~~~~

Mesa associates a set of ancillary (depth, accumulation, stencil and
alpha) buffers with each X window it draws into. These ancillary buffers
are allocated for each X window the first time the X window is passed to
glXMakeCurrent(). Mesa, however, can't detect when an X window has been
destroyed in order to free the ancillary buffers.

The best it can do is to check for recently destroyed windows whenever
the client calls the glXCreateContext() or glXDestroyContext()
functions. This may not be sufficient in all situations though.

The GLX_MESA_release_buffers extension allows a client to explicitly
deallocate the ancillary buffers by calling glxReleaseBuffersMESA() just
before an X window is destroyed. For example:

.. code-block:: c

   #ifdef GLX_MESA_release_buffers
      glXReleaseBuffersMESA( dpy, window );
   #endif
   XDestroyWindow( dpy, window );

`GLX_MESA_release_buffers
specification <specs/MESA_release_buffers.spec>`__

This extension was added in Mesa 2.0.

GLX_MESA_copy_sub_buffer
~~~~~~~~~~~~~~~~~~~~~~~~

This extension adds the glXCopySubBufferMESA() function. It works like
glXSwapBuffers() but only copies a sub-region of the window instead of
the whole window.

`GLX_MESA_copy_sub_buffer
specification <specs/MESA_copy_sub_buffer.spec>`__

This extension was added in Mesa 2.6

Summary of X-related environment variables
------------------------------------------

+-----------------------------+--------------------------------------+
| Environment variable        | Description                          |
+=============================+======================================+
| :envvar:`MESA_RGB_VISUAL`   | specifies the X visual and depth for |
|                             | RGB mode (X only)                    |
+-----------------------------+--------------------------------------+
| :envvar:`MESA_BACK_BUFFER`  | specifies how to implement the back  |
|                             | color buffer (X only)                |
+-----------------------------+--------------------------------------+
