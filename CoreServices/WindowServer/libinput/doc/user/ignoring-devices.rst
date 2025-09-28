.. _ignoring_devices:

==============================================================================
Ignoring specific devices
==============================================================================

If a device has the **LIBINPUT_IGNORE_DEVICE** udev property set to any
value but "0", that device is not initialized by libinput.
For a context created with **libinput_udev_create_context()**, the device is
silently ignored and never shows up. If the device is added with
**libinput_path_add_device()** to a context created with
**libinput_path_create_context()**, adding the device will fail and return NULL
(see that function's documentation for more
information).

If the property value is exactly "0", then the property is considered unset
and libinput initializes the device normally.

This property should be used for devices that are correctly detected as
input devices (see :ref:`udev_device_type`) but that should not be used by
libinput. It is recommended that devices that should not be handled as input
devices at all unset the **ID_INPUT** and related properties instead. The
**LIBINPUT_IGNORE_DEVICE** property signals that only libinput should
ignore this property but other parts of the stack (if any) should continue
treating this device normally.

Below is an example udev rule  to assign **LIBINPUT_IGNORE_DEVICE** to the
device with the vendor/model ID of ``012a``/``034b``. ::

  $ cat /etc/udev/rules.d/99-ignore-my-device.rules
  ACTION!="remove", KERNEL=="event[0-9]*", \
     ENV{ID_VENDOR_ID}=="012a", \
     ENV{ID_MODEL_ID}=="034b", \
     ENV{LIBINPUT_IGNORE_DEVICE}="1"

See :ref:`udev_config` for more details on libinput's udev properties.
