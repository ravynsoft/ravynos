# Raven's Guide to the Source Galaxy

Don't panic. This document will give you an overview of the important folders and where things are. The organization of the source tree is still evolving, so check back frequently!


### Kernel
`Kernel` contains the xnu kernel itself, kernel extensions (kexts), and related low-level stuff.

* xnu - the xnu kernel tree
  * `config` - configurations for exported apis for supported architecture and platform
  * `SETUP` - Basic set of tools used for configuring the kernel, versioning and kextsymbol 
management.
  * `EXTERNAL_HEADERS` - Headers sourced from other projects to avoid dependency cycles when
 building. These headers should be regularly synced when source is updated.
  * `libkern` - C++ IOKit library code for handling of drivers and kexts.
  * `libsa` -  kernel bootstrap code for startup
  * `libsyscall` - syscall library interface for userspace programs
  * `libkdd` - source for user library for parsing kernel data like kernel chunked data.
  * `makedefs` - top level rules and defines for kernel build.
  * `osfmk` - Mach kernel based subsystems
  * `pexpert` - Platform specific code like interrupt handling, atomics etc.
  * `security` - Mandatory Access Check policy interfaces and related implementation.
  * `bsd` - BSD subsystems code
  * `tools` - A set of utilities for testing, debugging and profiling kernel.
* libfirehose_kernel - kernel side of the log system. Source is actually in `libdispatch`
* Extensions - kexts for IOKit


### Libraries
`Libraries` holds the source of most libraries, including the core `libsystem_*` that make up libSystem.

* CommonCrypto
* libdispatch - Grand Central Dispatch
* Libc (libsystem_c)
* libplatform (libsystem_platform)
* libpthread (libsystem_pthread)
* libmalloc (libsystem_malloc)
* Libnotify (libsystem_notify)
* Libinfo (libsystem_info)
* Libsystem
* dyld - runtime dynamic linker and loader


### Developer

`Developer` contains all of the tools and artifacts needed to create software for the platform. The ravynOS (MacOSX) SDK is here, as well as the Default.xctoolchain. `Developer` is unique in that some of it builds twice: once as `Bootstrap` for the host toolchain, then again using the host toolchain for the platform target.

* Toolchains/Default.xctoolchain - project file to assemble the toolchain
* Platforms/ravynOS.platform - platform definitions and SDKs
  * Developer/SDKs/ravynOS.sdk - project file and some sources for the SDK
* cctools - lipo, ld64 linker, otool, etc
* dtrace_ctf - ctf utilities
* llvm-project - Apple fork of LLVM and Apple TAPI project
* unifdef
* xcbuild - Xcode project builder from Meta, `xcrun`, `xcodebuild`, `xcode-select`
* cmake - configure Version.cmake with the product and kernel versions
* gmake - GNU make, the default on macOS
* mig - Mach interface generator
* xar - archiver tool used in the build


### BSD (__Planned__)

The `BSD` folder holds system utilities and services for the BSD subsystem: UNIX command line tools, daemons like `syslogd`, and generally anything going to `/bin`, `/usr/bin`, etc that doesn't have another home. This folder isn't part of the build yet.

* syslog
* launchd - the newer one we used in FreeBSD (to be ported)
* old_launchd - archive of the really old one for cross reference

## These folders have not yet been converted to CMake or Darwin

### SystemLibrary

This folder contains the template for SystemVersion.plist which is set from `/Developer/cmake/Version.cmake`. It also has most things meant for /System/Library or /Library on the built system that are not built elsewhere, like `Desktop Pictures` and `LaunchDaemons`.


### Frameworks

The ravynOS implementation of Cocoa, Quartz, and Core APIs in various frameworks live in `Frameworks`. API compliance ranges from 10.7-ish to 11.0-ish and there are many stubs for the intrepid explorer to complete. Some of this source originated from Cocotron. Add new frameworks here if they belong in /S/L/F or /L/F.

* Foundation
* AppKit
* ApplicationServices
* Cocoa
* LaunchServices
* CoreFoundation
* CoreText
* CoreVideo
* CoreServices
* CoreGraphics
* CoreData
* CFNetwork
* QuartzCore
* Onyx2D - graphics toolkit underpinning CoreGraphics, equivalent to Quartz2D
* OpenGL
* PreferencePanes

* CF - this is the Swift CF-Lite to be merged in with our CF

### CoreServices

`CoreServices` mostly contains core system services above the BSD layer, such as Dock, WindowServer, the desktop shell, and Filer app.