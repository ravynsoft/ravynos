.. _absolute_axes:

==============================================================================
Absolute axes
==============================================================================

Devices with absolute axes are those that send positioning data for an axis in
a device-specific coordinate range, defined by a minimum and a maximum value.
Compare this to relative devices (e.g. a mouse) that can only detect
directional data, not positional data.

libinput supports three types of devices with absolute axes:

 - multi-touch screens
 - single-touch screens
 - :ref:`graphics tablets <tablet-support>`

Touchpads are technically absolute devices but libinput converts the axis values
to directional motion and posts events as relative events. Touchpads do not count
as absolute devices in libinput.

For all absolute devices in libinput, the default unit for x/y coordinates is
in mm off the top left corner on the device, or more specifically off the
device's sensor. If the device is physically rotated from its natural
position and this rotation was communicated to libinput (e.g. by setting
the device left-handed),
the coordinate origin is the top left corner in the current rotation.

.. _absolute_axes_handling:

------------------------------------------------------------------------------
Handling of absolute coordinates
------------------------------------------------------------------------------

In most use-cases, absolute input devices are mapped to a single screen. For
direct input devices such as touchscreens the aspect ratio of the screen and
the device match. Mapping the input device position to the output position is
thus a simple mapping between two coordinates. libinput provides the API for
this with

- **libinput_event_pointer_get_absolute_x_transformed()** for pointer events
- **libinput_event_touch_get_x_transformed()** for touch events

libinput's API only provides the call to map into a single coordinate range.
If the coordinate range has an offset, the compositor is responsible for
applying that offset after the mapping. For example, if the device is mapped
to the right of two outputs, add the output offset to the transformed
coordinate.

.. _absolute_axes_nores:

------------------------------------------------------------------------------
Devices without x/y resolution
------------------------------------------------------------------------------

An absolute device that does not provide a valid resolution is considered
buggy and must be fixed in the kernel. Some touchpad devices do not
provide resolution, those devices are correctly handled within libinput
(touchpads are not absolute devices, as mentioned above).

.. _calibration:

------------------------------------------------------------------------------
Calibration of absolute devices
------------------------------------------------------------------------------

Absolute devices may require calibration to map precisely into the output
range required. This is done by setting a transformation matrix, see
**libinput_device_config_calibration_set_matrix()** which is applied to
each input coordinate.

.. math::
    \begin{pmatrix}
     cos\theta & -sin\theta & xoff \\
     sin\theta &  cos\theta & yoff \\
     0   & 0    & 1
    \end{pmatrix} \begin{pmatrix}
    x \\ y \\ 1
    \end{pmatrix}

:math:`\theta` is the rotation angle. The offsets :math:`xoff` and :math:`yoff` are
specified in device dimensions, i.e. a value of 1 equals one device width or
height. Note that rotation applies to the device's origin, rotation usually
requires an offset to move the coordinates back into the original range.

The most common matrices are:

- 90 degree clockwise:
     .. math::
         \begin{pmatrix}
          0 & -1 & 1 \\
          1 & 0 & 0 \\
          0 & 0 & 1
         \end{pmatrix}
- 180 degree clockwise:
     .. math::
         \begin{pmatrix}
          -1 & 0 & 1 \\
          0 & -1 & 1 \\
          0 & 0 & 1
         \end{pmatrix}
- 270 degree clockwise:
     .. math::
         \begin{pmatrix}
          0 & 1 & 0 \\
          -1 & 0 & 1 \\
          0 & 0 & 1
         \end{pmatrix}
- reflection along y axis:
     .. math::
         \begin{pmatrix}
          -1 & 0 & 1 \\
          1 & 0 & 0 \\
          0 & 0 & 1
         \end{pmatrix}

See Wikipedia's
`Transformation Matrix article <http://en.wikipedia.org/wiki/Transformation_matrix>`_
for more information on the matrix maths. See
**libinput_device_config_calibration_get_default_matrix()** for how these
matrices must be supplied to libinput.

Once applied, any x and y axis value has the calibration applied before it
is made available to the caller. libinput does not provide access to the
raw coordinates before the calibration is applied.

.. _absolute_axes_nonorm:

------------------------------------------------------------------------------
Why x/y coordinates are not normalized
------------------------------------------------------------------------------

x/y are not given in :ref:`normalized coordinates <motion_normalization>`
([0..1]) for one simple reason: the aspect ratio of virtually all current
devices is something other than 1:1. A normalized axes thus is only useful to
determine that the stylus is e.g. at 78% from the left, 34% from the top of
the device. Without knowing the per-axis resolution, these numbers are
meaningless. Worse, calculation based on previous coordinates is simply wrong:
a movement from 0/0 to 50%/50% is not a 45-degree line.

This could be alleviated by providing resolution and information about the
aspect ratio to the caller. Which shifts processing and likely errors into the
caller for little benefit. Providing the x/y axes in mm from the outset
removes these errors.
