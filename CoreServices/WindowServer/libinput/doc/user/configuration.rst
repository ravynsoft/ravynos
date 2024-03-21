.. _config_options:

==============================================================================
Configuration options
==============================================================================

Below is a list of configurable options exposed to the users.

.. hint:: Not all configuration options are available on all devices. Use
	  :ref:`libinput list-devices <libinput-list-devices>` to show the
	  configuration options for local devices.

libinput's configuration interface is available to the caller only, not
directly to the user. Thus is is the responsibility of the caller to expose
the various options and how these options are exposed. For example, the
xf86-input-libinput driver exposes the options through X Input device
properties and xorg.conf.d options. See the `libinput(4)
<https://www.mankier.com/4/libinput>`_ man page for more details.


------------------------------------------------------------------------------
Tap-to-click
------------------------------------------------------------------------------

See :ref:`tapping` for details on how this feature works. Configuration
options exposed by libinput are:

- how many tapping fingers are supported by this device
- a toggle to enable/disable tapping
- a toggle to enable/disable tap-and-drag, see :ref:`tapndrag`.
- a toggle to enable/disable tap-and-drag drag lock see :ref:`tapndrag`
- The default order is 1, 2, 3 finger tap mapping to left, right, middle
  click, respectively. This order can be changed to left, middle, right click,
  respectively.

Tapping is usually available on touchpads and the touchpad part of external
graphics tablets. Tapping is usually **not** available on touch screens,
for those devices it is expected to be implemented by the toolkit.

------------------------------------------------------------------------------
Send Events Mode
------------------------------------------------------------------------------

The Send Events Mode is libinput's terminology for disabling a device. It is
more precise in that the device only stops sending events but may not get
fully disabled. For example, disabling the touchpad on a
:ref:`Lenovo T440 and similar <t440_support>` leaves the top software
buttons enabled for the trackpoint. Available options are
**enabled** (send events normally), **disabled** ( do not send events),
**disabled on external mouse** (disable the device while an external mouse
is plugged in).


.. _config_pointer_acceleration:

------------------------------------------------------------------------------
Pointer acceleration
------------------------------------------------------------------------------

Pointer acceleration is a function to convert input deltas to output deltas,
usually based on the movement speed of the device, see
:ref:`pointer-acceleration` for details.

Pointer acceleration is normalized into a [-1, 1] range, where -1 is
"slowest" and 1 is "fastest". Most devices use a default speed of 0.

The pointer acceleration profile defines **how** the input deltas are
converted, see :ref:`ptraccel-profiles`. Most devices have their default
profile (usually called "adaptive") and a "flat" profile. The flat profile
does not apply any acceleration.

------------------------------------------------------------------------------
Scrolling
------------------------------------------------------------------------------

"Natural scrolling" is the terminology for moving the content in the
direction of scrolling, i.e. moving the wheel or fingers down moves the page
down. Traditional scrolling moves the content in the opposite direction.
Natural scrolling can be turned on or off, it is usually off by default.

The scroll method defines how to trigger scroll events. On touchpads
libinput provides two-finger scrolling and edge scrolling. Two-finger
scrolling converts a movement with two fingers to a series of scroll events.
Edge scrolling converts a movement with one finger along the right or bottom
edge of the touchpad into a series of scroll events.

On other libinput provides button-scrolling - movement of the device while
the designated scroll button is down is converted to scroll events. The
button used for scrolling is configurable.

The scroll method can be chosen or disabled altogether but most devices only
support a subset of available scroll methods. libinput's default is
two-finger scrolling for multi-touch touchpads, edge scrolling for
single-touch touchpads. On tracksticks, button scrolling is enabled by
default.

See :ref:`scrolling` for more details on how the scroll methods work.

------------------------------------------------------------------------------
Left-handed Mode
------------------------------------------------------------------------------

Left-handed mode switches the device's functionality to be more
accommodating for left-handed users. On mice this usually means swapping the
left and right mouse button, on tablets this allows the tablet to be used
upside-down to present the pad buttons for the non-dominant right hand. Not
all devices have left-handed mode.

Left-handed mode can be enabled or disabled and is disabled by default.

------------------------------------------------------------------------------
Middle Button Emulation
------------------------------------------------------------------------------

Middle button emulation converts a simultaneous left and right button click
into a middle button. The emulation can be enabled or disabled. Middle
button emulation is usually enabled when the device does not provide a
middle button.

------------------------------------------------------------------------------
Click method
------------------------------------------------------------------------------

The click method defines how button events are triggered on a :ref:`clickpad
<clickpad_softbuttons>`. When set to button areas, the bottom area of the
touchpad is divided into a left, middle and right button area. When set to
clickfinger, the number of fingers on the touchpad decide the button type.
Clicking with 1, 2, 3 fingers triggers a left, right, or middle click,
respectively. The default click method is software button areas. Click
methods are usually only available on :ref:`clickpads
<clickpad_softbuttons>`.

------------------------------------------------------------------------------
Disable while typing
------------------------------------------------------------------------------

DWT is the most generic form of palm detection on touchpad. While the user
is typing on an internal keyboard the touchpad is disabled, the touchpad
is enabled again after a timeout.  See :ref:`disable-while-typing` for more
info.

Disable-while-typing can be enabled or disabled, it is enabled by default on
most touchpads.

------------------------------------------------------------------------------
Disable while trackpointing
------------------------------------------------------------------------------

DWTP is a form of palm detecion for devices that have a trackpoint (like
Thinkpads). While the user is using the trackpoint, the touchpad is disabled,
being enabled again after a timeout. See :ref:`disable-while-trackpointing` for
more info.

Disable-while-trackpointing can be enabled or disabled, it is enabled by
default.

------------------------------------------------------------------------------
Calibration
------------------------------------------------------------------------------

Calibration is available for some direct-input devices (touch screens,
graphics tablets, etc.). The purpose of calibration is to ensure the input
lines up with the output and the configuration data is a transformation
matrix. It is thus not expected that the user sets this option. The desktop
environment should provide an interface for this.

------------------------------------------------------------------------------
Rotation
------------------------------------------------------------------------------

The device rotation applies a corrective angle to relative input events,
allowing the device to be used e.g. sideways or upside-down. For example, a
trackball may be used in a 90° rotated position for accessibility reasons -
such a rotated position allows triggering the buttons with the thumb or
the non-dominant hand.

Note that where a device rotation is higher than 160 but less than 200 degrees,
the direction of wheels is also inverted. For all other angles, the wheel
direction is left as-is.
