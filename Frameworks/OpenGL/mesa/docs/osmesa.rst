Off-screen Rendering
====================

Mesa's off-screen interface is used for rendering into user-allocated
memory without any sort of window system or operating system
dependencies. That is, the GL_FRONT colorbuffer is actually a buffer in
main memory, rather than a window on your display.

The OSMesa API provides three basic functions for making off-screen
renderings: OSMesaCreateContext(), OSMesaMakeCurrent(), and
OSMesaDestroyContext(). See the Mesa/include/GL/osmesa.h header for more
information about the API functions.

The OSMesa interface may be used with the gallium software renderers:

#. LLVMpipe - this is the high-performance Gallium LLVM driver
#. Softpipe - this is the reference Gallium software driver

There are several examples of OSMesa in the mesa/demos repository.

Building OSMesa
---------------

Configure and build Mesa with something like:

.. code-block:: console

   meson setup builddir -Dosmesa=true -Dgallium-drivers=swrast -Dvulkan-drivers=[] -Dprefix=$PWD/builddir/install
   meson install -C builddir

Make sure you have LLVM installed first if you want to use the LLVMpipe
driver.

When the build is complete you should find:

::

   $PWD/builddir/install/lib/libOSMesa.so

Set your LD_LIBRARY_PATH to point to $PWD/builddir/install to use the
libraries

When you link your application, link with -lOSMesa
