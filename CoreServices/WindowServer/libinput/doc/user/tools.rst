.. _tools:

==============================================================================
Helper tools
==============================================================================

libinput provides a ``libinput`` tool to query state and events. This tool
takes a subcommand as argument, similar to the **git** command. A full
explanation of the various commands available in the libinput tool is
available in the **libinput(1)** man page.

The most common tools used are:

- ``libinput list-devices``: to list locally available devices as seen by libinput,
  see :ref:`here <libinput-list-devices>`
- ``libinput list-kernel-devices``: to list locally available devices as seen by the kernel,
  see :ref:`here <libinput-list-kernel-devices>`
- ``libinput debug-events``: to monitor and debug events,
  see :ref:`here <libinput-debug-events>`
- ``libinput debug-gui``: to visualize events,
  see :ref:`here <libinput-debug-gui>`
- ``libinput record``: to record an event sequence for replaying,
  see :ref:`here <libinput-record>`
- ``libinput measure``: measure properties on a kernel device,
  see :ref:`here <libinput-measure>`
- ``libinput analyze``: analyse event recordings from a kernel device,
  see :ref:`here <libinput-analyze>`
- ``libinput quirks``: show quirks assigned to a device, see
  :ref:`here <libinput-quirks>`

Most of the tools must be run as root to have access to the kernel's
``/dev/input/event*`` device files.

.. _libinput-list-devices:

------------------------------------------------------------------------------
libinput list-devices
------------------------------------------------------------------------------

The ``libinput list-devices`` command shows information about devices
recognized by libinput and can help identifying why a device behaves
different than expected. For example, if a device does not show up in the
output, it is not a supported input device.

.. note:: This tool does **not** show your desktop's configuration, just the
          libinput built-in defaults.

::

     $ sudo libinput list-devices
     [...]
     Device:           SynPS/2 Synaptics TouchPad
     Kernel:           /dev/input/event4
     Group:            9
     Seat:             seat0, default
     Size:             97.33x66.86mm
     Capabilities:     pointer
     Tap-to-click:     disabled
     Tap drag lock:    disabled
     Left-handed:      disabled
     Nat.scrolling:    disabled
     Middle emulation: n/a
     Calibration:      n/a
     Scroll methods:   *two-finger
     Click methods:    *button-areas clickfinger
     [...]


The above listing shows example output for a touchpad. The
``libinput list-devices`` command lists general information about the device
(the kernel event node) but also the configuration options. If an option is
``n/a`` it does not exist on this device. Otherwise, the tool will show the
default configuration for this device, for options that have more than a
binary state all available options are listed, with the default one prefixed
with an asterisk (``*``). In the example above, the default click method is
button-areas but clickfinger is available.

.. note:: This tool is intended for human-consumption and may change its output
          at any time.

.. _libinput-list-kernel-devices:

------------------------------------------------------------------------------
libinput list-kernel-devices
------------------------------------------------------------------------------

The ``libinput list-kernel-devices`` command shows the devices known by **the
kernel**. This command can help identify issues when a device is not handled by
libinput.

::

    $ libinput list-kernel-devices
    /dev/input/event0:	Sleep Button
    /dev/input/event1:	Power Button
    /dev/input/event2:	Power Button
    /dev/input/event3:	Microsoft Microsoft® 2.4GHz Transceiver v9.0
    /dev/input/event4:	Microsoft Microsoft® 2.4GHz Transceiver v9.0 Mouse
    [...]


In some cases, knowing about the HID devices behind the kernel's event nodes
can be useful. To list HID devices, supply the ``--hid`` commandline flag:

::

    $ libinput list-kernel-devices --hid
    hid:
    - name:   'Logitech Gaming Mouse G303'
      id:     '046d:c080'
      driver: 'hid-generic'
      hidraw: ['/dev/hidraw6']
      evdev:  ['/dev/input/event13']

    - name:   'Logitech Gaming Mouse G303'
      id:     '046d:c080'
      driver: 'hid-generic'
      hidraw: ['/dev/hidraw7']
      evdev:  ['/dev/input/event14']

    - name:   'Microsoft Microsoft® 2.4GHz Transceiver v9.0'
      id:     '045e:07a5'
      driver: 'hid-generic'
      hidraw: ['/dev/hidraw0']
      evdev:  ['/dev/input/event3']

.. note:: This tool is intended for human-consumption and may change its output
          at any time.

.. _libinput-debug-events:

------------------------------------------------------------------------------
libinput debug-events
------------------------------------------------------------------------------
The ``libinput debug-events`` command prints events from devices and can help
to identify why a device behaves different than expected. ::

     $ sudo libinput debug-events --enable-tapping --set-click-method=clickfinger

All configuration options (enable/disable tapping,
etc.) are available as commandline arguments. To reproduce the event
sequence as your desktop session sees it, ensure that all options are turned
on or off as required. See the **libinput-debug-events(1)** man page or the
``--help`` output for information about the available options.

.. note:: When submitting a bug report, always use the ``--verbose`` flag to get
          additional information: ``libinput debug-events --verbose <other options>``

An example output from this tool may look like the snippet below. ::

     $ sudo libinput debug-events --enable-tapping --set-click-method=clickfinger
     -event2   DEVICE_ADDED     Power Button                      seat0 default group1  cap:k
     -event5   DEVICE_ADDED     Video Bus                         seat0 default group2  cap:k
     -event0   DEVICE_ADDED     Lid Switch                        seat0 default group3  cap:S
     -event1   DEVICE_ADDED     Sleep Button                      seat0 default group4  cap:k
     -event4   DEVICE_ADDED     HDA Intel HDMI HDMI/DP,pcm=3      seat0 default group5  cap:
     -event11  DEVICE_ADDED     HDA Intel HDMI HDMI/DP,pcm=7      seat0 default group6  cap:
     -event12  DEVICE_ADDED     HDA Intel HDMI HDMI/DP,pcm=8      seat0 default group7  cap:
     -event13  DEVICE_ADDED     HDA Intel HDMI HDMI/DP,pcm=9      seat0 default group8  cap:
     -event14  DEVICE_ADDED     HDA Intel HDMI HDMI/DP,pcm=10     seat0 default group9  cap:
     -event19  DEVICE_ADDED     Integrated Camera: Integrated C   seat0 default group10 cap:k
     -event15  DEVICE_ADDED     HDA Intel PCH Dock Mic            seat0 default group11 cap:
     -event16  DEVICE_ADDED     HDA Intel PCH Mic                 seat0 default group12 cap:
     -event17  DEVICE_ADDED     HDA Intel PCH Dock Headphone      seat0 default group13 cap:
     -event18  DEVICE_ADDED     HDA Intel PCH Headphone           seat0 default group14 cap:
     -event6   DEVICE_ADDED     ELAN Touchscreen                  seat0 default group15 cap:t  size 305x172mm ntouches 10 calib
     -event3   DEVICE_ADDED     AT Translated Set 2 keyboard      seat0 default group16 cap:k
     -event20  DEVICE_ADDED     SynPS/2 Synaptics TouchPad        seat0 default group17 cap:pg  size 100x76mm tap(dl off) left scroll-nat scroll-2fg-edge click-buttonareas-clickfinger dwt-on
     -event21  DEVICE_ADDED     TPPS/2 IBM TrackPoint             seat0 default group18 cap:p left scroll-nat scroll-button
     -event7   DEVICE_ADDED     ThinkPad Extra Buttons            seat0 default group19 cap:k
     -event20  POINTER_MOTION    +3.62s	  2.72/ -0.93
      event20  POINTER_MOTION    +3.63s	  1.80/ -1.42
      event20  POINTER_MOTION    +3.65s	  6.16/ -2.28
      event20  POINTER_MOTION    +3.66s	  6.42/ -1.99
      event20  POINTER_MOTION    +3.67s	  8.99/ -1.42
      event20  POINTER_MOTION    +3.68s	 11.30/  0.00
      event20  POINTER_MOTION    +3.69s	 21.32/  1.42


.. _libinput-debug-gui:

------------------------------------------------------------------------------
libinput debug-gui
------------------------------------------------------------------------------

A simple GTK-based graphical tool that shows the behavior and location of
touch events, pointer motion, scroll axes and gestures. Since this tool
gathers data directly from libinput, it is thus suitable for
pointer-acceleration testing.

.. note:: This tool does **not** use your desktop's configuration, just the
          libinput built-in defaults.

::

     $ sudo libinput debug-gui --enable-tapping


As with :ref:`libinput-debug-events`, all options must be specified on the
commandline to emulate the correct behavior.
See the **libinput-debug-gui(1)** man page or the ``--help`` output for information about
the available options.

.. _libinput-record:

------------------------------------------------------------------------------
libinput record and libinput replay
------------------------------------------------------------------------------

.. note:: For libinput versions 1.10 and older, use :ref:`evemu`.

The ``libinput record`` command records the **kernel** events from a specific
device node. The recorded sequence can be replayed with the ``libinput
replay`` command. This pair of tools is crucial to capturing bugs and
reproducing them on a developer's machine.

.. graphviz:: libinput-record.gv
	:align: center

The recorded events are **kernel events** and independent of the
libinput context. libinput does not need to be running, it does
not matter whether a user is running X.Org or Wayland or even what
version of libinput is currently running.

The use of the tools is straightforward, just run without arguments, piping
the output into a file: ::

     $ sudo libinput record > touchpad.yml
     Available devices:
     /dev/input/event0:	Lid Switch
     /dev/input/event1:	Sleep Button
     /dev/input/event2:	Power Button
     /dev/input/event3:	AT Translated Set 2 keyboard
     /dev/input/event4:	ThinkPad Extra Buttons
     /dev/input/event5:	ELAN Touchscreen
     /dev/input/event6:	Video Bus
     /dev/input/event7:	HDA Intel HDMI HDMI/DP,pcm=3
     /dev/input/event8:	HDA Intel HDMI HDMI/DP,pcm=7
     /dev/input/event9:	HDA Intel HDMI HDMI/DP,pcm=8
     /dev/input/event10:	HDA Intel HDMI HDMI/DP,pcm=9
     /dev/input/event11:	HDA Intel HDMI HDMI/DP,pcm=10
     /dev/input/event12:	HDA Intel PCH Dock Mic
     /dev/input/event13:	HDA Intel PCH Mic
     /dev/input/event14:	HDA Intel PCH Dock Headphone
     /dev/input/event15:	HDA Intel PCH Headphone
     /dev/input/event16:	Integrated Camera: Integrated C
     /dev/input/event17:	SynPS/2 Synaptics TouchPad
     /dev/input/event18:	TPPS/2 IBM TrackPoint
     Select the device event number: 17
     /dev/input/event17 recording to stdout

Without arguments, ``libinput record`` displays the available devices and lets
the user select one. Supply the number (17 in this case for
``/dev/input/event17``) and the tool will print the device information and
events to the file it is redirected to. More arguments are available, see
the **libinput-record(1)** man page.

.. note:: When reproducing a bug that crashes libinput, run inside ``screen`` or
          ``tmux``.

Reproduce the bug, ctrl+c and attach the output file to a bug report.
For data protection, ``libinput record`` obscures key codes by default, any
alphanumeric key shows up as letter "a".

.. warning:: The longer the recording, the harder it is to identify the event
	     sequence triggering the bug. Please keep the event sequence as
	     short as possible.

The recording can be replayed with the ``libinput replay`` command: ::

     $ sudo libinput replay touchpad.yml
     SynPS/2 Synaptics TouchPad: /dev/input/event19
     Hit enter to start replaying


``libinput replay`` creates a new virtual device based on the description in
the log file. Hitting enter replays the event sequence once and the tool
stops once all events have been replayed. Hitting enter again replays the
sequence again, Ctrl+C stops it and removes the virtual device.

Users are advised to always replay a recorded event sequence to ensure they
have captured the bug.

More arguments are available, see the **libinput-record(1)** and
**libinput-replay(1)** man pages.

.. _libinput-record-autorestart:

..............................................................................
libinput record's autorestart feature
..............................................................................

``libinput record`` often collects thousands of events per minute. However,
the output of ``libinput record`` usually needs to be visually inspected
or replayed in realtime on a developer machine. It is thus imperative that
the event log is kept as short as possible.

For bugs that are difficult to reproduce use
``libinput record --autorestart=2 --output-file=recording.yml``.
All events will be recorded to a file named
``recording.yml.<current-date-and-time>`` and whenever the device does not
send events for 2 seconds, a new file is created. This helps to keep
individual recordings short.

To use the ``--autorestart`` option correctly:

- run ``libinput record --autorestart=2 --output-file=<somefilename>.yml``.
  You may provide a timeout other than 2 if needed.
- use the device to reproduce the bug, pausing frequently for 2s and longer
  to rotate the logs
- when the bug triggers, **immediately stop using the device** and wait
  several seconds for the log to rotate
- Ctrl+C the ``libinput record`` process without using the device
  again. Attach the **last recording** to the bug report.

If you have to use the recorded device to stop ``libinput record`` (e.g. to
switch windows), remember that this will cause a new recording to be
created. Thus, attach the **second-to-last recording** to the bug report
because this one contains the bug trigger.

.. _libinput-record-multiple:

..............................................................................
Recording multiple devices at once
..............................................................................

In some cases, an interaction between multiple devices is the cause for a
specific bug. For example, a touchpad may not work in response to keyboard
events. To accurately reproduce this sequence, the timing between multiple
devices must be correct and we need to record the events in one go.

``libinput record`` has a ``--multiple`` argument to record multiple devices at
once. Unlike the normal invocation, this one requires a number of arguments: ::

     $ sudo libinput record --multiple --output-file=touchpad-bug.yml /dev/input/event17 /dev/input/event3
     recording to 'touchpad-bug.yml'

As seen above, a user must specify ``--multiple`` and the ``--output-file``.
Finally, all devices to be recorded must be specified on the commandline as
well.

Replaying events is the same as for a single recording: ::

     $ sudo libinput replay touchpad-bug.yml

.. _libinput-measure:

------------------------------------------------------------------------------
Measuring device properties with libinput measure
------------------------------------------------------------------------------

The ``libinput measure`` tool is a multiplexer for various sub-tools that can
measure specific properties on the device. These tools generally measure one
thing and one thing only and their usage is highly specific to the tool.
Please see the **libinput-measure(1)** man page for information about what
tools are available and the man page for each respective tool.

.. _libinput-analyze:

------------------------------------------------------------------------------
Analyzing device events with libinput analyze
------------------------------------------------------------------------------

The ``libinput analyze`` tool is a multiplexer for various sub-tools that
can analyze input events previously recorded from a device.

Please see the **libinput-analyze(1)** man page for information about what
tools are available and the man page for each respective too.


.. _libinput-quirks:

------------------------------------------------------------------------------
Listing quirks assigned to a device
------------------------------------------------------------------------------

The ``libinput quirks`` tool can show quirks applied for any given device. ::

     $ libinput quirks list /dev/input/event0
     AttrLidSwitchReliability=unreliable

If the tool's output is empty, no quirk is applied. See :ref:`device-quirks`
for more information.
