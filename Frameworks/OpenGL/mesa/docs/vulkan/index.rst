Vulkan Runtime
==============

The Vulkan runtime and utility code in Mesa provides a powerful shared core
for building Vulkan drivers.  It's a collection of base structures (think
base classes in OOO) which allow us to implement a bunch of the annoying
hardware-agnostic bits in common code.

.. toctree::
   :maxdepth: 2

   base-objs
   dispatch
   command-pools
   graphics-state
   renderpass
