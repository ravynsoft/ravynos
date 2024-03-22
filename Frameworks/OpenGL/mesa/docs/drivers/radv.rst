RADV
====

RADV is a Vulkan driver for AMD GCN/RDNA GPUs.

Debugging
---------

For a list of environment variables to debug RADV, please see
:ref:`radv env-vars` for a list.

Hardware Documentation
----------------------

You can find a list of documentation for the various generations of
AMD hardware on the `X.Org wiki
<https://www.x.org/wiki/RadeonFeature/#documentation>`__.

Additional community-written documentation is also available in Mesa:

.. toctree::
   :glob:

   amd/hw/*

ACO
---

ACO is the shader-compiler used in RADV. You read its documentation
`here <https://gitlab.freedesktop.org/mesa/mesa/-/blob/main/src/amd/compiler/README.md>`__.
