.. _udev_config:

==============================================================================
Static device configuration via udev
==============================================================================

libinput supports some static configuration through udev properties.
These properties are read when the device is initially added
to libinput's device list, i.e. before the
**LIBINPUT_EVENT_DEVICE_ADDED** event is generated.

The following udev properties are supported:

LIBINPUT_CALIBRATION_MATRIX
    Sets the calibration matrix, see
    **libinput_device_config_calibration_get_default_matrix()**. If unset,
    defaults to the identity matrix.

    The udev property is parsed as 6 floating point numbers separated by a
    single space each (scanf(3) format ``"%f %f %f %f %f %f"``).
    The 6 values represent the first two rows of the calibration matrix as
    described in **libinput_device_config_calibration_set_matrix()**.

    Example values are: ::

          ENV{LIBINPUT_CALIBRATION_MATRIX}="1 0 0 0 1 0" # default
          ENV{LIBINPUT_CALIBRATION_MATRIX}="0 -1 1 1 0 0" # 90 degree clockwise
          ENV{LIBINPUT_CALIBRATION_MATRIX}="-1 0 1 0 -1 1" # 180 degree clockwise
          ENV{LIBINPUT_CALIBRATION_MATRIX}="0 1 0 -1 0 1" # 270 degree clockwise
          ENV{LIBINPUT_CALIBRATION_MATRIX}="-1 0 1 0 1 0" # reflect along y axis


LIBINPUT_DEVICE_GROUP
    A string identifying the **libinput_device_group** for this device. Two
    devices with the same property value are grouped into the same device group,
    the value itself is irrelevant otherwise.

LIBINPUT_IGNORE_DEVICE
    If set to anything other than "0", the device is ignored by libinput.
    See :ref:`ignoring_devices` for more details.

ID_SEAT
    Assigns the physical :ref:`seat <seats>` for this device. See
    **libinput_seat_get_physical_name()**. Defaults to "seat0".

ID_INPUT
    If this property is set, the device is considered an input device. Any
    device with this property missing will be ignored, see :ref:`udev_device_type`.

ID_INPUT_KEYBOARD, ID_INPUT_KEY, ID_INPUT_MOUSE, ID_INPUT_TOUCHPAD, ID_INPUT_TOUCHSCREEN, ID_INPUT_TABLET, ID_INPUT_JOYSTICK, ID_INPUT_ACCELEROMETER
    If any of the above is set, libinput initializes the device as the given
    type, see :ref:`udev_device_type`. Note that for historical reasons more than
    one of these may be set at any time, libinput will select only one of these
    to determine the device type. To ensure libinput selects the correct device
    type, only set one of them.

WL_SEAT
    Assigns the logical :ref:`seat <seats>` for this device. See
    **libinput_seat_get_logical_name()** context. Defaults to "default".

MOUSE_DPI
    HW resolution and sampling frequency of a relative pointer device.
    See :ref:`motion_normalization` for details.

MOUSE_WHEEL_CLICK_ANGLE
    The angle in degrees for each click on a mouse wheel. See
    **libinput_pointer_get_axis_source()** for details.


Below is an example udev rule to assign "seat1" to a device from vendor
``0x012a`` with the model ID of ``0x034b``. ::

   $ cat /etc/udev/rules.d/99-my-device-is-on-seat1.rules
     ACTION!="remove", KERNEL=="event[0-9]*", \
     ENV{ID_VENDOR_ID}=="012a", \
     ENV{ID_MODEL_ID}=="034b", \
     ENV{ID_SEAT}="seat1"



.. _udev_device_type:

------------------------------------------------------------------------------
Device type assignment via udev
------------------------------------------------------------------------------

libinput requires the **ID_INPUT** property to be set on a device,
otherwise the device will be ignored. In addition, one of
**ID_INPUT_KEYBOARD, ID_INPUT_KEY, ID_INPUT_MOUSE, ID_INPUT_TOUCHPAD,
ID_INPUT_TOUCHSCREEN, ID_INPUT_TABLET, ID_INPUT_JOYSTICK,
ID_INPUT_ACCELEROMETER** must be set on the device to determine the
device type. The usual error handling applies within libinput and a device
type label does not guarantee that the device is initialized by libinput.
If a device fails to meet the requirements for a device type (e.g. a keyboard
labelled as touchpad) the device will not be available through libinput.

Only one device type should be set per device at a type, though libinput can
handle some combinations for historical reasons.

Below is an example udev rule  to remove an **ID_INPUT_TOUCHPAD** setting
and change it into an **ID_INPUT_TABLET** setting. This rule would apply
for a device with the vendor/model ID of ``012a``/``034b``. ::

   $ cat /etc/udev/rules.d/99-my-device-is-a-tablet.rules
     ACTION!="remove", KERNEL=="event[0-9]*", \
     ENV{ID_VENDOR_ID}=="012a", \
     ENV{ID_MODEL_ID}=="034b", \
     ENV{ID_INPUT_TOUCHPAD}="", ENV{ID_INPUT_TABLET}="1"


.. _model_specific_configuration:

------------------------------------------------------------------------------
Model-specific configuration
------------------------------------------------------------------------------

As of libinput 1.12, model-specific configuration is stored in the
:ref:`device-quirks` and not in the hwdb anymore. Please see
:ref:`device-quirks` for
details.

.. _model_specific_configuration_x220fw81:

..............................................................................
Lenovo x220 with touchpad firmware v8.1
..............................................................................

The property **LIBINPUT_MODEL_LENOVO_X220_TOUCHPAD_FW81** may be set by a
user in a local hwdb file. This property designates the touchpad on a Lenovo
x220 with a touchpad firmware version 8.1. When this firmware version is
installed, the touchpad is imprecise. The touchpad device does not send
continuous x/y axis position updates, a behavior also observed on its
successor model, the Lenovo x230 which has the same firmware version. If the
above property is set, libinput adjusts its behavior to better suit this
particular model.

The touchpad firmware version cannot be detected automatically by libinput,
local configuration is required to set this property. Refer to the libinput
model quirks hwdb for instructions.

This property must not be used for any other purpose, no specific behavior
is guaranteed.


.. _hwdb:

------------------------------------------------------------------------------
Configuring the hwdb
------------------------------------------------------------------------------

This section outlines how to query the
`udev hwdb <https://www.freedesktop.org/software/systemd/man/hwdb.html>`_
and reload properties so they are available to libinput.

The hwdb contains a set of match rules that assign udev properties that are
available to libinput when the device is connected and/or libinput is
initialized. This section only describes the hwdb in relation to libinput,
it is not a full documentation on how the hwdb works.

libinput's use of the hwdb is limited to properties systemd and custom
rules files (where available) provide. Hardware-specific quirks as used by
libinput are in the :ref:`device-quirks` system.

.. _hwdb_querying:

..............................................................................
Querying the hwdb
..............................................................................

libinput only uses device nodes in the form of ``/dev/input/eventX`` where X
is the number of the specific device. Running ``libinput debug-events`` lists
all devices currently available to libinput and their event node name: ::

    $> sudo libinput debug-events
    -event2   DEVICE_ADDED     Power Button                      seat0 default group1  cap:k
    -event5   DEVICE_ADDED     Video Bus                         seat0 default group2  cap:k
    -event0   DEVICE_ADDED     Lid Switch                        seat0 default group3  cap:S

    ...

Note the event node name for your device and translate it into a syspath in
the form of ``/sys/class/input/eventX``. This path can be supplied to ``udevadm
info`` ::

    $> udevadm info
    P: /devices/LNXSYSTM:00/LNXSYBUS:00/PNP0C0D:00/input/input0/event0
    N: input/event0
    E: DEVNAME=/dev/input/event0
    E: DEVPATH=/devices/LNXSYSTM:00/LNXSYBUS:00/PNP0C0D:00/input/input0/event0
    E: ID_INPUT=1
    E: ID_INPUT_SWITCH=1
    E: MAJOR=13
    E: MINOR=64
    E: SUBSYSTEM=input
    E: TAGS=:power-switch:
    E: USEC_INITIALIZED=7167898

Lines starting with ``E:`` are udev properties available to libinput. For
example, the above device's ``ID_INPUT_SWITCH`` property will cause libinput
to treat this device as switch device.


.. _hwdb_reloading:

..............................................................................
Reloading the hwdb
..............................................................................

The actual hwdb is stored in binary file on-disk and must be updated
manually whenever a ``.hwdb`` file changes. This is required both when a user
manually edits the ``.hwdb`` file but also when the git tree is updated (and
that update causes a hwdb change).

To update the binary file on-disk, run: ::

    sudo systemd-hwdb update

Then, to trigger a reload of all properties on your device, run: ::

    sudo udevadm trigger /sys/class/input/eventX

Then check with ``udevadm info`` whether the properties were updated, see
:ref:`hwdb_querying`. If a new property does not appear on the device, use ``udevadm
test`` to check for error messages by udev and the hwdb (e.g. syntax errors
in the udev rules files). ::

    sudo udevadm test /sys/class/input/eventX

.. warning::  ``udevadm test`` does not run commands specified in ``RUN``
	      directives. This affects the udev properties relying on e.g.
	      the udev keyboard builtin such as the :ref:`touchpad_jitter`
	      workarounds.

.. _hwdb_modifying:

..............................................................................
Modifying the hwdb
..............................................................................

.. warning:: This section has been removed as it no longer applies in libinput 1.12
             and later. libinput users should not need to modify the hwdb, any
             device-specific quirks must go in to the :ref:`device-quirks` system.

For information about older libinput versions, please see the documentation
for your version available in: https://wayland.freedesktop.org/libinput/doc/
