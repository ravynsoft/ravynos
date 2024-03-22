V3D
===

Mesa's V3D graphics driver stack includes a `conformant GLES3.1
driver
<https://www.khronos.org/conformance/adopters/conformant-products/opengles#submission_882>`__
called V3D and a Vulkan graphics driver called V3DV, notably
used on the Raspberry Pi 4 and Raspberry Pi 5.

The V3D Mesa drivers communicate directly with the `V3D
<https://www.kernel.org/doc/html/latest/gpu/v3d.html>`__ kernel DRM
driver for scheduling GPU commands.  Additionally, on the Raspberry Pi
4 and 5, the kernel uses the VC4 DRM driver for display support, so Mesa
exposes a ``vc4_dri.so`` using the kmsro helpers to do
behind-the-scenes buffer management between the two kernel drivers,
while executing rendering on the V3D kernel module.

Initial development work was done on the Broadcom 7268 (V3D 3.3) and
7278 (V3D 4.1). Development since then has been on V3D 4.2 (Raspberry
Pi 4), and V3D 7.1 (Raspberry Pi 5). When the support for V3D 7.1
landed, the support for 3.3 and 4.1 was dropped as it was not tested
anymore (see `MR#25851
<https://gitlab.freedesktop.org/mesa/mesa/-/merge_requests/25851>`__)
Broadcom's reference software platforms do not make use of the open
source V3D stack, but porting a particular hardware implementation to
use it would still be possible.

Hardware Documentation
----------------------

Broadcom never released a public specification for the V3D 3.x or 4.x
series.

For driver developers, Broadcom publicly released a `specification
<https://docs.broadcom.com/doc/12358545>`__ PDF for the 21553, which
is closely related to the VC4 GPU present in the Raspberry Pi.  They
also released a `snapshot <https://docs.broadcom.com/docs/12358546>`__
of a corresponding Android graphics driver.  That graphics driver was
ported to Raspbian for a demo, but was not expected to have ongoing
development.

Developers with NDA access with Broadcom or Raspberry Pi can get
access to the V3D architecture specification for documentation of the
GPU's programming model.  There is also a C++ software simulator
called simpenrose, and the Mesa driver includes a backend
(``src/broadcom/drm-shim/``) to use simpenrose from an x86 system with
the i915 graphics driver with all of the VC4 rendering commands
emulated on simpenrose and memcpyed to the real GPU.  Note that
simpenrose's API drifts over time, so you need to be synced up with
whatever version Mesa was last being developed against.
