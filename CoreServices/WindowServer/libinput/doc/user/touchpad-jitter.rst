.. _touchpad_jitter:

==============================================================================
Touchpad jitter
==============================================================================

Touchpad jitter describes random movement by a few pixels even when the
user's finger is unmoving.

libinput has a mechanism called a **hysteresis** to avoid that jitter. When
active, movement with in the **hysteresis margin** is discarded. If the
movement delta is larger than the margin, the movement is passed on as
pointer movement. This is a simplified summary, developers should
read the implementation of the hysteresis in ``src/evdev.c``.

libinput uses the kernel ``fuzz`` value to determine the size of the
hysteresis. Users should override this with a udev hwdb entry where the
device itself does not provide the correct value.

.. _touchpad_jitter_fuzz_override:

------------------------------------------------------------------------------
Overriding the hysteresis margins
------------------------------------------------------------------------------

libinput provides the debugging tool ``libinput measure fuzz`` to help edit or
test a fuzz value. This tool is interactive and provides a udev hwdb entry
that matches the device. To check if a fuzz is currently present, simply run
without arguments or with the touchpad's device node:


::

     $ sudo libinput measure fuzz
     Using Synaptics TM2668-002: /dev/input/event17
       Checking udev property... not set
       Checking axes... x=16 y=16


In the above output, the axis fuzz is set to 16. To set a specific fuzz, run
with the ``--fuzz=<value>`` argument.


::

     $ sudo libinput measure fuzz --fuzz=8


The tool will attempt to construct a hwdb file that matches your touchpad
device. Follow the printed prompts.

In the ideal case, the tool will provide you with a file that can be
submitted to the systemd repo for inclusion.

However, hwdb entry creation is difficult to automate and it's likely
that the tools fails in doing so, especially if an existing entry is already
present.

Below is the outline of what a user needs to do to override a device's fuzz
value in case the ``libinput measure fuzz`` tool fails.

Check with ``udevadm info /sys/class/input/eventX`` (replace your device node
number) whether an existing hwdb override exists. If the ``EVDEV_ABS_``
properties are present, the hwdb override exists. Find the file that
contains that entry, most likely in ``/etc/udev/hwdb.d`` or
``/usr/lib/udev/hwdb.d``.

The content of the property is a set of values in the format
``EVDEV_ABS_00=min:max:resolution:fuzz``. You need to set the ``fuzz`` part,
leaving the remainder of the property as-is. Values may be empty, e.g. a
property that only sets resolution and fuzz reads as ``EVDEV_ABS_00=::32:8``.

If no properties exist, your hwdb.entry should look approximately like this:

::

     evdev:name:Synaptics TM2668-002:dmi:*:svnLENOVO*:pvrThinkPadT440s*:
      EVDEV_ABS_00=:::8
      EVDEV_ABS_01=:::8
      EVDEV_ABS_35=:::8
      EVDEV_ABS_36=:::8


Substitute the ``name`` field with the device name (see the output of
``libinput measure fuzz`` and the DMI match content with your hardware. See
:ref:`hwdb_modifying` for details.

Once the hwdb entry has been modified, added, or created,
:ref:`reload the hwdb <hwdb_reloading>`. Once reloaded, :ref:`libinput-record`
"libinput record" should show the new fuzz value for the axes.

Restart the host and libinput should pick up the revised fuzz values.

.. _kernel_fuzz:

------------------------------------------------------------------------------
Kernel fuzz
------------------------------------------------------------------------------

A fuzz set on an absolute axis in the kernel causes the kernel to apply
hysteresis-like behavior to the axis. Unfortunately, this behavior leads to
inconsistent deltas. To avoid this, libinput sets the kernel fuzz on the
device to 0 to disable this kernel behavior but remembers what the fuzz was
on startup. The fuzz is stored in the ``LIBINPUT_FUZZ_XX`` udev property, on
startup libinput will check that property as well as the axis itself.
