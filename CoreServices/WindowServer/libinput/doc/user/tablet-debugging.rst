.. _tablet-debugging:

==============================================================================
Debugging tablet issues
==============================================================================

.. _tablet-capabilities:

------------------------------------------------------------------------------
Required tablet capabilities
------------------------------------------------------------------------------

To handle a tablet correctly, libinput requires a set of capabilities
on the device. When these capabilities are missing, libinput ignores the
device and prints an error to the log. This error messages reads

::

     missing tablet capabilities: xy pen btn-stylus resolution. Ignoring this device.

or in older versions of libinput simply:

::

     libinput bug: device does not meet tablet criteria. Ignoring this device.


When a tablet is rejected, it is usually possible to verify the issue with
the ``libinput record`` tool.

- **xy** indicates that the tablet is missing the ``ABS_X`` and/or ``ABS_Y``
  axis. This indicates that the device is mislabelled and the udev tag
  ``ID_INPUT_TABLET`` is applied to a device that is not a tablet.
  A bug should be filed against `systemd <http://github.com/systemd/systemd>`__.
- **pen** or **btn-stylus** indicates that the tablet does not have the
  ``BTN_TOOL_PEN`` or ``BTN_STYLUS`` bit set. libinput requires either or both
  of them to be present. This indicates a bug in the kernel driver
  or the HID descriptors of the device.
- **resolution** indicates that the device does not have a resolution set
  for the x and y axes. This can be fixed with a hwdb entry, locate and read
  the `60-evdev.hwdb
  <https://github.com/systemd/systemd/blob/main/hwdb.d/60-evdev.hwdb>`__ file
  on your machine and file a pull request with the fixes against
  `systemd <https://github.com/systemd/systemd/>`__.
