.. _switches:

==============================================================================
Switches
==============================================================================

libinput supports the lid and tablet-mode switches. Unlike button events
that come in press and release pairs, switches are usually toggled once and
left at the setting for an extended period of time.

Only some switches are handled by libinput, see **libinput_switch** for a
list of supported switches. Switch events are exposed to the caller, but
libinput may handle some switch events internally and enable or disable
specific features based on a switch state.

The order of switch events is guaranteed to be correct, i.e., a switch will
never send consecutive switch on, or switch off, events.

.. _switches_lid:

------------------------------------------------------------------------------
Lid switch handling
------------------------------------------------------------------------------

Where available, libinput listens to devices providing a lid switch.
The evdev event code ``EV_SW`` ``SW_LID`` is provided as
**LIBINPUT_SWITCH_LID**. If devices with a lid switch have a touchpad device,
the device is disabled while the lid is logically closed. This is to avoid
ghost touches that can be caused by interference with touchpads and the
closed lid. The touchpad is automatically re-enabled whenever the lid is
opened.

This handling of lid switches is transparent to the user, no notifications
are sent and the device appears as enabled at all times.

On some devices, the device's lid state does not always reflect the physical
state and the lid state may report as closed even when the lid is physically
open. libinput employs some heuristics to detect user input (specifically
typing) to re-enable the touchpad on those devices. Where input is detected,
libinput updates the lid status of the kernel device so other consumers of
the kernel events also get the accurate state.

.. _switches_tablet_mode:

------------------------------------------------------------------------------
Tablet mode switch handling
------------------------------------------------------------------------------

Where available, libinput listens to devices providing a tablet mode switch.
This switch is usually triggered on devices that can switch between a normal
laptop layout and a tablet-like layout. One example for such a device is the
Lenovo Yoga.

The event sent by the kernel is ``EV_SW`` ``SW_TABLET_MODE`` and is provided as
**LIBINPUT_SWITCH_TABLET_MODE**. When the device switches to tablet mode,
the touchpad and internal keyboard are disabled. If a trackpoint exists,
it is disabled too. The input devices are automatically re-enabled whenever
tablet mode is disengaged.

This handling of tablet mode switches is transparent to the user, no
notifications are sent and the device appears as enabled at all times.
