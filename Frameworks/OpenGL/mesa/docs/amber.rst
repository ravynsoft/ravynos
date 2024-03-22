Amber Branch
============

After Mesa 21.3, all non-Gallium DRI drivers were removed from the Mesa
source-tree. These drivers are still being maintained to some degree,
but only on the ``amber`` branch, and only for critical fixes.

These drivers include:

-  Radeon
-  r200
-  i915
-  i965
-  Nouveau (the DRI driver for NV04-NV20)

At the same time, the OpenSWR Gallium driver was removed from the Mesa
source-tree, because it was already practically speaking unmaintained and
the actively maintained LLVMpipe offers much of the same functionality.

Users with Intel GPUs that were using i965 should migrate to either Iris
or Crocus, depending on their GPU. These drivers generally speaking both
perform better and have more features than i965 had, and due to sharing
more code with the rest of the Mesa infrastructure, gets more bug fixes
and features.

Similarly, users of i915 should migrate to i915g (the Gallium driver for
the same hardware), as it's still being maintained.

Users who depend on the removed drivers will have to use them built from
the Amber branch in order to get updates.

Building
--------

The Amber branch has some extra logic to be able to coexist with recent
Mesa releases without them stepping on each others toes. In order to
enable that logic, you need to pass the ``-Damber=true`` flag to Meson.

Documentation
-------------

On `docs.mesa3d.org <https://docs.mesa3d.org/>`__, we currently only
publish the documentation from our main branch. But you can view the
documentation for the Amber branch `here
<https://gitlab.freedesktop.org/mesa/mesa/-/tree/amber/docs>`__.

