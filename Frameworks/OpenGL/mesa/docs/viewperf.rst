Viewperf Issues
===============

This page lists known issues with `SPEC Viewperf
11 <https://gwpg.spec.org/benchmarks/benchmark/specviewperf-11/>`__ and
`SPEC Viewperf
12 <https://gwpg.spec.org/benchmarks/benchmark/specviewperf-12/>`__ when
running on Mesa-based drivers.

The Viewperf data sets are basically GL API traces that are recorded
from CAD applications, then replayed in the Viewperf framework.

The primary problem with these traces is they blindly use features and
OpenGL extensions that were supported by the OpenGL driver when the
trace was recorded, but there's no checks to see if those features are
supported by the driver when playing back the traces with Viewperf.

These issues have been reported to the SPEC organization in the hope
that they'll be fixed in the future.

Viewperf 11
-----------

Some of the Viewperf 11 tests use a lot of memory. At least 2GB of RAM
is recommended.

Catia-03 test 2
~~~~~~~~~~~~~~~

This test creates over 38000 vertex buffer objects. On some systems this
can exceed the maximum number of buffer allocations. Mesa generates
GL_OUT_OF_MEMORY errors in this situation, but Viewperf does no error
checking and continues. When this happens, some drawing commands become
no-ops. This can also eventually lead to a segfault either in Viewperf
or the Mesa driver.

Catia-03 tests 3, 4, 8
~~~~~~~~~~~~~~~~~~~~~~

These tests use features of the
:ext:`GL_NV_fragment_program2` and :ext:`GL_NV_vertex_program3` extensions
without checking if the driver supports them.

When Mesa tries to compile the vertex/fragment programs it generates
errors (which Viewperf ignores). Subsequent drawing calls become no-ops
and the rendering is incorrect.

sw-02 tests 1, 2, 4, 6
~~~~~~~~~~~~~~~~~~~~~~

These tests depend on the :ext:`GL_NV_primitive_restart` extension.

If the Mesa driver doesn't support this extension the rendering will be
incorrect and the test will fail.

Also, the color of the line drawings in test 2 seem to appear in a
random color. This is probably due to some uninitialized state
somewhere.

sw-02 test 6
~~~~~~~~~~~~

The lines drawn in this test appear in a random color. That's because
texture mapping is enabled when the lines are drawn, but no texture
image is defined (glTexImage2D() is called with pixels=NULL). Since GL
says the contents of the texture image are undefined in that situation,
we get a random color.

Lightwave-01 test 3
~~~~~~~~~~~~~~~~~~~

This test uses a number of mipmapped textures, but the textures are
incomplete because the last/smallest mipmap level (1 x 1 pixel) is never
specified.

A trace captured with `API
trace <https://github.com/apitrace/apitrace>`__ shows this sequences of
calls like this:

::

   2504 glBindTexture(target = GL_TEXTURE_2D, texture = 55)
   2505 glTexImage2D(target = GL_TEXTURE_2D, level = 0, internalformat = GL_RGBA, width = 512, height = 512, border = 0, format = GL_RGB, type = GL_UNSIGNED_SHORT, pixels = blob(1572864))
   2506 glTexImage2D(target = GL_TEXTURE_2D, level = 1, internalformat = GL_RGBA, width = 256, height = 256, border = 0, format = GL_RGB, type = GL_UNSIGNED_SHORT, pixels = blob(393216))
   2507 glTexImage2D(target = GL_TEXTURE_2D, level = 2, internalformat = GL_RGBA, width = 128, height = 128, border = 0, format = GL_RGB, type = GL_UNSIGNED_SHORT, pixels = blob(98304))
   [...]
   2512 glTexImage2D(target = GL_TEXTURE_2D, level = 7, internalformat = GL_RGBA, width = 4, height = 4, border = 0, format = GL_RGB, type = GL_UNSIGNED_SHORT, pixels = blob(96))
   2513 glTexImage2D(target = GL_TEXTURE_2D, level = 8, internalformat = GL_RGBA, width = 2, height = 2, border = 0, format = GL_RGB, type = GL_UNSIGNED_SHORT, pixels = blob(24))
   2514 glTexParameteri(target = GL_TEXTURE_2D, pname = GL_TEXTURE_MIN_FILTER, param = GL_LINEAR_MIPMAP_LINEAR)
   2515 glTexParameteri(target = GL_TEXTURE_2D, pname = GL_TEXTURE_WRAP_S, param = GL_REPEAT)
   2516 glTexParameteri(target = GL_TEXTURE_2D, pname = GL_TEXTURE_WRAP_T, param = GL_REPEAT)
   2517 glTexParameteri(target = GL_TEXTURE_2D, pname = GL_TEXTURE_MAG_FILTER, param = GL_NEAREST)

Note that one would expect call 2514 to be glTexImage(level=9, width=1,
height=1) but it's not there.

The minification filter is GL_LINEAR_MIPMAP_LINEAR and the texture's
GL_TEXTURE_MAX_LEVEL is 1000 (the default) so a full mipmap is expected.

Later, these incomplete textures are bound before drawing calls.
According to the GL specification, if a fragment program or fragment
shader is being used, the sampler should return (0,0,0,1) ("black") when
sampling from an incomplete texture. This is what Mesa does and the
resulting rendering is darker than it should be.

It appears that NVIDIA's driver (and possibly AMD's driver) detects this
case and returns (1,1,1,1) (white) which causes the rendering to appear
brighter and match the reference image (however, AMD's rendering is
*much* brighter than NVIDIA's).

If the fallback texture created in \_mesa_get_fallback_texture() is
initialized to be full white instead of full black the rendering appears
correct. However, we have no plans to implement this work-around in
Mesa.

Maya-03 test 2
~~~~~~~~~~~~~~

This test makes some unusual calls to glRotate. For example:

.. code-block:: c

   glRotate(50, 50, 50, 1);
   glRotate(100, 100, 100, 1);
   glRotate(52, 52, 52, 1);

These unusual values lead to invalid modelview matrices. For example,
the last glRotate command above produces this matrix with Mesa:

.. math::

   \begin{matrix}
   1.08536 \times 10^{24} & 2.55321 \times 10^{-23} & -0.000160389         & 0\\
   5.96937 \times 10^{25} & 1.08536 \times 10^{24}  & 103408               & 0\\
                   103408 & -0.000160389            & 1.74755\times 10^{9} & 0\\
   0                      &                       0 &                      0 & nan
   \end{matrix}

and with NVIDIA's OpenGL:

.. math::

   \begin{matrix}
   1.4013 \times 10^{-45} &                      0 &                   -nan & 0\\
                        0 & 1.4013 \times 10^{-45} & 1.4013 \times 10^{-45} & 0\\
   1.4013 \times 10^{-45} &                   -nan & 1.4013 \times 10^{-45} & 0\\
                        0 &                      0 &                      0 & 1.4013 \times 10^{-45}
   \end{matrix}

This causes the object in question to be drawn in a strange orientation
and with a semi-random color (between white and black) since GL_FOG is
enabled.

Proe-05 test 1
~~~~~~~~~~~~~~

This uses depth testing but there's two problems:

#. The glXChooseFBConfig() call doesn't request a depth buffer
#. The test never calls glClear(GL_DEPTH_BUFFER_BIT) to initialize the
   depth buffer

If the chosen visual does not have a depth buffer, you'll see the
wireframe car model but it won't be rendered correctly.

If (by luck) the chosen visual has a depth buffer, its initial contents
will be undefined so you may or may not see parts of the model.

Interestingly, with NVIDIA's driver most visuals happen to have a depth
buffer and apparently the contents are initialized to 1.0 by default so
this test just happens to work with their drivers.

Finally, even if a depth buffer was requested and the
glClear(GL_COLOR_BUFFER_BIT) calls were changed to
glClear(GL_COLOR_BUFFER_BIT \| GL_DEPTH_BUFFER_BIT) the problem still
wouldn't be fixed because GL_DEPTH_WRITEMASK=GL_FALSE when glClear is
called so clearing the depth buffer would be a no-op anyway.

Proe-05 test 6
~~~~~~~~~~~~~~

This test draws an engine model with a two-pass algorithm. The first
pass is drawn with polygon stipple enabled. The second pass is drawn
without polygon stipple but with blending and GL_DEPTH_FUNC=GL_LEQUAL.
If either of the two passes happen to use a software fallback of some
sort, the Z values of fragments may be different between the two passes.
This leads to incorrect rendering.

For example, the VMware SVGA Gallium driver uses a special semi-fallback
path for drawing with polygon stipple. Since the two passes are rendered
with different vertex transformation implementations, the rendering
doesn't appear as expected. Setting the SVGA_FORCE_SWTNL environment
variable to 1 will force the driver to use the software vertex path all
the time and clears up this issue.

According to the OpenGL invariance rules, there's no guarantee that the
pixels produced by these two rendering states will match. To achieve
invariance, both passes should enable polygon stipple and blending with
appropriate patterns/modes to ensure the same fragments are produced in
both passes.

Viewperf 12
-----------

Note that Viewperf 12 only runs on 64-bit Windows 7 or later.

catia-04
~~~~~~~~

One of the catia tests calls wglGetProcAddress() to get some
:ext:`GL_EXT_direct_state_access` functions (such as
glBindMultiTextureEXT) and some :ext:`GL_NV_half_float` functions (such
as glMultiTexCoord3hNV). If the extension/function is not supported,
wglGetProcAddress() can return NULL. Unfortunately, Viewperf doesn't check
for null pointers and crashes when it later tries to use the pointer.

Another catia test uses OpenGL 3.1's primitive restart feature. But when
Viewperf creates an OpenGL context, it doesn't request version 3.1 If
the driver returns version 3.0 or earlier all the calls related to
primitive restart generate an OpenGL error. Some of the rendering is
then incorrect.

energy-01
~~~~~~~~~

This test creates a 3D luminance texture of size 1K x 1K x 1K. If the
OpenGL driver/device doesn't support a texture of this size the
glTexImage3D() call will fail with GL_INVALID_VALUE or GL_OUT_OF_MEMORY
and all that's rendered is plain white polygons. Ideally, the test would
use a proxy texture to determine the max 3D texture size. But it does
not do that.

maya-04
~~~~~~~

This test generates many GL_INVALID_OPERATION errors in its calls to
glUniform(). Causes include:

-  Trying to set float uniforms with glUniformi()
-  Trying to set float uniforms with glUniform3f()
-  Trying to set matrix uniforms with glUniform() instead of
   glUniformMatrix().

Apparently, the indexes returned by glGetUniformLocation() were
hard-coded into the application trace when it was created. Since
different implementations of glGetUniformLocation() may return different
values for any given uniform name, subsequent calls to glUniform() will
be invalid since they refer to the wrong uniform variables. This causes
many OpenGL errors and leads to incorrect rendering.

medical-01
~~~~~~~~~~

This test uses a single GLSL fragment shader which contains a GLSL 1.20
array initializer statement, but it neglects to specify ``#version 120``
at the top of the shader code. So, the shader does not compile and all
that's rendered is plain white polygons.

Also, the test tries to create a very large 3D texture that may exceed
the device driver's limit. When this happens, the glTexImage3D call
fails and all that's rendered is a white box.

showcase-01
~~~~~~~~~~~

This is actually a DX11 test based on Autodesk's Showcase product. As
such, it won't run with Mesa.
