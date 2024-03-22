License and Copyright
=====================

Disclaimer
----------

Mesa implements various APIs, including `OpenGL`_, `OpenGL ES`_,
`Vulkan`_ and `OpenCL`_. Even though Mesa implements these APIs, the
implementation isn't formally conformant on all combinations of drivers
and hardware. `Khronos`_ maintains lists of conformant implementations
for each of their APIs, as well as `trademark details`_.

Please do not refer to the library as *MesaGL* (for legal reasons). It's
just *Mesa* or *The Mesa 3-D graphics library*.

.. _OpenGL: https://www.opengl.org/
.. _OpenGL ES: https://www.khronos.org/opengles/
.. _Vulkan: https://www.vulkan.org/
.. _OpenCL: https://www.khronos.org/opencl/
.. _Khronos: https://www.khronos.org/
.. _trademark details: https://www.khronos.org/legal/trademarks/

License / Copyright Information
-------------------------------

The Mesa distribution consists of several components. Different
copyrights and licenses apply to different components. For example, the
GLX client code uses the SGI Free Software License B, and some of the
Mesa device drivers are copyrighted by their authors. See below for a
list of Mesa's main components and the license for each.

The core Mesa library is licensed according to the terms of the MIT
license. This allows integration with the XFree86, X.Org and DRI
projects.

The default Mesa license is as follows:

::

   Copyright (C) 1999-2007  Brian Paul   All Rights Reserved.

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.

Attention, Contributors
-----------------------

When contributing to the Mesa project you must agree to the licensing
terms of the component to which you're contributing. The following
section lists the primary components of the Mesa distribution and their
respective licenses.

Mesa Component Licenses
-----------------------

+-----------------+------------------------+-----------------------------+
| Component       | Location               | License                     |
+=================+========================+=============================+
| Main Mesa code  | src/mesa/              | MIT                         |
+-----------------+------------------------+-----------------------------+
| Gallium code    | src/gallium/           | MIT                         |
+-----------------+------------------------+-----------------------------+
| Ext headers     | include/GL/glext.h,    | Khronos                     |
|                 | include/GL/glxext.h    |                             |
+-----------------+------------------------+-----------------------------+
| GLX client code | src/glx/               | SGI Free Software License B |
+-----------------+------------------------+-----------------------------+
| C11 thread      | src/c11/impl/threads*  | Boost (permissive)          |
| emulation       |                        |                             |
+-----------------+------------------------+-----------------------------+

In general, consult the source files for license terms.
