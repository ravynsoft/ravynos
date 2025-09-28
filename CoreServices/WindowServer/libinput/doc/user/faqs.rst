.. _faq:

==============================================================================
FAQs - Frequently Asked Questions
==============================================================================

Frequently asked questions about libinput.


.. contents::
    :local:
    :backlinks: entry

.. _faq_feature:

------------------------------------------------------------------------------
Why doesn't libinput support ...?
------------------------------------------------------------------------------

First, read :ref:`what_is_libinput` If you have a feature that you think
libinput needs to support, please file a bug report. See :ref:`reporting_bugs`
for more details.

.. _faq_fast_mouse:

------------------------------------------------------------------------------
My mouse moves too fast, even at the slowest setting
------------------------------------------------------------------------------

This is a symptom of high-dpi mice (greater than 1000dpi). These devices
need a udev hwdb entry to normalize their motion. See
:ref:`motion_normalization` for a detailed explanation.

.. _faq_fast_trackpoint:

------------------------------------------------------------------------------
My trackpoint moves too slow or too fast
------------------------------------------------------------------------------

This is a symptom of an invalid trackpoint multiplier. These devices need
:ref:`device-quirks` to specify the range available so libinput can adjust the
pointer acceleration accordingly. See :ref:`trackpoint_range` for a detailed
explanation.

.. _faq_pointer_acceleration:

------------------------------------------------------------------------------
Why is libinput's pointer acceleration worse than synaptics/evdev
------------------------------------------------------------------------------

This is a known problem affecting some devices and/or use-case but the exact
cause is still unknown. It may be a device-specific issue, it may be a bug
in libinput's acceleration code, it may be a disagreement about how pointer
acceleration should feel. Unfortunately this is something that affected
users need to investigate and analyze.

.. _faq_enable_tapping:

------------------------------------------------------------------------------
Why isn't touchpad tap-to-click enabled by default
------------------------------------------------------------------------------

See :ref:`tapping_default`

.. _faq_touchpad_pressure:

------------------------------------------------------------------------------
Why does my touchpad lose track of touches
------------------------------------------------------------------------------

The most common cause for this is an incorrect pressure threshold range.
See :ref:`touchpad_pressure` for more info.

.. _faq_kinetic_scrolling:

------------------------------------------------------------------------------
Kinetic scrolling does not work
------------------------------------------------------------------------------

The X.Org synaptics driver implemented kinetic scrolling in the driver. It
measures the scroll speed and once the finger leaves the touchpad the driver
keeps sending scroll events for a predetermined time. This effectively
provides for kinetic scrolling without client support but triggers an
unfixable `bug <https://bugs.freedesktop.org/show_bug.cgi?id=38909>`_: the
client cannot know that the events are from a kinetic scroll source. Scroll
events in X are always sent to the current cursor position, a movement of the
cursor after lifting the finger will send the kinetic scroll events to the
new client, something the user does not usually expect. A key event during
the kinetic scroll procedure causes side-effects such as triggering zoom.

libinput does not implement kinetic scrolling for touchpads. Instead it
provides the **libinput_event_pointer_get_axis_source()** function that enables
callers to implement kinetic scrolling on a per-widget basis, see
:ref:`scroll_sources`.

.. _faq_gpl:

------------------------------------------------------------------------------
Is libinput GPL-licensed?
------------------------------------------------------------------------------

No, libinput is MIT licensed. The Linux kernel header file linux/input.h in
libinput's tree is provided to ensure the same behavior regardless of which
kernel version libinput is built on. It does not make libinput GPL-licensed.

.. _faq_config_options:

------------------------------------------------------------------------------
Where is the configuration stored?
------------------------------------------------------------------------------

libinput does not store configuration options, it is up to the caller to
manage these and decide which configuration option to apply to each device.
This must be done at startup, after a resume and whenever a new device is
detected.

One commonly used way to configure libinput is to have the Wayland
compositor expose a compositor-specific configuration option. For example,
in a GNOME stack, the gnome-control-center modifies dconf entries. These
changes are read by mutter and applied to libinput. Changing these entries
via the gsettings commandline tool has the same effect.

Another commonly used way to configure libinput is to have xorg.conf.d
snippets. When libinput is used with the xf86-input-libinput driver in an
X.Org stack, these options are read on startup and apply to each device.
Changing properties at runtime with the xinput commandline tool has the same
effect.

In both cases, the selection of available options and how they are exposed
depends on the libinput caller (e.g. mutter or xf86-input-libinput).

.. graphviz:: libinput-stack-gnome.gv

This has an effect on the availability of configuration options: if an
option is not exposed by the intermediary, it cannot be configured by the
client. Also some configuration options that are provided by the
intermediary may not be libinput-specific configuration options.

.. _faq_configure_wayland:

------------------------------------------------------------------------------
How do I configure my device on Wayland?
------------------------------------------------------------------------------

See :ref:`faq_config_options` Use the configuration tool provided by your
desktop environment (e.g. gnome-control-center) or direct access to your
desktop environment's configuration storage (e.g. gsettings).

.. _faq_configure_xorg:

------------------------------------------------------------------------------
How do I configure my device on X?
------------------------------------------------------------------------------

See :ref:`faq_config_options`  If your desktop environment does not provide a
graphical configuration tool you can use an
`xorg.conf.d snippet <https://www.x.org/archive/current/doc/man/man5/xorg.conf.5.xhtml>`_.
Usually, such a snippet looks like this:

::

     $> cat /etc/X11/xorg.conf.d/99-libinput-custom-config.conf
     Section "InputClass"
       Identifier "something to identify this snippet"
       MatchDriver "libinput"
       MatchProduct "substring of the device name"
       Option "some option name" "the option value"
     EndSection


The identifier is merely a human-readable string that shows up in the log
file. The MatchProduct line should contain the device name or a substring of
the device name that the snippet should apply to. For a full list of option
names and permitted values, see the
`libinput man page <https://www.mankier.com/4/libinput>`_.
xorg.conf.d snippets like the above apply to hotplugged devices but can be
overwritten at runtime by desktop tools. Multiple snippets may be placed
into the same file.

For run-time configuration and testing, the
`xinput <https://www.x.org/archive/X11R7.5/doc/man/man1/xinput.1.html>`_
debugging tool can modify a devices' properties. See the
`libinput man page <https://www.mankier.com/4/libinput>`_
for supported property names and values. Usually, an invocation looks like
this:

::

     $> xinput set-prop "the device name" "the property name" value [value2] [value3]


.. note:: Changes performed by xinput do not persist across device hotplugs. xinput
	is considered a debugging and testing tool only and should not be used
	for permanent configurations.

.. _faq_configuration:

------------------------------------------------------------------------------
Can you add a configuration option for $FEATURE?
------------------------------------------------------------------------------

No. At least that's going to be the initial answer. Read
`Why libinput doesn't have a lot of configuration options <http://who-t.blogspot.com/2016/04/why-libinput-doesnt-have-lot-of-config.html>`_
first. Configuration options for most features are a signal that we are incapable
of handling it correctly. To get to that point, we want to be sure we're
truly incapable of doing so. libinput has several features that
are handled automatically (and correctly) that users wanted to have
configuration options for initially.

So the answer to this question will almost always be 'no'. A configuration
option is, in most cases, a cop-out.

.. _faq_synclient:

------------------------------------------------------------------------------
Why don't synclient and syndaemon work with libinput?
------------------------------------------------------------------------------

Synclient and syndaemon rely on X input device properties that are specific
to the xf86-input-synaptics X.Org input driver. Both were written when the
synaptics driver was the only common touchpad driver in existence. They
assume that if the properties aren't available, no touchpad is available
either. The xf86-input-libinput X.Org input driver does not export these
driver-specific properties, synclient/syndaemon will thus not detect the
touchpad and refuse to work. Other tools that rely on synclient/syndaemon or
those same properties also do not work with xf86-input-libinput.

Most of syndaemon's functionality is built into libinput, see
:ref:`disable-while-typing`. synclient is merely a configuration tool, see
:ref:`faq_configure_xorg` for similar functionality.

See also the blog posts
`The definitive guide to synclient <http://who-t.blogspot.com.au/2017/01/the-definitive-guide-to-synclient.html>`_ and
`The future of xinput, xmodmap, setxkbmap, xsetwacom and other tools under Wayland <http://who-t.blogspot.com.au/2016/12/the-future-of-xinput-xmodmap-setxkbmap.html>`_

.. _faq_tablets:

------------------------------------------------------------------------------
Does libinput support non-Wacom tablets?
------------------------------------------------------------------------------

Yes, though unfortunately many non-Wacom tablets suffer from bad firmware
and don't send the required events. But they should all work nonetheless. If
you have a tablet that does not work with libinput, please
:ref:`file a bug <reporting_bugs>`.

.. _faq_tablet_capabilities:

------------------------------------------------------------------------------
My tablet doesn't work
------------------------------------------------------------------------------

If you see the message

::

     libinput bug: device does not meet tablet criteria. Ignoring this device.


or the message

::

     missing tablet capabilities [...] Ignoring this device.


your tablet device does not have the required capabilities to be treated as
a tablet. This is usually a problem with the device and the kernel driver.
See :ref:`tablet-capabilities` for more details.

.. _faq_hwdb_changes:

------------------------------------------------------------------------------
How to apply hwdb changes
------------------------------------------------------------------------------

Sometimes users are asked to test updates to the
`udev hwdb <https://www.freedesktop.org/software/systemd/man/hwdb.html>`_
or patches that include a change to the hwdb. See :ref:`hwdb` for
details on the hwdb and how to modify it locally.

.. note:: As of libinput 1.12, libinput-specific properties are now stored in
	the :ref:`device-quirks` system. There are no libinput-specific hwdb
	entries anymore and any changes to the hwdb must be merged into the
	systemd repository.

.. _faq_timer_offset:

------------------------------------------------------------------------------
What causes the "your system is too slow" warning?
------------------------------------------------------------------------------

libinput relies on the caller to call **libinput_dispatch()** whenever data is
available. **libinput_dispatch()** will process the state of all devices,
including some time-sensitive features (e.g. palm detection, tap-to-click,
disable-while-typing, etc.).

If the time between the event and the call to **libinput_dispatch()**
is excessive, those features may not work correctly. For example, a delay in
touch event processing may cause wrong or missing tap-to-click events or
a palm may not be detected correctly.

When this warning appears, it simply means that too much time has passed
between the event occurring and the current time. In almost all cases this
is an indication of the caller being overloaded and not handling events as
speedily as required.

The warning has no immediate effect on libinput's behavior but some of the
functionality that relies on the timer may be impeded. This is not a bug in
libinput. libinput does not control how quickly **libinput_dispatch()** is
called.

.. _faq_wayland:

------------------------------------------------------------------------------
Is libinput required for Wayland?
------------------------------------------------------------------------------

Technically - no. But for your use-case - probably.

Wayland is a display server communication protocol. libinput is a low-level
library to simplify handling input devices and their events. They have no
direct connection. As a technical analogy, the question is similar to "is
glibc required for HTTP", or (stretching the analogy a bit further) "Is a
pen required to write English". No, it isn't.

You can use libinput without a Wayland compositor, you can write a Wayland
compositor without libinput. On most major distributions, libinput is the
standard input stack used with the X.Org X server through the
xf86-input-libinput driver.

So why "for your use-case - probably"?  All general-purpose Wayland
compositors use libinput for their input stack. Wayland compositors that
are more specialized (e.g. in-vehicle infotainment or IVI) can handle input
devices directly but the compositor you want to use
on your desktop needs an input stack that is more complex. And right now,
libinput is the only input stack that exists for this use-case.

.. _faq_separate_contexts:

------------------------------------------------------------------------------
Can I write a program to make libinput do $FOO
------------------------------------------------------------------------------

A common question is whether it's possible to write a program that can change
libinput's behavior - specifically the libinput that is used inside the
compositor. This indicates a misunderstanding of how libinput works:
libinput is a library that converts kernel events into libinput events, much
like ``sed`` reads data in, modifies it, and provides it to stdout.

If ``sed`` is used by a shell-script, that script has full control over how
``sed`` processes data. In this analogy, ``sed`` is libinput and the
shell script is the compositor. It is not possible to write a program
to modify the behavior of the ``sed`` instance used inside that shell script

Writing a program that uses libinput is akin to writing a new script that
invoke ``sed``. It will not have any effect on the original ``sed`` instance.

The only way to modify libinput's behavior is to use the configuration options
exposed by the respective compositor. Those affect the libinput context inside
the compositor and thus have an effect on the input device behavior.

------------------------------------------------------------------------------
Can I configure scroll speed?
------------------------------------------------------------------------------

No, or at least, not as a libinput option.

When using a mouse, libinput notifies callers about physical scroll wheel
movement. When using another device, libinput notifies scroll in scroll units.

It is up to the caller to transform those events into a number of pixels to
scroll and, if desired, provide a way to adjust scroll speed.

This transformation cannot be done in libinput because it may depend on context
only known by the caller. For example, a caller may want to scroll faster
depending on how many pages a document has or depending on the widget that
receives the scroll events.
