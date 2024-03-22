NVK
===

NVK is a Vulkan driver for Nvidia GPUs.

Hardware support
----------------

NVK currently supports Turing (RTX 20XX and GTX 16XX) and later GPUs.
Eventually, we plan to support as far back as Kepler (GeForce 600 and 700
series) GPUs but anything pre-Turing is currently disabled by default.

Kernel requirements
-------------------

NVK requires at least a Linux 6.6 kernel

Conformance status:
-------------------

NVK is not currently conformant on any hardware.  As of the writing of this
documentation, it was failing about 2000 tests with the current feature
set.

Debugging
---------

Here are a few environment variable debug environment variables
specific to NVK:

:envvar:`NVK_DEBUG`:
   a comma-separated list of named flags, which do various things:

   ``push_dump``
      Dumps all pusbufs to stderr on submit.  This requires that
      ``push_sync`` also be set.
   ``push_sync``
      Waits for submit to complete before continuing
   ``zero_memory``
      Zeros all VkDeviceMemory objects upon creation

:envvar:`NVK_I_WANT_A_BROKEN_VULKAN_DRIVER`
   If defined to ``1`` or ``true``, this will enable enumeration of all
   GPUs Kepler and later, including GPUs for which hardware support is
   poorly tested or completely broken.  This is intended for developer use
   only.

Hardware Documentation
----------------------

What little documentation we have can be found in the `NVIDIA open-gpu-doc
repository <https://github.com/NVIDIA/open-gpu-doc>`__.  The majority of
our documentation comes in the form of class headers which describe the
class state registers.
