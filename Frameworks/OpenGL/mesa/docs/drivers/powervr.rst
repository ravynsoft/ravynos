PowerVR
=======

PowerVR is a Vulkan driver for Imagination Technologies PowerVR GPUs, starting
with those based on the Rogue architecture.

The driver is **not yet fully conformant** so it requires exporting
``PVR_I_WANT_A_BROKEN_VULKAN_DRIVER=1`` to the environment before running any
Vulkan content.

The following hardware is currently in active development:

========= =========== ============== ======= ==========
Product   Series      B.V.N.C        Vulkan  Conformant
========= =========== ============== ======= ==========
GX6250    Series 6XT  4.40.2.51      1.0     No
AXE-1-16M A-Series    33.15.11.3     1.0     No
BXS-4-64  B-Series    36.53.104.796  1.0     No
========= =========== ============== ======= ==========

In some cases, a product name is shared across multiple BVNCs so to check for
support make sure the BVNC matches the one listed. As the feature set and
hardware issues can vary between BVNCs, additional driver changes might be
necessary even for devices sharing the same product name.

Hardware documentation can be found at: https://docs.imgtec.com/

Note: GPUs prior to Series6 do not have the hardware capabilities required to
support Vulkan and therefore cannot be supported by this driver.

Chat
----

PowerVR developers and users hang out on IRC at ``#powervr`` on OFTC. Note
that registering and authenticating with ``NickServ`` is required to prevent
spam. `Join the chat. <https://webchat.oftc.net/?channels=powervr>`_

Hardware glossary
-----------------

.. glossary:: :sorted:

   BVNC
      Set of four numbers used to uniquely identify each GPU (Series6 onwards).
      This is used to determine the GPU feature set, along with any known
      hardware issues.
