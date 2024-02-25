
.. _what_is_libinput:

==============================================================================
What is libinput?
==============================================================================

This page describes what libinput is, but more importantly it also describes
what libinput is **not**.

.. _what_libinput_is:

------------------------------------------------------------------------------
What libinput is
------------------------------------------------------------------------------

libinput is an input stack for processes that need to provide events from
commonly used input devices. That includes mice, keyboards, touchpads,
touchscreens and graphics tablets. libinput handles device-specific quirks
and provides an easy-to-use API to receive events from devices.

libinput is designed to handle all input devices available on a system but
it is possible to limit which devices libinput has access to.
For example, the use of xf86-input-libinput depends on xorg.conf snippets
for specific devices. But libinput works best if it handles all input
devices as this allows for smarter handling of features that affect multiple
devices.

libinput restricts device-specific features to those devices that require
those features. One example for this are the top software buttons on the
touchpad in the Lenovo T440. While there may be use-cases for providing top
software buttons on other devices, libinput does not do so.

`This introductory blog post from 2015
<https://who-t.blogspot.com/2015/06/libinput-and-lack-of-device-types.html>`_
describes some of the motivations.

.. _what_libinput_is_not:

------------------------------------------------------------------------------
What libinput is not
------------------------------------------------------------------------------

libinput is **not** a project to support experimental devices. Unless a
device is commonly available off-the-shelf, libinput will not support this
device. libinput can serve as a useful base for getting experimental devices
enabled and reduce the amount of boilerplate required. But such support will
not land in libinput master until the devices are commonly available.

libinput is **not** a box of legos. It does not provide the pieces to
assemble a selection of features. Many features can be disabled through
configuration options, but some features are hardcoded and/or only available
on some devices. There are plenty of use-cases to provide niche features,
but libinput is not the place to support these.

libinput is **not** a showcase for features. There are a lot of potential
features that could be provided on input devices. But unless they have
common usage, libinput is not the place to implement them. Every feature
multiplies the maintenance effort, any feature that is provided but unused
is a net drain on the already sparse developer resources libinput has
available.

libinput is boring. It does not intend to break new grounds on how devices
are handled. Instead, it takes best practice and the common use-cases and
provides it in an easy-to-consume package for compositors or other processes
that need those interactions typically expected by users.

.. _libinput-wayland:

------------------------------------------------------------------------------
libinput and Wayland
------------------------------------------------------------------------------

libinput is not used directly by Wayland applications, it is an input stack
used by the compositor. The typical software stack for a system running
Wayland is:

.. graphviz:: libinput-stack-wayland.gv

The Wayland compositor may be Weston, mutter, KWin, etc. Note that
Wayland encourages the use of toolkits, so the Wayland client (your
application) does not usually talk directly to the compositor but rather
employs a toolkit (e.g. GTK) to do so. The Wayland client does not know
whether libinput is in use.

libinput is not a requirement for Wayland or even a Wayland compositor.
There are some specialized compositors that do not need or want libinput.

.. _libinput-xorg:

------------------------------------------------------------------------------
libinput and X.Org
------------------------------------------------------------------------------

libinput is not used directly by X applications but rather through the
custom xf86-input-libinput driver. The simplified software stack for a
system running X.Org is:

.. graphviz:: libinput-stack-xorg.gv

libinput is not employed directly by the X server but by the
xf86-input-libinput driver instead. That driver is loaded by the server
on demand, depending on the xorg.conf.d configuration snippets. The X client
does not know whether libinput is in use.

libinput and xf86-input-libinput are not a requirement, the driver will only
handle those devices explicitly assigned through an xorg.conf.d snippets. It
is possible to mix xf86-input-libinput with other X.Org drivers.

------------------------------------------------------------------------------
Device types
------------------------------------------------------------------------------

libinput handles all common devices used to interact with a desktop system.
This includes mice, keyboards, touchscreens, touchpads and graphics tablets.
libinput does not expose the device type to the caller, it solely provides
capabilities and the attached features (see
`this blog post <https://who-t.blogspot.com/2015/06/libinput-and-lack-of-device-types.html>`_).

For example, a touchpad in libinput is a device that provides pointer
events, gestures and has a number of :ref:`config_options` such as
:ref:`tapping`. A caller may present the device as touchpad to the user, or
simply as device with a config knob to enable or disable tapping.

..............................................................................
Handled device types
..............................................................................

- :ref:`Touchpads`
- Touchscreens
- Mice
- Keyboards
- Virtual absolute pointing devices such as those used by QEMU or VirtualBox
- Switches (Lid Switch and Tablet Mode switch)
- Graphics tablets
- :ref:`Trackpoints`

If a device falls into one of the above categories but does not work as
expected, please :ref:`file a bug <reporting_bugs>`.

..............................................................................
Unhandled device types
..............................................................................

libinput does not handle some devices. The primary reason is that these
device have no clear interaction with a desktop environment.

Joysticks:
     Joysticks have one or more axes and one or more buttons. Beyond that it is
     difficult to find common ground between joysticks and much of the
     interaction is application-specific, not system-specific. libinput does not
     provide support for joysticks for that reason, any abstraction libinput
     would provide for joysticks would be so generic that libinput would
     merely introduce complexity and processing delays for no real benefit.
