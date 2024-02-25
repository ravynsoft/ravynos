.. _motion_normalization:

==============================================================================
Normalization of relative motion
==============================================================================

Most relative input devices generate input in so-called "mickeys". A
mickey is in device-specific units that depend on the resolution
of the sensor. Most optical mice use sensors with 1000dpi resolution, but
some devices range from 100dpi to well above 8000dpi.

Without a physical reference point, a relative coordinate cannot be
interpreted correctly. A delta of 10 mickeys may be a millimeter of
physical movement or 10 millimeters, depending on the sensor. This
affects pointer acceleration in libinput and interpretation of relative
coordinates in callers.

libinput does partial normalization of relative input. For devices with a
resolution of 1000dpi and higher, motion events are normalized to a default
of 1000dpi before pointer acceleration is applied. As a result, devices with
1000dpi and above feel the same.

Devices below 1000dpi are not normalized (normalization of a 1-device unit
movement on a 400dpi mouse would cause a 2.5 pixel movement). Instead,
libinput applies a dpi-dependent acceleration function. At low speeds, a
1-device unit movement usually translates into a 1-pixel movements. As the
movement speed increases, acceleration is applied - at high speeds a low-dpi
device will roughly feel the same as a higher-dpi mouse.

The reason for the normalization is convenience: a caller can assume that a
delta of 1 should result in a movement of 1 pixel on a traditional
(low-dpi) screen. On screens with high resolutions, the caller must scale
according to the UI scale factors.

This normalization only applies to accelerated coordinates, unaccelerated
coordinates are left in device-units. It is up to the caller to interpret
those coordinates correctly.

.. _motion_normalization_touchpad:

------------------------------------------------------------------------------
Normalization of touchpad coordinates
------------------------------------------------------------------------------

Touchpads may have a different resolution for the horizontal and vertical
axis. Interpreting coordinates from the touchpad without taking resolution
into account results in uneven motion.

libinput scales unaccelerated touchpad motion to the resolution of the
touchpad's x axis, i.e. the unaccelerated value for the y axis is:
``y = (x / resolution_x) * resolution_y``.

.. _motion_normalization_tablet:

------------------------------------------------------------------------------
Normalization of tablet coordinates
------------------------------------------------------------------------------

See :ref:`tablet-relative-motion`

.. _motion_normalization_customization:

------------------------------------------------------------------------------
Setting custom DPI settings
------------------------------------------------------------------------------

Devices usually do not advertise their resolution and libinput relies on
the udev property **MOUSE_DPI** for this information. This property is usually
set via the
`udev hwdb <http://cgit.freedesktop.org/systemd/systemd/tree/hwdb/70-mouse.hwdb>`_.
The ``mouse-dpi-tool`` utility provided by
`libevdev <https://freedesktop.org/wiki/Software/libevdev/>`_ should be
used to measure a device's resolution.

The format of the property for single-resolution mice is: ::

          MOUSE_DPI=resolution@frequency

The resolution is in dots per inch, the frequency in Hz.
The format of the property for multi-resolution mice may list multiple
resolutions and frequencies: ::

          MOUSE_DPI=r1@f1 *r2@f2 r3@f3

The default frequency must be pre-fixed with an asterisk.

For example, these two properties are valid: ::

          MOUSE_DPI=800@125
          MOUSE_DPI=400@125 800@125 *1000@500 5500@500

The behavior for a malformed property is undefined. If the property is
unset, libinput assumes the resolution is 1000dpi.

Note that HW does not usually provide information about run-time
resolution changes, libinput will thus not detect when a resolution
changes to the non-default value.
