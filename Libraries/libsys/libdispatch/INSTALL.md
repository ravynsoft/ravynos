## Grand Central Dispatch (GCD)

GCD is a concurrent programming framework first shipped with Mac OS X Snow
Leopard.  This package is an open source bundling of libdispatch, the core
user space library implementing GCD.  At the time of writing, support for
the BSD kqueue API, and specifically extensions introduced in Mac OS X Snow
Leopard and FreeBSD 9-CURRENT, are required to use libdispatch.  Linux is
supported, but requires specific packages to be installed (see Linux
section at the end of the file). Other systems are currently unsupported.

### Configuring and installing libdispatch (general comments)

GCD is built using autoconf, automake, and libtool, and has a number of
compile-time configuration options that should be reviewed before starting.
An uncustomized install of the C-API to libdispatch requires:

	sh autogen.sh
	./configure
	make
	make install

libdispatch can be optionally built to include a Swift API. This requires a
Swift toolchain to compile the Swift code in libdispatch and can be done
in two possible scenarios.

If you are building your own Swift toolchain from source, then you should build
libdispatch simply by giving additional arguments to swift/utils/build-script:

    ./swift/utils/build-script --libdispatch -- --install-libdispatch

To build libdispatch using a pre-built Swift toolchain and install libdispatch
into that toolchain (to allow that toolchain to compile Swift code containing
"import Dispatch") requires:

    sh autogen.sh
	./configure --with-swift-toolchain=<PATH_TO_SWIFT_TOOLCHAIN> --prefix=<PATH_TO_SWIFT_TOOLCHAIN>
	make
	make install

Note that once libdispatch is installed into a Swift toolchain, that
toolchain cannot be used to compile libdispatch again (you must 'make uninstall'
libdispatch from the toolchain before using it to rebuild libdispatch).

You can also use the build-toolchain script to create a toolchain
that includes libdispatch on Linux:

1. Add libdispatch and install-libdispatch lines to ./swift/utils/build-presets.ini under `[preset: buildbot_linux]` section, as following:

    ```
    [preset: buildbot_linux]
    mixin-preset=mixin_linux_installation
    build-subdir=buildbot_linux
    lldb
    release
    test
    validation-test
    long-test
    libdispatch
    foundation
    lit-args=-v
    dash-dash

    install-libdispatch
    install-foundation
    reconfigure
    ```

2. Run:

    ```
    ./swift/utils/build-toolchain local.swift
    ```

Note that adding libdispatch in build-presets.ini is for Linux only as Swift on macOS platforms uses the system installed libdispatch, so its not required.

### Building and installing on OS X

The following configure options may be of general interest:

`--with-apple-libpthread-source`

Specify the path to Apple's libpthread package, so that appropriate headers
	can be found and used.

`--with-apple-libplatform-source`

Specify the path to Apple's libplatform package, so that appropriate headers
	can be found and used.

`--with-apple-xnu-source`

Specify the path to Apple's XNU package, so that appropriate headers can be
	found and used.

`--with-blocks-runtime`

On systems where -fblocks is supported, specify an additional library path in which libBlocksRuntime can be found. This is not required on OS X, where the Blocks runtime is included in libSystem, but is required on FreeBSD.

The following options are likely to only be useful when building libdispatch on
OS X as a replacement for /usr/lib/system/libdispatch.dylib:

`--disable-libdispatch-init-constructor`

Do not tag libdispatch's init routine as __constructor, in which case it must be run manually before libdispatch routines can be called. This is the default when building on OS X. For /usr/lib/system/libdispatch.dylib the init routine is called automatically during process start.

`--enable-apple-tsd-optimizations`

Use a non-portable allocation scheme for pthread per-thread data (TSD) keys when building libdispatch for /usr/lib/system on OS X.  This should not be used on other OS's, or on OS X when building a stand-alone library.

#### Typical configuration commands

The following command lines create the configuration required to build
libdispatch for /usr/lib/system on OS X El Capitan:

	clangpath=$(dirname `xcrun --find clang`)
	sudo mkdir -p "$clangpath/../local/lib/clang/enable_objc_gc"
	LIBTOOLIZE=glibtoolize sh autogen.sh
	cflags='-arch x86_64 -arch i386 -g -Os'
	./configure CFLAGS="$cflags" OBJCFLAGS="$cflags" CXXFLAGS="$cflags" \
		--prefix=/usr --libdir=/usr/lib/system --disable-static \
		--enable-apple-tsd-optimizations \
		--with-apple-libpthread-source=/path/to/10.11.0/libpthread-137.1.1 \
		--with-apple-libplatform-source=/path/to/10.11.0/libplatform-73.1.1 \
		--with-apple-xnu-source=/path/to/10.11.0/xnu-3247.1.106 \
	make check

### Building and installing for FreeBSD

Typical configuration line for FreeBSD 8.x and 9.x to build libdispatch with
clang and blocks support:

    ```
    cmake -G Ninja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DBlocksRuntime_INCLUDE_DIR=/usr/local/include -DBlocksRuntime_LIBRARIES=/usr/local/lib/libBlocksRuntime.so <path-to-source>
    ninja
    ninja test
    ```

### Building for android

Note that this assumes that you are building on Linux.  It requires that you
have the android NDK available.  It has been tested against API Level 21.

    ```
    cmake -G Ninja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_SYSTEM_NAME=Android -DCMAKE_SYSTEM_VERSION=21 -DCMAKE_ANDROID_NDK=<path to android NDK> <path-to-source>
    ninja
    ```

### Building and installing for Linux

Note that libdispatch development and testing is done only
on Ubuntu; currently supported versions are 14.04, 15.10 and 16.04.

1. The first thing to do is install required packages:

    `sudo apt-get install cmake ninja-build clang systemtap-sdt-dev libbsd-dev linux-libc-dev`

    Note: compiling libdispatch requires clang 3.8 or better and
the gold linker. If the default clang on your Ubuntu version is
too old, see http://apt.llvm.org/ to install a newer version.
On older Ubuntu releases, you may need to install binutils-gold
to get the gold linker.

2. Build

    ```
    cmake -G Ninja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ <path-to-source>
    ninja
    ninja install
    ```

