.. _device-quirks:

==============================================================================
Device quirks
==============================================================================

libinput requires extra information from devices that is not always readily
available. For example, some touchpads are known to have jumping cursors
under specific conditions. libinput ships a set of files containing the
so-called model quirks to provide that information. Model quirks are usually
installed under ``/usr/share/libinput/<filename>.quirks`` and are standard
``.ini`` files. A file may contain multiple section headers (``[some
identifier]``) followed by one or more ``MatchFoo=Bar`` directives, followed by
at least one of ``ModelFoo=1`` or ``AttrFoo=bar`` directive. See the
``quirks/README.md`` file in the libinput source repository for more details on
their contents.

.. warning:: Model quirks are internal API and may change at any time. No
             backwards-compatibility is guaranteed.

For example, a quirks file may have this content to label all keyboards on
the serial bus (PS/2) as internal keyboards: ::

     [Serial Keyboards]
     MatchUdevType=keyboard
     MatchBus=serial
     AttrKeyboardIntegration=internal


The model quirks are part of the source distribution and should never be
modified locally. Updates to libinput may overwrite modifications or even
stop parsing any property. For temporary local workarounds, see
:ref:`device-quirks-local`.

Device quirks are parsed on libinput initialization. A parsing error in the
device quirks disables **all** device quirks and may negatively impact
device behavior on the host. If the quirks cannot be loaded, an error
message is posted to the log and users should use the information in
:ref:`device-quirks-debugging` to verify their quirks files.

.. _device-quirks-local:

------------------------------------------------------------------------------
Installing temporary local device quirks
------------------------------------------------------------------------------

The model quirks are part of the source distribution and should never be
modified. For temporary local workarounds, libinput reads the
``/etc/libinput/local-overrides.quirks`` file. Users may add a sections to
this file to add a device quirk for a local device but beware that **any
modification must be upstreamed** or it may cease to work at any time.

.. warning:: Model quirks are internal API and may change at any time. No
             backwards-compatibility is guaranteed. Local overrides should only
             be used until the distribution updates the libinput packages.

The ``local-overrides.quirks`` file usually needs to be created by the user.
Once the required section has been added, use the information from section
:ref:`device-quirks-debugging` to validate and test the quirks.

.. _device-quirks-debugging:

------------------------------------------------------------------------------
Debugging device quirks
------------------------------------------------------------------------------

libinput provides the ``libinput quirks`` tool to debug the quirks database.
This tool takes an action as first argument, the most common invocation is
``libinput quirks list`` to list model quirks that apply to one or more local
devices. ::

     $ libinput quirks list /dev/input/event19
     $ libinput quirks list /dev/input/event0
     AttrLidSwitchReliability=unreliable

The device `event19` does not have any quirks assigned.

When called with the ``--verbose`` argument, ``libinput quirks list`` prints
information about all files and its attempts to match the device: ::

     $ libinput quirks list --verbose /dev/input/event0
     quirks debug: /usr/share/share/libinput is data root
     quirks debug: /usr/share/share/libinput/10-generic-keyboard.quirks
     quirks debug: /usr/share/share/libinput/10-generic-lid.quirks
     [...]
     quirks debug: /usr/share/etc/libinput/local-overrides.quirks
     quirks debug: /dev/input/event0: fetching quirks
     quirks debug: [Serial Keyboards] (10-generic-keyboard.quirks) wants MatchBus but we don't have that
     quirks debug: [Lid Switch Ct9] (10-generic-lid.quirks) matches for MatchName
     quirks debug: [Lid Switch Ct10] (10-generic-lid.quirks) matches for MatchName
     quirks debug: [Lid Switch Ct10] (10-generic-lid.quirks) matches for MatchDMIModalias
     quirks debug: [Lid Switch Ct10] (10-generic-lid.quirks) is full match
     quirks debug: property added: AttrLidSwitchReliability from [Lid Switch Ct10] (10-generic-lid.quirks)
     quirks debug: [Aiptek No Tilt Tablet] (30-vendor-aiptek.quirks) wants MatchBus but we don't have that
     [...]
     quirks debug: [HUION PenTablet] (30-vendor-huion.quirks) wants MatchBus but we don't have that
     quirks debug: [Logitech Marble Mouse Trackball] (30-vendor-logitech.quirks) wants MatchBus but we don't have that
     quirks debug: [Logitech K400] (30-vendor-logitech.quirks) wants MatchBus but we don't have that
     quirks debug: [Logitech K400r] (30-vendor-logitech.quirks) wants MatchBus but we don't have that
     quirks debug: [Logitech K830] (30-vendor-logitech.quirks) wants MatchBus but we don't have that
     quirks debug: [Logitech K400Plus] (30-vendor-logitech.quirks) wants MatchBus but we don't have that
     quirks debug: [Logitech Wireless Touchpad] (30-vendor-logitech.quirks) wants MatchBus but we don't have that
     quirks debug: [Microsoft Surface 3 Lid Switch] (30-vendor-microsoft.quirks) matches for MatchName
     [...]
     AttrLidSwitchReliability


Note that this is an example only, the output may change over time. The tool
uses the same parser as libinput and any parsing errors will show up in the
output.

.. _device-quirks-list:

------------------------------------------------------------------------------
List of currently available device quirks
------------------------------------------------------------------------------

This list is a guide for developers to ease the process of submitting
patches upstream. This section shows device quirks currently available in
|git_version|.

.. warning:: Quirks are internal API and may change at any time for any reason.
             No guarantee is given that any quirk below works on your version of
             libinput.

In the documentation below, the letters N, M, O, P refer to arbitrary integer
values.

Quirks starting with **Model*** triggers implementation-defined behaviour
for this device not needed for any other device. Only the more
general-purpose **Model*** flags are listed here.

ModelALPSTouchpad, ModelAppleTouchpad, ModelWacomTouchpad, ModelChromebook
    Reserved for touchpads made by the respective vendors
ModelTouchpadVisibleMarker
    Indicates the touchpad has a drawn-on visible marker between the software
    buttons.
ModelTabletModeNoSuspend
    Indicates that the device does not need to be
    suspended in :ref:`switches_tablet_mode`.
ModelTabletModeSwitchUnreliable
    Indicates that this tablet mode switch's state cannot be relied upon.
ModelTrackball
    Reserved for trackballs
ModelBouncingKeys
    Indicates that the device may send fake bouncing key events and
    timestamps can not be relied upon.
ModelSynapticsSerialTouchpad
    Reserved for touchpads made by Synaptics on the serial bus
ModelPressurePad
    Unlike in traditional touchpads, whose pressure value equals contact size,
    on pressure pads pressure is a real physical axis.
    Indicates that the device is a pressure pad.
AttrSizeHint=NxM, AttrResolutionHint=N
    Hints at the width x height of the device in mm, or the resolution
    of the x/y axis in units/mm. These may only be used where they apply to
    a large proportion of matching devices. They should not be used for any
    specific device, override ``EVDEV_ABS_*`` instead, see
    :ref:`absolute_coordinate_ranges_fix`.
AttrTouchSizeRange=N:M, AttrPalmSizeThreshold=O
    Specifies the touch size required to trigger a press (N) and to trigger
    a release (M). O > N > M. See :ref:`touchpad_touch_size_hwdb` for more
    details.
    An AttrPalmSizeThreshold of zero unsets any threshold that has been
    inherited from another quirk.
AttrPressureRange=N:M, AttrPalmPressureThreshold=O, AttrThumbPressureThreshold=P
    Specifies the touch pressure required to trigger a press (N) and to
    trigger a release (M), when a palm touch is triggered (O) and when a
    thumb touch is triggered (P). O > P > N > M. See
    :ref:`touchpad_pressure_hwdb` for more details.
    An AttrPalmPressureThreshold of zero unsets any threshold that has been
    inherited from another quirk.
AttrLidSwitchReliability=reliable|unreliable|write_open
    Indicates the reliability of the lid switch. This is a string enum.
    Very few devices need this, if in doubt do not set. See :ref:`switches_lid`
    for details. libinput 1.21.0 changed the default from unreliable to
    reliable, which may be removed from local overrides.
AttrKeyboardIntegration=internal|external
    Indicates the integration of the keyboard. This is a string enum.
    Generally only needed for USB keyboards.
AttrTPKComboLayout=below
    Indicates the position of the touchpad on an external touchpad+keyboard
    combination device. This is a string enum. Don't specify it unless the
    touchpad is below.
AttrEventCode=+EV_ABS;-BTN_STYLUS;+EV_KEY:0x123;
    Enables or disables the evdev event type/code tuples on the device. The prefix
    for each entry is either '+' (enable) or '-' (disable). Entries may be
    a named event type, or a named event code, or a named event type with a
    hexadecimal event code, separated by a single colon.
AttrInputProp=+INPUT_PROP_BUTTONPAD;-INPUT_PROP_POINTER;
    Enables or disables the evdev input property on the device. The prefix
    for each entry is either '+' (enable) or '-' (disable). Entries may be
    a named input property or the hexadecimal value of that property.
AttrPointingStickIntegration=internal|external
    Indicates the integration of the pointing stick. This is a string enum.
    Only needed for external pointing sticks. These are rare.
AttrTabletSmoothing=1|0
    Enables (1) or disables (0) input smoothing for tablet devices. Smoothing is enabled
    by default, except on AES devices.
