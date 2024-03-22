VMware SVGA3D
=============

This page describes how to build, install and use the
`VMware <https://www.vmware.com/>`__ guest GL driver (aka the SVGA or
SVGA3D driver) for Linux using the latest source code. This driver gives
a Linux virtual machine access to the host's GPU for
hardware-accelerated 3D. VMware Workstation running on Linux or Windows
and VMware Fusion running on MacOS are all supported.

With VMware Workstation 17 / Fusion 13 releases, OpenGL 4.3 is
supported in the guest. This requires

- The vmwgfx kernel module version 2.20 or later
- The VM needs to be configured to hardware version 20 or later.
- MESA 22.0 or later should be installed.

You can disable GL4.3 support using environment variable SVGA_GL43=0 or
lowering hardware version.

Most modern Linux distributions include the SVGA3D driver so end users
shouldn't be concerned with this information. But if your distributions
lacks the driver or you want to update to the latest code these
instructions explain what to do.

Components
----------

The components involved in this include:

-  Linux kernel module: vmwgfx
-  User-space libdrm library
-  Mesa/Gallium OpenGL driver: "svga"

All of these components reside in the guest Linux virtual machine. On
the host, all you're doing is running VMware
`Workstation Pro <https://www.vmware.com/products/workstation-pro.html>`__ or
`Fusion <https://www.vmware.com/products/fusion.html>`__.

Prerequisites
-------------

-  vmwgfx Kernel module version at least 2.20
-  Ubuntu: For Ubuntu you need to install a number of build
   dependencies.

   ::

      sudo apt-get install autoconf automake libtool flex bison zstd
      sudo apt-get install build-essential g++ git
      sudo apt-get install libexpat1-dev libpciaccess-dev \
                           libpthread-stubs0-dev \
                           libudev-dev libx11-xcb-dev \
                           libxcb-dri2-0-dev libxcb-dri3-dev
      sudo apt-get install libxcb-glx0-dev libxcb-present-dev \
                           libxcb-shm0-dev libxcb-xfixes0-dev
      sudo apt-get install libxdamage-dev libxext-dev \
                           libxfixes-dev libxkbcommon-dev
      sudo apt-get install libxml2-dev libxrandr-dev \
                           libxshmfence-dev libxxf86vm-dev
      sudo apt-get install mesa-utils meson ninja-build \
                           pkg-config python3-mako python3-setuptools
      sudo apt-get install x11proto-dri2-dev x11proto-gl-dev \
                           xutils-dev libglvnd-dev

Depending on your Linux distribution, other packages may be needed. Meson
should tell you what's missing.

Getting the Latest Source Code
------------------------------

Begin by saving your current directory location:

::

   export TOP=$PWD
     

-  Mesa/Gallium main branch. This code is used to build libGL, and the
   direct rendering svga driver for libGL, vmwgfx_dri.so, and the X
   acceleration library libxatracker.so.x.x.x.

   ::

      git clone https://gitlab.freedesktop.org/mesa/mesa.git
        

-  libdrm, a user-space library that interfaces with DRM. Most
   distributions ship with this but it's safest to install a newer
   version. To get the latest code from Git:

   ::

      git clone https://gitlab.freedesktop.org/mesa/drm.git
        

Building the Code
-----------------

-  Determine where the GL-related libraries reside on your system and
   set the LIBDIR environment variable accordingly.

   For Ubuntu systems:

   ::

      export LIBDIR=/usr/lib/x86_64-linux-gnu


-  Build libdrm:

   ::

      cd $TOP/drm
      meson builddir --prefix=/usr --libdir=${LIBDIR}
      meson compile -C builddir
      sudo meson install -C builddir
        

-  Build Mesa:

   ::

      cd $TOP/mesa
      meson builddir -Dvulkan-drivers= -Dgallium-drivers=svga -Ddri-drivers= -Dglvnd=true -Dglvnd-vendor-name=mesa

      meson compile -C builddir
      sudo meson install -C builddir
        

   Note that you may have to install other packages that Mesa depends
   upon if they're not installed in your system. You should be told
   what's missing.

   The generated vmwgfx_dri.so is used by the OpenGL libraries during direct rendering, and by the X.Org
   server during accelerated indirect GL rendering.

Running OpenGL Programs
-----------------------

In a shell, run 'glxinfo' and look for the following to verify that the
driver is working:

::

   OpenGL vendor string: VMware, Inc.
   OpenGL renderer string: SVGA3D; build: RELEASE;
   OpenGL version string: 4.3 (Compatibility Profile) Mesa 23.0

If OpenGL 4.3 is not working (you only get OpenGL 4.1):

-  Make sure the VM uses hardware version 20 or later.
-  Make sure the vmwgfx kernel module is version 2.20.0 or later.
-  Check the vmware.log file for errors.
