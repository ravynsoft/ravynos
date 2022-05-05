# Key Technologies

- Base system, kernel, drivers: a soft fork of FreeBSD "current" (14.0)
- Graphics system: Wayland, EGL, GLESv2, DRM/KMS
- Desktop environment: Custom, written in Cocoa on Wayland

- Apple Open Source (https://opensource.apple.com/source/)
- GNUstep (http://www.gnustep.org) libobjc2
- The Cocotron (https://www.cocotron.org)
- Mach
- launchd, XPC

# General approach

## Software Compatibility

The initial goal is _source compatibility_. ravynOS currently provides Framework bundles that implement a good chunk of Cocoa (not the most recent, however) and some makefile includes that make it easy to build App bundles. It also provides in-kernel support for Mach IPC, tasks, ports, port rights, etc, an implementation of `launchd` and `launchctl` to replace `init`, Grand Central Dispatch, a version of XPC, and other pieces. The general idea here is to implement enough of macOS services and APIs that software will build and run with few modifications.

Standard folders like /Applications, /Library, /System and /Users are all provided. Tools to build an XCode project directly are also planned.

Good progress is being made on this part. It can be called successful when something like Firefox compiles/runs as an App bundle on ravynOS as if it was building on macOS.

A later goal is _binary compatibility with Darwin_. This would allow binary-only applications for x86-64 (and eventually M1) macOS machines to run on ravynOS.


## Look and Feel

Our main goal here is "_user compatibility_'. The system needs to provide a desktop environment and services close to what macOS provides. Our original attempt at this with KDE and helloSystem proved unsatisfying due to limitations of the underlying stack, and a new custom desktop environment is being developed ground-up using the same stack as macOS (Cocoa, Mach, and friends).

A large part of the "feel" will come from supporting AppDirs and AppImages, DMG archives for installing software, a consistent set of key bindings, and familiar metaphors like dragging to trash, multi-finger gestures for various functions, and so on.
