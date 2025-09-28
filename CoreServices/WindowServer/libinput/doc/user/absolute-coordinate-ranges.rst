.. _absolute_coordinate_ranges:

==============================================================================
Coordinate ranges for absolute axes
==============================================================================

libinput requires that all touchpads provide a correct axis range and
resolution. These are used to enable or disable certain features or adapt
the interaction with the touchpad. For example, the software button area is
narrower on small touchpads to avoid reducing the interactive surface too
much. Likewise, palm detection works differently on small touchpads as palm
interference is less likely to happen.

Touchpads with incorrect axis ranges generate error messages
in the form:
<blockquote>
Axis 0x35 value 4000 is outside expected range [0, 3000]
</blockquote>

This error message indicates that the ABS_MT_POSITION_X axis (i.e. the x
axis) generated an event outside the expected range of 0-3000. In this case
the value was 4000.
This discrepancy between the coordinate range the kernels advertises vs.
what the touchpad sends can be the source of a number of perceived
bugs in libinput.

.. _absolute_coordinate_ranges_fix:

------------------------------------------------------------------------------
Measuring and fixing touchpad ranges
------------------------------------------------------------------------------

To fix the touchpad you need to:

#. measure the physical size of your touchpad in mm
#. run the ``libinput measure touchpad-size`` tool
#. verify the hwdb entry provided by this tool
#. test locally
#. send a patch to the `systemd project <https://github.com/systemd/systemd>`_.

Detailed explanations are below.

.. note:: ``libinput measure touchpad-size`` was introduced in libinput
	  1.16. For earlier versions, use `libevdev <http://freedesktop.org/wiki/Software/libevdev/>`_'s
	  ``touchpad-edge-detector`` tool.


The ``libinput measure touchpad-size`` tool is an interactive tool. It must
be called with the physical dimensions of the touchpad in mm. In the example
below, we use 100mm wide and 55mm high. The tool will find the touchpad device
automatically.

::

     $> sudo libinput measure touchpad-size 100x55
     Using "Touchpad SynPS/2 Synaptics TouchPad": /dev/input/event4

     Kernel specified touchpad size: 99.7x75.9mm
     User specified touchpad size:   100.0x55.0mm

     Kernel axis range:   x [1024..5112], y [2024..4832]
     Detected axis range: x [   0..   0], y [   0..   0]

     Move one finger along all edges of the touchpad
     until the detected axis range stops changing.

     ...

Move the finger around until the detected axis range matches the data sent
by the device. ``Ctrl+C`` terminates the tool and prints a
suggested hwdb entry. ::

    ...
    Kernel axis range:   x [1024..5112], y [2024..4832]
    ^C
    Detected axis range: x [2072..4880], y [2159..4832]
    Resolutions calculated based on user-specified size: x 28, y 49 units/mm

    Suggested hwdb entry:
    Note: the dmi modalias match is a guess based on your machine's modalias:
      dmi:bvnLENOVO:bvrGJET72WW(2.22):bd02/21/2014:svnLENOVO:pn20ARS25701:pvrThinkPadT440s:rvnLENOVO:rn20ARS25701:rvrSDK0E50512STD:cvnLENOVO:ct10:cvrNotAvailable:
    Please verify that this is the most sensible match and adjust if necessary.
    -8<--------------------------
    # Laptop model description (e.g. Lenovo X1 Carbon 5th)
    evdev:name:SynPS/2 Synaptics TouchPad:dmi:*svnLENOVO:*pvrThinkPadT440s*
     EVDEV_ABS_00=2072:4880:28
     EVDEV_ABS_01=2159:4832:49
     EVDEV_ABS_35=2072:4880:28
     EVDEV_ABS_36=2159:4832:49
    -8<--------------------------
    Instructions on what to do with this snippet are in /usr/lib/udev/hwdb.d/60-evdev.hwdb


If there are discrepancies between the coordinate range the kernels
advertises and what what the touchpad sends, the hwdb entry should be added to the
``60-evdev.hwdb`` file provided by the `systemd project <https://github.com/systemd/systemd>`_.
An example commit can be found
`here <https://github.com/systemd/systemd/commit/26f667eac1c5e89b689aa0a1daef6a80f473e045>`_.

The ``libinput measure touchpad-size`` tool attempts to provide the correct
dmi match but it does require user verification.

In most cases the dmi match can and should be trimmed to the system vendor (``svn``)
and the product version (``pvr``) or product name (``pn``), with everything else
replaced by a wildcard (``*``). In the above case, the match string is:

::

     evdev:name:SynPS/2 Synaptics TouchPad:dmi:*svnLENOVO:*pvrThinkPadT440s*

As a general rule: for Lenovo devices use ``pvr`` and for all others use
``pn``.

.. note:: hwdb match strings only allow for alphanumeric ascii characters. Use a
	wildcard (* or ?, whichever appropriate) for special characters.

The actual axis overrides are in the form:

::

     # axis number=min:max:resolution
      EVDEV_ABS_00=2072:4880:28

or, if the range is correct but the resolution is wrong

::

     # axis number=::resolution
      EVDEV_ABS_00=::28


Note the leading single space. The axis numbers are in hex and can be found
in ``linux/input-event-codes.h``. For touchpads ``ABS_X``, ``ABS_Y``,
``ABS_MT_POSITION_X`` and ``ABS_MT_POSITION_Y`` are required.

.. note:: The touchpad's ranges and/or resolution should only be fixed when
	there is a significant discrepancy. A few units do not make a
	difference and a resolution that is off by 2 or less usually does
	not matter either.

Once a match and override rule has been found, follow the instructions at
the top of the
`60-evdev.hwdb <https://github.com/systemd/systemd/blob/main/hwdb.d/60-evdev.hwdb>`_
file to save it locally and trigger the udev hwdb reload. Rebooting is
always a good idea. If the match string is correct, the new properties will
show up in the
output of

::

        udevadm info /sys/class/input/event4


Adjust the command for the event node of your touchpad.
A udev builtin will apply the new axis ranges automatically.

When the axis override is confirmed to work, please submit it as a pull
request to the `systemd project <https://github.com/systemd/systemd>`_.
