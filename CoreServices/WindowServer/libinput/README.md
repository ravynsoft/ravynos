libinput
========

libinput is a library that provides a full input stack for display servers
and other applications that need to handle input devices provided by the
kernel.

libinput provides device detection, event handling and abstraction to
minimize the amount of custom input code the user of libinput needs to
provide the common set of functionality that users expect. Input event
processing includes scaling touch coordinates, generating
relative pointer events from touchpads, pointer acceleration, etc.

User documentation
------------------

Documentation explaining features available in libinput is available
[here](https://wayland.freedesktop.org/libinput/doc/latest/features.html).

This includes the [FAQ](https://wayland.freedesktop.org/libinput/doc/latest/faqs.html)
and the instructions on
[reporting bugs](https://wayland.freedesktop.org/libinput/doc/latest/reporting-bugs.html).


Source code
-----------

The source code of libinput can be found at:
https://gitlab.freedesktop.org/libinput/libinput

For a list of current and past releases visit:
https://www.freedesktop.org/wiki/Software/libinput/

Build instructions:
https://wayland.freedesktop.org/libinput/doc/latest/building.html

Reporting Bugs
--------------

Bugs can be filed on freedesktop.org GitLab:
https://gitlab.freedesktop.org/libinput/libinput/issues/

Where possible, please provide the `libinput record` output
of the input device and/or the event sequence in question.

See https://wayland.freedesktop.org/libinput/doc/latest/reporting-bugs.html
for more info.

Documentation
-------------

- Developer API documentation: https://wayland.freedesktop.org/libinput/doc/latest/development.html
- High-level documentation about libinput's features:
  https://wayland.freedesktop.org/libinput/doc/latest/features.html
- Build instructions:
  https://wayland.freedesktop.org/libinput/doc/latest/building.html
- Documentation for previous versions of libinput: https://wayland.freedesktop.org/libinput/doc/

Examples of how to use libinput are the debugging tools in the libinput
repository. Developers are encouraged to look at those tools for a
real-world (yet simple) example on how to use libinput.

- A commandline debugging tool: https://gitlab.freedesktop.org/libinput/libinput/tree/main/tools/libinput-debug-events.c
- A GTK application that draws cursor/touch/tablet positions: https://gitlab.freedesktop.org/libinput/libinput/tree/main/tools/libinput-debug-gui.c

License
-------

libinput is licensed under the MIT license.

> Permission is hereby granted, free of charge, to any person obtaining a
> copy of this software and associated documentation files (the "Software"),
> to deal in the Software without restriction, including without limitation
> the rights to use, copy, modify, merge, publish, distribute, sublicense,
> and/or sell copies of the Software, and to permit persons to whom the
> Software is furnished to do so, subject to the following conditions: [...]

See the [COPYING](https://gitlab.freedesktop.org/libinput/libinput/tree/main/COPYING)
file for the full license information.

About
-----

Documentation generated from git commit [__GIT_VERSION__](https://gitlab.freedesktop.org/libinput/libinput/commit/__GIT_VERSION__)
