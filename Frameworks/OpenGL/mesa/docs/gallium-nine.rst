Gallium Nine
============

The Gallium frontend, which implements Direct3D 9.

Nine implements the full IDirect3DDevice9 COM interface and a custom COM interface called ID3DAdapter9, which is used to implement the final IDirect3D9Ex COM interface.
ID3DAdapter9 is completely agnostic regarding the window system code, meaning this can be provided by wine, Xlib, Wayland, etc.

Gallium Nine is commonly used in conjunction with `Wine <https://www.winehq.org/>`__.
`Gallium Nine Standalone <https://github.com/iXit/wine-nine-standalone>`__ is the standalone version of the Wine parts of Gallium Nine which makes it possible to use it with any stock Wine version. It's simple to install through `Winetricks <https://github.com/Winetricks/winetricks>`__ with ``winetricks galliumnine``.
Aside from Wine, Gallium Nine works well with `Box86 <https://ptitseb.github.io/box86/>`__.
Can be used via `Zink <https://www.supergoodcode.com/to-the-nines/>`__ even on the `Vulkan API <https://en.wikipedia.org/wiki/Vulkan>`__.

In the majority of cases this implementation has better performance than Wine doing the translation from D3D9 to OpenGL itself.

It's also possible to use D3D9 directly from the Linux environment. For tests, demos, and more details, you can see `this repository <https://github.com/iXit/nine-tests>`__.

Build
-----

Beware: Most Direct3D games are 32-bit, and thus need a 32-bit version of Mesa.

.. code-block:: console

   $ meson configure \
         -D gallium-nine=true \
         -D dri3=enabled \
         ...

Paths
-----

You need to point wine-nine-standalone to the location of ``d3dadapter9.so``.
If you use distribution packaged Mesa, it should work out of the box.

There are three options (sorted from permanent to temporary):
 - compile Wine Nine Standalone with ``D3D9NINE_MODULEPATH`` pointing to your local library
 - set ModulePath of Software\Wine\Direct3DNine in the wine registers
 - ``$ D3D_MODULE_PATH="$MESA_INSTALLDIR/lib/d3d/d3dadapter9.so" wine ...``

Run
---

Before running your application in Wine, verify that everything works as expected by running:

.. code-block:: console

   $ wine ninewinecfg
