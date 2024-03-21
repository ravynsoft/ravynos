.. _reporting_bugs:

==============================================================================
Reporting bugs
==============================================================================

A new bug can be filed here:
https://gitlab.freedesktop.org/libinput/libinput/issues/new

.. hint:: libinput has lots of users but very few developers. It is in your
	   own interest to follow the steps here precisely to ensure your bug can be
	   dealt with efficiently.

When reporting bugs against libinput, you will need:

- a reliable :ref:`reproducer <reporting_bugs_reproducer>` for the bug
- a :ref:`recording <libinput-record>` of the device while the bug is reproduced
- device-specific information, see

     - :ref:`reporting_bugs_touchpad`
     - :ref:`reporting_bugs_mouse`
     - :ref:`reporting_bugs_keyboard`
     - :ref:`reporting_bugs_trackpoint`
     - :ref:`reporting_bugs_other`

- the :ref:`libinput version <reporting_bugs_version>` you are on.
- the :ref:`configuration options <reporting_bugs_options>` you have set
- a `gitlab account <https://gitlab.freedesktop.org/users/sign_in>`_

Stay technical, on-topic, and keep the description concise.

.. _reporting_bugs_version:

------------------------------------------------------------------------------
Obtaining the libinput version
------------------------------------------------------------------------------

If your libinput version is older than the current stable branch, please try
the latest version. If you run a distribution-provided
libinput, use the package manager to get the **full** package name and
version of libinput, e.g.

- ``rpm -q libinput``
- ``dpkg -s libinput10``

If you run a self-compiled version of libinput provide the git commit you
have built or the tarball name.

As a last resort, use ``libinput --version``

.. _reporting_bugs_reproducer:

------------------------------------------------------------------------------
Reproducing bugs
------------------------------------------------------------------------------

Try to identify the bug by reproducing it reliably. Bugs without a
reliable reproducer will have lowest priority. The more specific a bug
description and reproducer is, the easier it is to fix.

Try to replicate the series of events that lead to the bug being triggered.
Narrow it down until you have a reliable sequence that can trigger the bug.
For the vast majority of bugs you should not take longer than 5 seconds or
three interactions (clicks, touches, taps, ...) with the device to
reproduce. If it takes longer than that, you can narrow it down further.

Once you can reproduce it, use the :ref:`libinput-debug-events` helper
tool::

 $> libinput debug-events --verbose

The output is textual and can help identify whether the bug is in libinput
at all. Note that any configuration options you have set must be specified
on the commandline, see the :ref:`libinput-debug-events`
man page. Use the ``--verbose`` flag to get more information about how
libinput processes events.

If the bug cannot be reproduced with the :ref:`libinput-debug-events` helper,
even with the correct configuration options set, it is likely not a bug in
libinput.

.. _reporting_bugs_options:

------------------------------------------------------------------------------
libinput configuration settings
------------------------------------------------------------------------------

libinput has a number of device-specific default configuration settings that
may differ from the ones your desktop environment picks by default. You may
have changed some options in a settings panel or in an the xorg.conf snippet
yourself.

You must provide these options in the bug report, otherwise a developer
reproducing the issue may not be able to do so.

If you are on X11, the current settings can be can be obtained with
``xinput list-props "your device name"``. Use ``xinput list`` to
obtain the device name.

If you are on Wayland, provide a manual summary of the options you have
changed from the default (e.g. "I enabled tap-to-click").

.. _reporting_bugs_touchpad:

------------------------------------------------------------------------------
Reporting touchpad bugs
------------------------------------------------------------------------------

When you file a bug, please attach the following information:

- a virtual description of your input device, see :ref:`libinput-record`.
  This is the most important piece of information, do not forget it!
- the output from udevadm info, see :ref:`udev_info`.
- the vendor model number of your laptop (e.g. "Lenovo Thinkpad T440s")
- and the content of ``/sys/class/dmi/id/modalias``.
- run the ``touchpad-edge-detector`` tool (provided by libevdev) and verify
  that the ranges and sizes it prints match the touchpad (up to 5mm
  difference is ok)

If you are reporting a bug related to button event generation:

- does your touchpad have (separate) physical hardware buttons or is the
  whole touchpad clickable?
- Are you using software buttons or clickfinger? See :ref:`clickpad_softbuttons`.
- Do you have :ref:`tapping` enabled?

.. _reporting_bugs_mouse:

------------------------------------------------------------------------------
Reporting mouse bugs
------------------------------------------------------------------------------

When you file a bug, please attach the following information:

- a virtual description of your input device, see :ref:`libinput-record`.
  This is the most important piece of information, do not forget it!
- the vendor model number of the device (e.g. "Logitech M325")
- the output from udevadm info, see :ref:`udev_info`.

If the bug is related to the :ref:`speed of the mouse <motion_normalization_customization>`:

- the resolution of the mouse as specified by the vendor (in DPI)
- the output of the ``mouse-dpi-tool`` (provided by libevdev)

.. _reporting_bugs_keyboard:

------------------------------------------------------------------------------
Reporting keyboard bugs
------------------------------------------------------------------------------

Is your bug related to a keyboard layout? libinput does not handle keyboard
layouts and merely forwards the physical key events. File the bug with your
desktop environment instead (e.g. GNOME, KDE, ...), that's most likely where
the issue is.

When you file a bug, please attach the following information:

- a virtual description of your input device, see :ref:`libinput-record`.
  This is the most important piece of information, do not forget it!

.. _reporting_bugs_trackpoint:

------------------------------------------------------------------------------
Reporting trackpoint bugs
------------------------------------------------------------------------------

When you file a bug, please attach the following information:

- a virtual description of your input device, see :ref:`libinput-record`.
  This is the most important piece of information, do not forget it!
- the vendor model number of the device (e.g. "Logitech M325")
- the output from udevadm info, see :ref:`udev_info`.
- the sensitivity of the trackpoint if it exists (adjust the event node number as needed): ::

     $ cat /sys/class/input/event17/device/device/sensitivity


.. _reporting_bugs_other:

------------------------------------------------------------------------------
All other devices
------------------------------------------------------------------------------

When you file a bug, please attach the following information:

- a virtual description of your input device, see :ref:`libinput-record`.
  This is the most important piece of information, do not forget it!
- the vendor model number of the device (e.g. "Sony Plastation3 controller")

.. _udev_info:

------------------------------------------------------------------------------
udev information for the device
------------------------------------------------------------------------------

In many cases, we require the udev properties assigned to the device to
verify whether device-specific quirks were applied. This can be obtained
with ``udevadm info /sys/class/input/eventX``, with the correct event
node for your device. An example output is below: ::

     $ udevadm info /sys/class/input/event4
     P: /devices/platform/i8042/serio1/input/input5/event4
     N: input/event4
     E: DEVNAME=/dev/input/event4
     E: DEVPATH=/devices/platform/i8042/serio1/input/input5/event4
     E: EVDEV_ABS_00=::41
     E: EVDEV_ABS_01=::37
     E: EVDEV_ABS_35=::41
     E: EVDEV_ABS_36=::37
     E: ID_INPUT=1
     E: ID_INPUT_HEIGHT_MM=66
     E: ID_INPUT_TOUCHPAD=1
     E: ID_INPUT_WIDTH_MM=97
     E: MAJOR=13
     E: MINOR=68
     E: SUBSYSTEM=input
     E: USEC_INITIALIZED=5463031


.. _evemu:

------------------------------------------------------------------------------
Recording devices with evemu
------------------------------------------------------------------------------

.. warning:: Where available, the :ref:`libinput-record` tools should be used instead
             of evemu

`evemu-record <https://www.freedesktop.org/wiki/Evemu/>`_ records the
device capabilities together with the event stream from the kernel. On our
side, this allows us to recreate a virtual device identical to your device
and re-play the event sequence, hopefully triggering the same bug.

evemu-record takes a ``/dev/input/eventX`` event node, but without arguments
it will simply show the list of devices and let you select: ::

     $ sudo evemu-record > scroll.evemu
     Available devices:
     /dev/input/event0:	Lid Switch
     /dev/input/event1:	Sleep Button
     /dev/input/event2:	Power Button
     /dev/input/event3:	AT Translated Set 2 keyboard
     /dev/input/event4:	SynPS/2 Synaptics TouchPad
     /dev/input/event5:	Video Bus
     /dev/input/event6:	ELAN Touchscreen
     /dev/input/event10:	ThinkPad Extra Buttons
     /dev/input/event11:	HDA Intel HDMI HDMI/DP,pcm=3
     /dev/input/event12:	HDA Intel HDMI HDMI/DP,pcm=7
     /dev/input/event13:	HDA Intel HDMI HDMI/DP,pcm=8
     /dev/input/event14:	HDA Intel PCH Dock Mic
     /dev/input/event15:	HDA Intel PCH Mic
     /dev/input/event16:	HDA Intel PCH Dock Headphone
     /dev/input/event17:	HDA Intel PCH Headphone
     /dev/input/event18:	Integrated Camera
     /dev/input/event19:	TPPS/2 IBM TrackPoint
     Select the device event number [0-19]:


Select the device that triggers the issue, then reproduce the bug and Ctrl+C
the process. The resulting recording, ("scroll.evemu" in this example) will
contain the sequence required to reproduce the bug. If the bug fails to
reproduce during recording, simply Ctrl+C and restart evemu-record.
Always start the recording from a neutral state, i.e. without any buttons or
keys down, with the position of the device in the neutral position, without
touching the screen/touchpad.

.. note:: The longer the recording, the harder it is to identify the event
          sequence triggering the bug. Please keep the event sequence as short
          as possible.

To verify that the recording contains the bug, you can replay it on your
device. For example, to replay the sequence recorded in the example above: ::

     $ sudo evemu-play /dev/input/event4 < scroll.evemu


If the bug is triggered by replaying on your device, attach the recording to
the bug report.

libinput does not affect the evemu recording. libinput and evemu talk
directly to the kernel's device nodes. An evemu recording is not
influenced by the libinput version or whether a libinput context is
currently active.

.. graphviz:: evemu.gv

.. _fixed_bugs:

------------------------------------------------------------------------------
My bug was closed as fixed, what now?
------------------------------------------------------------------------------

libinput's policy on closing bugs is: once the fix for a given bug is on git
master, the bug is considered fixed and the gitlab issue will be closed
accordingly.

Of course, unless you actually run git master, the bug will continue to
affect you on your local machine. You are most likely running the
distribution's package and you will need to wait until the distribution has
updated its package accordingly.

.. warning:: Do not re-open a bug just because it hasn't trickled down to
             your distribution's package version yet.

Whether the bug fix ends up in your distribution depends on a number of
things. Any given bug fix **may** be cherry-picked into the current stable
branch, depending on its severity, impact, and likelihood to cause
regressions. Once cherry-picked it will land in the next stable branch
release. These are usually a few weeks apart.

.. warning:: Do not re-open a bug because it wasn't picked into a stable branch
             release or because your distribution didn't update to the latest stable
             branch release.

Stable branches are usually discontinued when the next release comes out.

Your distribution may pick a patch up immediately and ship the fix
even before the next stable branch update is released. For example, Fedora
does this frequently.

.. hint:: If a bug needs to be fixed urgently, file a bug in your
          distribution's bug tracker.

Patches on git master will end up in the next libinput release. Once your
distribution updates to that release, your local libinput version will
contain the fix.

.. warning:: Do not re-open a bug because your distribution didn't update to
             the release.

You can always run libinput from git master (see :ref:`building_libinput`).
Even while in development, libinput is very stable so this option isn't as
scary as it may sounds.

.. _reporting_bugs_reopen:

..............................................................................
When is it ok to re-open a fixed bug?
..............................................................................

Any time the bug was considered fixed but it turns out that the fix is
insufficient and/or causes a regression.

However, if the regression is in behavior unrelated to the fix itself it is
usually better to file a new bug to reduce the noise. For example, if a fix
to improve tapping breaks two-finger scrolling behavior, you should file a
new bug but reference the original bug.

.. _reporting_bugs_tags:

------------------------------------------------------------------------------
Gitlab issue tracker tags
------------------------------------------------------------------------------

The gitlab issue tracker allows developers to add tags to bugs to classify
them.

- **being worked on**: someone is currently working on this feature. This
  tag is used for features that will take a long time to implement fully and
  prevents others from having to duplicate the work. Do reach out and ask if
  help and/or further testing is needed.
- **bug**: issue is confirmed to be a bug
- **cantfix**: for technical reasons, this bug cannot be fixed, or at least
  it cannot be fixed in libinput.
- **enhancement**: this issue describes a future feature, not a bug.
- **help needed**: this issue requires someone outside the libinput core
  developer team to implement it. It is unlikely to be implemented
  without someone stepping up to do the work. If you do see this tag, do ask
  for guidance on how to implement it.
- **hw issue**: an issue that affects a specific device and is a hardware
  bug, not a software bug. Often these needs to be worked around in libinput
  but there are cases where a hw issue ends up as *cantfix*.
- **janitor**: a cleanup task that does not substantially affect how
  libinput works. These are usually good bugs for newcomers to start on.
- **kernel**: this issue is a kernel bug, not a libinput bug. Often closed
  as *cantfix* of *wontfix* as we wait for the kernel to address the issue
  instead.
- **needs triage**: bug has not yet been confirmed by a core developer.
- **not our bug**: the issue is in some other component of the stack and
  needs to be addressed there.
- **please test**: a fix is available but not yet merged and should be
  tested by the reporter or others affected by the issue.
- **quirk**: this is issue needs :ref:`device-quirks` to be fixed
- **regression**: the issue is a regression to previous versions of
  libinput. These issues get priorities.
- **waiting on reporter**: some more information is required from the
  reporter and the issue cannot be fixed until the issue has been provided.
  Where a bug is left in this state for too long, the bug will be closed as
  *cantfix*.
- **wontfix**: this issue will not get fixed. This tag is usually assigned
  to feature requests that are outside the scope of libinput or would put an
  unreasonable maintenance burdern on the maintainers.

These tags are high-level categories only, always look for the comments in
the issue to get further details.
