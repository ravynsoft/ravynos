# Key Technologies

- Base system, kernel, drivers: FreeBSD 12
- Graphics system: X11, Qt
- Desktop environment: probably based on KDE Plasma

- Apple Open Source (https://opensource.apple.com/source/)
- GNUstep (http://www.gnustep.org) libobjc2
- Darling (https://darlinghq.org)
- Apportable Foundation (https://github.com/apportable/Foundation)
- The Cocotron (https://www.cocotron.org)
- helloSystem

# General approach

## Software Compatibility

The initial goal is _source compatibility_. Airyx currently provides Framework bundles that implement a good chunk of Cocoa (not the most recent, however) and some makefile includes that make it easy to build App bundles. The general idea here is to implement Cocoa on top of native BSD services like pthreads, X11, TCP sockets, etc. Mach-specific bits like Ports will be emulated.

Standard folders like /Applications, /Library, /System and /Users are all provided. An equivalent of LaunchServices, the `open` command, `launchctl`, and other commands are planned. Tools to build an XCode project are also planned.

Good progress is being made on this part. It can be called successful when something like Firefox compiles/runs as an App bundle on Airyx as if it was building on macOS.

A later goal is _binary compatibility with Darwin_. This would allow binary-only applications for x86-64 macOS machines to run on Airyx. The general idea is to create an emulation of the Mach-specific bits and a shim layer that adapts Mach-O libraries/frameworks to sit on top of ELF libraries and BSD services - basically porting parts of Darling to BSD, but also adapting dyld to handle hybrid Mach-O/ELF links so we can use a single set of Frameworks.


## Look and Feel

Our main goal here is "_user compatibility_'.

By bundling in helloSystem, we've gained a nice lightweight desktop environment, global menu bar, simple file manager and dock components which should feel familiar, although a bit rudimentary. I'm currently integrating Cocoa with the global menu bar over DBUS.

Some improvements are needed to these components, like a sidebar and different views in Filter. Document Preview. Many Preferences panels need to be created. A Notification Center needs to be added. But most of this needs the work of the first goal!

Many projects emulate components like the Dock or Finder. Icon sets, high-quality fonts, and other elements are easy to get. Many window decoration themes exist for KDE, GNOME, and other desktop environments. The biggest challenge will be finding the best of breed and bringing it all together coherently.

Most desktop environments now have the same basic functionality, including features like "night mode" color shifting, system-wide index and search, notification area, widgets for battery life, sound volume, etc.

A large part of the "feel" will come from supporting AppDirs and AppImages, DMG-style archives for installing software, a consistent set of key bindings, and familiar metaphors like dragging to trash, multi-finger gestures for various functions, and so on.
