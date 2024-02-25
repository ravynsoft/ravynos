.. _clickpads_with_right_buttons:

==============================================================================
Clickpads with a fake right button
==============================================================================

libinput relies on the kernel to label :ref:`Clickpads <touchpads_buttons_clickpads>`
with the ``INPUT_PROP_BUTTONPAD`` property so it can enable the correct
:ref:`clickpad_softbuttons`. Clickpads are not expected to have a right button
since the whole surface can be depressed.

A common bug encountered with :ref:`Clickpads <touchpads_buttons_clickpads>`
is that the device advertises a physical right button where no such button
exists. This is usually a bug in the firmware of the device and causes the
following warning to be emitted by libinput::

    "<device name> clickpad advertising right button"

The user-visible effect of this is usually negligible since these devices
cannot actually trigger a right click and libinput's default behaviors for
clickpads work as expected.

However, we should nonetheless correct the device to get rid of this warning
and avoid potential issues with future features. The :ref:`device-quirks`
provide a simple way to disable the fake right button on the device. The
following quirk disables the right button on the MyModel laptop from the
MyVendor OEM::

    [MyVendor MyModel Touchpad]
    MatchName=Foo Bar Touchpad
    MatchUdevtype=touchpad
    MatchDMIModAlias=dmi:*svnMyVendor:pnMyModel:*
    AttrEventCode=-BTN_RIGHT

The name of the device can be obtained using :ref:`libinput record <libinput-record>`,
the modalias match is a shell-style glob against the value of ``/sys/class/dmi/id/modalias``.
In most cases, matching should be against ``svn`` (system vendor) and one of
``pn`` (product name) or ``pvr`` (product version), whichever provides a
useful description of the individual laptop model. See the
:ref:`device-quirks` documentation for details on testing local quirks.

For reference, some example commits that add such a quirk are:

 - `bf61ab9bb0694d0ac3d60a7f815779abfe4886e6 <https://gitlab.freedesktop.org/libinput/libinput/-/commit/bf61ab9bb0694d0ac3d60a7f815779abfe4886e6>`__
 - `74fac6d040ac62048882dfb6f73da567ace6a6f5 <https://gitlab.freedesktop.org/libinput/libinput/-/commit/74fac6d040ac62048882dfb6f73da567ace6a6f5>`__
 - `89cd0f990e3bee9906754d6ca8484ed5aa392249 <https://gitlab.freedesktop.org/libinput/libinput/-/commit/89cd0f990e3bee9906754d6ca8484ed5aa392249>`__

