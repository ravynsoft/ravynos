Distribution
============

Along with the interface definitions, the following drivers, Gallium frontends,
and auxiliary modules are shipped in the standard Gallium distribution.

Drivers
-------

Intel i915
^^^^^^^^^^

Driver for Intel i915 and i945 chipsets.

LLVM Softpipe
^^^^^^^^^^^^^

A version of :ref:`softpipe` that uses the Low-Level Virtual Machine to
dynamically generate optimized rasterizing pipelines.

NVIDIA NV30
^^^^^^^^^^^

Driver for the NVIDIA NV30 and NV40 families of GPUs.

NVIDIA NV50
^^^^^^^^^^^

Driver for the NVIDIA NV50 family of GPUs.

NVIDIA NVC0
^^^^^^^^^^^

Driver for the NVIDIA NVC0 / Fermi family of GPUs.

VMware SVGA
^^^^^^^^^^^

Driver for VMware virtualized guest operating system graphics processing.

ATI R300
^^^^^^^^

Driver for the ATI/AMD R300, R400, and R500 families of GPUs.

ATI/AMD R600
^^^^^^^^^^^^

Driver for the ATI/AMD R600, R700, Evergreen and Northern Islands families of GPUs.

AMD RadeonSI
^^^^^^^^^^^^

Driver for the AMD Southern Islands family of GPUs.

Freedreno
^^^^^^^^^

Driver for Qualcomm Adreno 2xx, 3xx, and 4xx series of GPUs.

.. _softpipe:

Softpipe
^^^^^^^^

Reference software rasterizer. Slow but accurate.

.. _trace:

Trace
^^^^^

Wrapper driver. Trace dumps an XML record of the calls made to the
:ref:`Context` and :ref:`Screen` objects that it wraps.

Gallium frontends
-----------------

Clover
^^^^^^

Tracker that implements the Khronos OpenCL standard.

.. _dri:

Direct Rendering Infrastructure
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Tracker that implements the client-side DRI protocol, for providing direct
acceleration services to X11 servers with the DRI extension. Supports DRI1
and DRI2. Only GL is supported.

GLX
^^^

MesaGL
^^^^^^

The Gallium frontend implementing a GL state machine. Not usable as
a standalone frontend; Mesa should be built with another Gallium frontend,
such as :ref:`DRI` or EGL.

Nine
^^^^

The Gallium frontend implements the Direct3D 9 API.

VDPAU
^^^^^

Tracker for Video Decode and Presentation API for Unix.

WGL
^^^

Xorg DDX
^^^^^^^^

Tracker for Xorg X11 servers. Provides device-dependent
modesetting and acceleration as a DDX driver.

Auxiliary
---------

OS
^^

The OS module contains the abstractions for basic operating system services:

* memory allocation
* simple message logging
* obtaining run-time configuration option
* threading primitives

This is the bare minimum required to port Gallium to a new platform.

The OS module already provides the implementations of these abstractions for
the most common platforms.  When targeting an embedded platform no
implementation will be provided -- these must be provided separately.

CSO Cache
^^^^^^^^^

The CSO cache is used to accelerate preparation of state by saving
driver-specific state structures for later use.

.. _draw:

Draw
^^^^

Draw is a software :term:`TCL` pipeline for hardware that lacks vertex shaders
or other essential parts of pre-rasterization vertex preparation.

Gallivm
^^^^^^^

Indices
^^^^^^^

Indices provides tools for translating or generating element indices for
use with element-based rendering.

Pipe Buffer Managers
^^^^^^^^^^^^^^^^^^^^

Each of these managers provides various services to drivers that are not
fully utilizing a memory manager.

Remote Debugger
^^^^^^^^^^^^^^^

Runtime Assembly Emission
^^^^^^^^^^^^^^^^^^^^^^^^^

TGSI
^^^^

The TGSI auxiliary module provides basic utilities for manipulating TGSI
streams.

Translate
^^^^^^^^^

Util
^^^^

