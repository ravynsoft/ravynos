.. _middle_button_emulation:

==============================================================================
Middle button emulation
==============================================================================

Middle button emulation provides users with the ability to generate a middle
click even when the device does not have a physical middle button available.

When middle button emulation is enabled, a simultaneous press of the left
and right button generates a middle mouse button event. Releasing the
buttons generates a middle mouse button release, the left and right button
events are discarded otherwise.

The middle button release event may be generated when either button is
released, or when both buttons have been released. The exact behavior is
device-dependent, libinput will implement the behavior that is most
appropriate to the physical device.

The middle button emulation behavior when combined with other device
buttons, including a physical middle button is device-dependent.
For example, :ref:`clickpad_softbuttons` provides a middle button area when
middle button emulation is disabled. That middle button area disappears
when middle button emulation is enabled - a middle click can then only be
triggered by a simultaneous left + right click.

Some devices provide middle mouse button emulation but do not allow
enabling/disabling that emulation. Likewise, some devices may allow middle
button emulation but have it disabled by default. This is the case for most
mouse-like devices where a middle button is detected.

libinput provides **libinput_device_config_middle_emulation_set_enabled()** to
enable or disable middle button emulation. See :ref:`faq_configure_wayland`
and :ref:`faq_configure_xorg` for info on how to enable or disable middle
button emulation in the Wayland compositor or the X stack.
