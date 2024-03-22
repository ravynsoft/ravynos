# Wayland

Wayland is a project to define a protocol for a compositor to talk to
its clients as well as a library implementation of the protocol.  The
compositor can be a standalone display server running on Linux kernel
modesetting and evdev input devices, an X application, or a wayland
client itself.  The clients can be traditional applications, X servers
(rootless or fullscreen) or other display servers.

The wayland protocol is essentially only about input handling and
buffer management.  The compositor receives input events and forwards
them to the relevant client.  The clients creates buffers and renders
into them and notifies the compositor when it needs to redraw.  The
protocol also handles drag and drop, selections, window management and
other interactions that must go through the compositor.  However, the
protocol does not handle rendering, which is one of the features that
makes wayland so simple.  All clients are expected to handle rendering
themselves, typically through cairo or OpenGL.

Building the wayland libraries is fairly simple, aside from libffi,
they don't have many dependencies:

    $ git clone https://gitlab.freedesktop.org/wayland/wayland
    $ cd wayland
    $ meson build/ --prefix=PREFIX
    $ ninja -C build/ install

where PREFIX is where you want to install the libraries.

See https://wayland.freedesktop.org for documentation.
