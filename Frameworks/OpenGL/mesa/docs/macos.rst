Notes for macOS
================

.. image:: https://github.com/mesa3d/mesa/actions/workflows/macos.yml/badge.svg
    :target: https://github.com/mesa3d/mesa/actions/workflows/macos.yml
    :alt: macOS CI

Mesa builds on macOS without modifications. However, there are some details to
be aware of.

-  Mesa has a number of build-time dependencies. Most dependencies, including
   Meson itself, are available in `homebrew <https://brew.sh>`__, which has a
   Mesa package for reference. The exception seems to be Mako, a Python module
   used for templating, which you can install as ``pip3 install mako``.
-  macOS is picky about its build-time environment. Type ``brew sh`` before
   building to get the Homebrew dependencies in your path.

Mesa's default builds with the Apple GLX uses Mesa as a front for the
hardware-accelerated system OpenGL framework, to provide hardware acceleration
to X11 applications on macOS running via XQuartz.

Mesa's software rasterizers also work on macOS. To build, set the build options
``-Dosmesa=true -Dglx=xlib`` and select an appropriate Gallium software
rasterizer.
