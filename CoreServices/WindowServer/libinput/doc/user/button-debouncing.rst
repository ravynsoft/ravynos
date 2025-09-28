
.. _button_debouncing:

==============================================================================
Button debouncing
==============================================================================

Physical buttons experience wear-and-tear with usage. On some devices this
can result in an effect called "contact bouncing" or "chatter". This effect
can cause the button to send multiple events within a short time frame, even
though the user only pressed or clicked the button once. This effect can be
counteracted by "debouncing" the buttons, usually by ignoring erroneous
events.

libinput provides two methods of debouncing buttons, referred to as the
"bounce" and "spurious" methods:

- In the "bounce" method, libinput monitors hardware bouncing on button
  state changes, i.e. when a user clicks or releases a button. For example,
  if a user presses a button but the hardware generates a
  press-release-press sequence in quick succession, libinput ignores the
  release and second press event. This method is always enabled.
- in the "spurious" method, libinput detects spurious releases of a button
  while the button is physically held down by the user. These releases are
  immediately followed by a press event. libinput monitors for these events
  and ignores the release and press event. This method is disabled by
  default and enables once libinput detects the first faulty event sequence.

The "bounce" method guarantees that all press events are delivered
immediately and most release events are delivered immediately. The
"spurious" method requires that release events are delayed, libinput thus
does not enable this method unless a faulty event sequence is detected. A
message is printed to the log when spurious deboucing was detected.

libinput's debouncing is supposed to correct hardware damage or
substandard hardware. Debouncing also exists as an accessibility feature
but the requirements are different. In the accessibility feature, multiple
physical key presses, usually caused by involuntary muscle movement, must be
filtered to only one key press. This feature must be implemented higher in
the stack, libinput is limited to hardware debouncing.

Below is an illustration of the button debouncing modes to show the relation
of the physical button state and the application state. Where applicable, an
extra line is added to show the timeouts used by libinput that
affect the button state handling. The waveform's high and low states
correspond to the buttons 'pressed' and 'released' states, respectively.

.. figure:: button-debouncing-wave-diagram.svg
    :align: center

    Diagram illustrating button debouncing


Some devices send events in bursts, erroneously triggering the button
debouncing detection. Please :ref:`file a bug <reporting_bugs>` if that
occurs for your device.
