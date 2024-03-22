Project History
===============

The Mesa project was originally started by Brian Paul. Here's a short
history of the project.

August, 1993: I begin working on Mesa in my spare time. The project has
no name at that point. I was simply interested in writing a simple 3D
graphics library that used the then-new OpenGL API. I was partially
inspired by the *VOGL* library which emulated a subset of IRIS GL. I had
been programming with IRIS GL since 1991.

November 1994: I contact SGI to ask permission to distribute my
OpenGL-like graphics library on the internet. SGI was generally
receptive to the idea and after negotiations with SGI's legal
department, I get permission to release it.

February 1995: Mesa 1.0 is released on the internet. I expected that a
few people would be interested in it, but not thousands. I was soon
receiving patches, new features and thank-you notes on a daily basis.
That encouraged me to continue working on Mesa. The name Mesa just
popped into my head one day. SGI had asked me not to use the terms
*"Open"* or *"GL"* in the project name and I didn't want to make up a
new acronym. Later, I heard of the Mesa programming language and the
Mesa spreadsheet for NeXTStep.

In the early days, OpenGL wasn't available on too many systems. It even
took a while for SGI to support it across their product line. Mesa
filled a big hole during that time. For a lot of people, Mesa was their
first introduction to OpenGL. I think SGI recognized that Mesa actually
helped to promote the OpenGL API, so they didn't feel threatened by the
project.

1995-1996: I continue working on Mesa both during my spare time and
during my work hours at the Space Science and Engineering Center at the
University of Wisconsin in Madison. My supervisor, Bill Hibbard, lets me
do this because Mesa is now being using for the
`Vis5D <https://www.ssec.wisc.edu/~billh/vis.html>`__ project.

October 1996: Mesa 2.0 is released. It implements the OpenGL 1.1
specification.

March 1997: Mesa 2.2 is released. It supports the new 3dfx Voodoo
graphics card via the Glide library. It's the first really popular
hardware OpenGL implementation for Linux.

September 1998: Mesa 3.0 is released. It's the first publicly-available
implementation of the OpenGL 1.2 API.

March 1999: I attend my first OpenGL ARB meeting. I contribute to the
development of several official OpenGL extensions over the years.

September 1999: I'm hired by Precision Insight, Inc. Mesa is a key
component of 3D hardware acceleration in the new DRI project for
XFree86. Drivers for 3dfx, 3dLabs, Intel, Matrox and ATI hardware soon
follow.

October 2001: Mesa 4.0 is released. It implements the OpenGL 1.3
specification.

November 2001: I cofounded Tungsten Graphics, Inc. with Keith Whitwell,
Jens Owen, David Dawes and Frank LaMonica. Tungsten Graphics was
acquired by VMware in December 2008.

November 2002: Mesa 5.0 is released. It implements the OpenGL 1.4
specification.

January 2003: Mesa 6.0 is released. It implements the OpenGL 1.5
specification as well as the :ext:`GL_ARB_vertex_program` and
:ext:`GL_ARB_fragment_program` extensions.

June 2007: Mesa 7.0 is released, implementing the OpenGL 2.1
specification and OpenGL Shading Language.

2008: Keith Whitwell and other Tungsten Graphics employees develop
`Gallium <https://en.wikipedia.org/wiki/Gallium3D>`__ - a new GPU
abstraction layer. The latest Mesa drivers are based on Gallium and
other APIs such as OpenVG are implemented on top of Gallium.

February 2012: Mesa 8.0 is released, implementing the OpenGL 3.0
specification and version 1.30 of the OpenGL Shading Language.

July 2016: Mesa 12.0 is released, including OpenGL 4.3 support and
initial support for Vulkan for Intel GPUs. Plus, there's another Gallium
software driver ("OpenSWR") based on LLVM and developed by Intel.

Ongoing: Mesa is the OpenGL implementation for devices designed by
Intel, AMD, NVIDIA, Qualcomm, Broadcom, Vivante, plus the VMware and
VirGL virtual GPUs. There's also several software-based renderers:
Softpipe (a Gallium reference driver) and LLVMpipe (LLVM/JIT-based
high-speed rasterizer).

Work continues on the drivers and core Mesa to implement newer versions
of the OpenGL, OpenGL ES and Vulkan specifications.
