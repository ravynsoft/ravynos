Building the TIFF Software Distribution
=======================================

.. image:: images/cramps.gif
    :width: 159
    :alt: cramps

This chapter contains step-by-step instructions on how to configure
and build the TIFF software distribution. The software is most
easily built on a UNIX system, but with a little bit of work it can
easily be built and used on other non-UNIX platforms.


Building on all systems with CMake
----------------------------------

CMake may be used to
generate build files for most common build systems and IDEs, and
supports all UNIX-like systems as well as Windows. See
the `CMake website <http://www.cmake.org/>`_ for further
details. To build the software on you need to first run
:command:`cmake` to configure the build and generate the system-specific
build files. This reads the top-level :file:`CMakeLists.txt` file,
which probes the target system for necessary tools and functions,
checks any options you specified to configure the build, and then
outputs build files configured for your system.  If using ``Unix
Makefiles``, once configuration is done, you simply
run :command:`make` (or :command:`gmake`) to build the software and
then :command:`make install` to do the installation.  For other build
systems, you do the equivalent steps with the tool for that system.
For example, on any UNIX system:

.. code-block:: shell

    $ cd ./tiff-4.0.5
    $ cmake
        ...lots of messages...
    $ make
        ...lots of messages...
    $ make test
        ...lots of messages...
    # make install

Building is dependent on a :program:`make` utility and a C
(and optionally a C++) compiler, so you will need these tools.

In general, the software is designed such that the following
targets will always be available:

::

    make [all]      build stuff
    make test       run the test suite
    make install    build and install stuff
    make clean      remove object files, executables and cruft

Build Trees
^^^^^^^^^^^

There are two schemes for configuring and building the software. If
you intend to build the software for only one target system, you
can configure the software so that it is built in the same
directories as the source code.

.. code-block:: shell

    $ gzip -dc tiff-4.0.5.tar.gz | tar -xf -
    $ cd ./tiff-4.0.5
    $ cmake
    $ make
    $ make test
    $ make install

Otherwise, you can configure a build tree that is parallel to
the source tree hierarchy (or in some completely different place)
but which contains only configured files and files created during
the build procedure.


.. code-block:: shell

    $ gzip -dc tiff-4.0.5.tar.gz | tar -xf -
    $ mkdir tiff-4.0.5-build
    $ cd ./tiff-4.0.5-build
    $ cmake ../tiff-4.0.5
    $ make
    $ make test
    $ make install

This second scheme is useful for:

* building multiple targets from a single source tree
* building from a read-only source tree
* sharing the source files via a network, but building on
  multiple systems
* keeping the source tree clean
  (unlike :program:`autoconf`, :program:`cmake` does not provide
  a ``distclean`` target, so out of source builds are
  recommended)

Generators
^^^^^^^^^^

The default generator for UNIX is ``Unix Makefiles``, and on Windows is
``NMake Makefiles`` or ``MSBuild`` depending upon the setup.
Run :command:`cmake --help` to list all the
generators available for your platform.  For example, to use the Ninja
`build system <https://martine.github.io/ninja/>`_ on UNIX or
Windows:

.. code-block:: shell

    cmake -G Ninja
    cmake --build .
    ctest -V
    cmake --build . --target install

Note that :command:`cmake --build .` is a build-system-independent way
of building a target; you can always use the build system directly.

Alternatively, using the MSBuild system on Windows (64-bit Release
build with VS2013):

.. code-block:: shell

    cmake -G "Visual Studio 12 2013 Win64"
    cmake --build . --config Release
    ctest -V -C Release
    cmake --build . --config Release --target install

With the above configuration, it's also possible to open the generated
solution file with the Visual Studio IDE as well as building on the
command-line.

Configuration Options
^^^^^^^^^^^^^^^^^^^^^

The configuration process is critical to the proper compilation,
installation, and operation of the
software. The :file:`CMakeLists.txt` script runs a series of tests to
decide whether or not the target system supports required
functionality and, if it does not, whether it can emulate or
workaround the missing functions.  After running :command:`cmake`, check
the :file:`CMakeCache.txt` file; this contains all the results of the
checks performed and the options set by the user.  If :program:`cmake`
failed to run, check :file:`CMakeFiles/CMakeOutput.log`
and :file:`CMakeFiles/CMakeError.log`; these should record the error
which caused the failure.

A second function of the configure script is to set the default
configuration parameters for the software. Of particular note are the
directories where the software is to be installed. By default the
software is installed in the :file:`/usr/local` hierarchy. To change
this behaviour the appropriate parameters can be specified on the
command line. Run :command:`cmake --help` to get a full list of possible
options, and :command:`cmake -LH` to list all the configurable options for
this software package, or :command:`cmake -LAH` to show all advanced
options in addition. Standard installation related options are shown
below.

  .. list-table:: Installation options
    :widths: 10 15
    :header-rows: 1

    * - Option
      - Description

    * - ``CMAKE_INSTALL_PREFIX``
      - Installation root directory.  The options below may be used to override
        individual installation locations.
    * - ``CMAKE_INSTALL_BINDIR``
      - user executables [:file:`PREFIX/bin`]
    * - ``CMAKE_INSTALL_SBINDIR``
      - system admin executables [:file:`PREFIX/sbin`]
    * - ``CMAKE_INSTALL_LIBEXECDIR``
      - program executables [:file:`PREFIX/libexec`]
    * - ``CMAKE_INSTALL_SYSCONFDIR``
      - read-only single-machine data [:file:`PREFIX/etc`]
    * - ``CMAKE_INSTALL_SHAREDSTATEDIR``
      - modifiable architecture-independent data [:file:`PREFIX/com`]
    * - ``CMAKE_INSTALL_LOCALSTATEDIR``
      - modifiable single-machine data [:file:`PREFIX/var`]
    * - ``CMAKE_INSTALL_LIBDIR``
      - object code libraries [:file:`PREFIX/lib`]
    * - ``CMAKE_INSTALL_INCLUDEDIR``
      - C header files [:file:`PREFIX/include`]
    * - ``CMAKE_INSTALL_OLDINCLUDEDIR``
      - C header files for non-gcc [:file:`/usr/include`]
    * - ``CMAKE_INSTALL_DATAROOTDIR``
      - read-only architecture-independent data root [:file:`PREFIX/share`]
    * - ``CMAKE_INSTALL_DATADIR``
      - read-only architecture-independent data [:file:`DATAROOTDIR`]
    * - ``CMAKE_INSTALL_LOCALEDIR``
      - locale-dependent data [:file:`DATAROOTDIR/locale`]
    * - ``CMAKE_INSTALL_MANDIR``
      - man documentation [:file:`DATAROOTDIR/man`]
    * - ``CMAKE_INSTALL_DOCDIR``
      - documentation root [:file:`DATAROOTDIR/doc/tiff`]
    
Also see the
CMake `documentation <http://www.cmake.org/cmake/help/latest/>`_
for `additional variables <http://www.cmake.org/cmake/help/latest/manual/cmake-variables.7.html>`_
which may be set.

Configuring Optional Packages/Support
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The TIFF software comes with several packages that are installed
only as needed, or only if specifically configured at the time the
configure script is run. Packages can be configured via the
:program:`cmake` commandline parameters.

Static/Shared Objects Support
.............................

``BUILD_SHARED_LIBS[=ON|OFF]``:

    Build shared libraries (default is ``ON``)

    This option controls whether or not to configure the software
    to build a shared and static binaries for the TIFF library. Use of
    shared libraries can significantly reduce the disk space needed for
    users of the TIFF software. If shared libraries are not used then
    the code is statically linked into each application that uses it.

``ld-version-script[=ON|OFF]``

    Enable linker version script (default is ``ON``)

    Add shared library symbol versioning on ELF-based systems (e.g.
    Linux and FreeBSD) which use the GNU linker. This is needed if
    several major versions of libtiff might be loaded at once into the
    same program.

JPEG Support
............

``jpeg[=ON|OFF]``

    Enable IJG JPEG library usage (required for JPEG compression, enabled by default)

``JPEG_INCLUDE_DIR=DIR``:

    Location of IJG JPEG library headers

``JPEG_LIBRARY=DIR``

    Location of IJG JPEG library binary

The ``JPEG`` package enables support for the handling of
TIFF images with JPEG-encoded data. Support for JPEG-encoded data
requires the Independent JPEG Group (IJG) ``libjpeg``
distribution; this software is available at `<http://www.ijg.org/>`_.
The CMake script automatically tries to search for a working IJG JPEG
installation. If it fails to find library, JPEG support will be
automatically disabled. If you want specify the exact paths to
library binary and headers, use above options for that.

ZIP Support
...........

The ``ZIP`` support enables support for the handling of TIFF
images with deflate-encoded data (enabled by default if
available). Support for deflate-encoded data requires the freely
available ``zlib`` distribution written by Jean-loup Gailly and
Mark Adler; this software is available at `<http://www.zlib.org/>`_.


Building on a UNIX System with Autoconf
---------------------------------------

.. program:: configure

To build the software on a UNIX system you need to first run the
:program:`configure` shell script that is located in the top level of the
source directory. This script probes the target system for
necessary tools and functions and constructs a build environment in
which the software may be compiled. Once configuration is done, you
simply run :command:`make` (or :command:`gmake`) to build the software
and then :command:`make install` to do the installation; for example:

.. code-block:: shell

    % cd ./tiff-4.0.5
    % ./configure
        ...lots of messages...
    % make
        ...lots of messages...
    % make check
        ...lots of messages...
    # make install

Supplied Makefiles are dependent on a :program:`make` utility and a C
(and optionally a C++ compiler), so you will need these tools.

In general, the software is designed such that the following
should be "make-able" in each directory:

  .. list-table:: Make targets
    :widths: 10 15
    :header-rows: 1

    * - Target
      - Description
    * - :command:`make [all]`
      - build everything
    * - :command:`make check`
      - run the test suite
    * - :command:`make install`
      - build and install everything
    * - :command:`make clean`
      - remove object files, executables and cruft
    * - :command:`make distclean`
      - remove everything that can be recreated

Note that after running :command:`make distclean` the
:program:`configure` script must be run again to create the :file:`Makefile`
and other make-related files.

Build Trees
^^^^^^^^^^^

There are two schemes for configuring and building the software. If
you intend to build the software for only one target system, you
can configure the software so that it is built in the same
directories as the source code.

.. code-block:: shell

    % gzip -dc tiff-4.0.5.tar.gz | tar -xf -
    % cd ./tiff-4.0.5
    % ./configure
    % make
    % make check
    % make install

Otherwise, you can configure a build tree that is parallel to
the source tree hierarchy (or in some completely different place)
but which contains only configured files and files created during
the build procedure.

.. code-block:: shell

    % gzip -dc tiff-4.0.5.tar.gz | tar -xf -
    % mkdir tiff-4.0.5-build
    % cd ./tiff-4.0.5-build
    % ../tiff-4.0.5/configure
    % make
    % make check
    % make install

This second scheme is useful for:

* building multiple targets from a single source tree
* building from a read-only source tree
* sharing the source files via a network, but building on
  multiple systems

Configuration Options
^^^^^^^^^^^^^^^^^^^^^

The configuration process is critical to the proper compilation,
installation, and operation of the software. The configure script
runs a series of tests to decide whether or not the target system
supports required functionality and, if it does not, whether it can
emulate or workaround the missing functions. This procedure is
fairly complicated and, due to the nonstandard nature of most UNIX
systems, prone to error. The first time that you configure the
software for use you should check the output from the configure
script and look for anything that does not make sense for your
system.

A second function of the configure script is to set the default
configuration parameters for the software. Of particular note are
the directories where the software is to be installed. By default
the software is installed in the :file:`/usr/local` hierarchy. To
change this behaviour the appropriate parameters can be specified
on the command line to configure. Run :command:`./configure --help` to
get a full list of possible options. Standard installation related
options are shown below.

Installation directories:

.. option:: --prefix=PREFIX

  install architecture-independent files in *PREFIX* [:file:`/usr/local`]

.. option:: --exec-prefix=EPREFIX

  install architecture-dependent files in *EPREFIX* [:file:`PREFIX`]

By default, :command:`make install` will install all the files in
:file:`/usr/local/bin`, :file:`/usr/local/lib` etc.  You can specify
an installation prefix other than :file:`/usr/local` using :option:`--prefix`,
for instance ``--prefix=$HOME``.  For better control, use the options below.

Fine tuning of the installation directories:

.. option:: --bindir=DIR

  user executables [:file:`EPREFIX/bin`]

.. option:: --sbindir=DIR

  system admin executables [:file:`EPREFIX/sbin`]

.. option:: --libexecdir=DIR

  program executables [:file:`EPREFIX/libexec`]

.. option:: --sysconfdir=DIR

  read-only single-machine data [:file:`PREFIX/etc`]

.. option:: --sharedstatedir=DIR

  modifiable architecture-independent data [:file:`PREFIX/com`]

.. option:: --localstatedir=DIR

  modifiable single-machine data [:file:`PREFIX/var`]

.. option:: --libdir=DIR

  object code libraries [:file:`EPREFIX/lib`]

.. option:: --includedir=DIR

  C header files [:file:`PREFIX/include`]

.. option:: --oldincludedir=DIR

  C header files for non-gcc [:file:`/usr/include`]

.. option:: --datarootdir=DIR

  read-only architecture-independent data root [:file:`PREFIX/share`]

.. option:: --datadir=DIR

  read-only architecture-independent data [:file:`DATAROOTDIR`]

.. option:: --localedir=DIR

  locale-dependent data [:file:`DATAROOTDIR/locale`]

.. option:: --mandir=DIR

  man documentation [:file:`DATAROOTDIR/man`]

.. option:: --docdir=DIR

  documentation root [:file:`DATAROOTDIR/doc/tiff`]

.. option:: --htmldir=DIR

  html documentation [:file:`DOCDIR`]


Program names:

.. option:: --program-prefix=PREFIX

  prepend *PREFIX* to installed program names

.. option:: --program-suffix=SUFFIX

append *SUFFIX* to installed program names

.. option:: --program-transform-name=PROGRAM

  run :command:`sed` *PROGRAM* on installed program names

Configuring Optional Packages/Support
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The TIFF software comes with several packages that are installed
only as needed, or only if specifically configured at the time the
configure script is run. Packages can be configured via the
:program:`configure` script commandline parameters.

Static/Shared Objects Support
.............................

.. option:: --enable-shared[=PKGS]

    Build shared libraries [enabled]

.. option:: --enable-static[=PKGS]

    Build static libraries [enabled]

    These options control whether or not to configure the software
    to build a shared and static binaries for the TIFF library. Use of
    shared libraries can significantly reduce the disk space needed for
    users of the TIFF software. If shared libraries are not used then
    the code is statically linked into each application that uses it.
    By default both types of binaries are configured.

.. option:: --enable-rpath

    Enable runtime linker paths (``-R`` libtool option)

    Add library directories (see other options below) to the TIFF
    library run-time linker path.

.. option:: --enable-ld-version-script

    Enable linker version script [yes]

    Add shared library symbol versioning on ELF-based systems (e.g.
    Linux and FreeBSD) which use the GNU linker. This is needed if
    several major versions of libtiff might be loaded at once into the
    same program.

JPEG Support
............

.. option:: --disable-jpeg

    Disable IJG JPEG library usage (required for JPEG compression, enabled by default)

.. option:: --with-jpeg-include-dir=DIR

    Location of IJG JPEG library headers

.. option:: --with-jpeg-lib-dir=DIR

    Location of IJG JPEG library binary)

The ``JPEG`` package enables support for the handling of
TIFF images with JPEG-encoded data. Support for JPEG-encoded data
requires the Independent JPEG Group (IJG) ``libjpeg``
distribution; this software is available at
`<http://www.ijg.org/>`_.  The :program:`configure`
script automatically tries to search for a working IJG JPEG
installation. If it fails to find library, JPEG support will be
automatically disabled. If you want specify the exact paths to
library binary and headers, use above switches for that.

ZIP Support
...........

The ``ZIP`` support enables support for the handling of
TIFF images with deflate-encoded data. Support for deflate-encoded
data requires the freely available ``zlib`` distribution
written by Jean-loup Gailly and Mark Adler; this software is
available at `<http://www.zlib.org/>`_.  Support will be
enabled automatically if ``zlib`` is found.


Building the Software on Other Systems
--------------------------------------

This section contains information that might be useful if you are
working on a non-UNIX system that is not directly supported. All
library-related files described below are located in the
:file:`libtiff` directory.

The library requires two files that are generated
*on-the-fly*. The file :file:`tif_fax3sm.c` has the state
tables for the Group 3 and Group 4 decoders. This file is generated
by the :program:`mkg3states` program on a UNIX system; for
example:

.. code-block:: shell

    cd libtiff
    cc -o mkg3states mkg3states.c
    rm -f tif_fax3sm.c
    ./mkg3states -c const tif_fax3sm.c

The ``-c`` option can be used to control whether or not the
resulting tables are generated with a ``const`` declaration.
The ``-s`` option can be used to specify a C storage class for
the table declarations. The ``-b`` option can be used to force
data values to be explicitly bracketed with ``{}`` (apparently
needed for some MS-Windows compilers); otherwise the structures are
emitted in as compact a format as possible. Consult the source code
for this program if you have questions.

The second file required to build the library, :file:`version.h`,
contains the version information returned by the
:c:func:`TIFFGetVersion` routine. This file is built on most systems
using the :file:`mkversion` program and the contents of the
:file:`VERSION` and :file:`tiff.alpha` files; for example,

.. code-block:: shell

    cd libtiff
    cc -o mkversion mkversion.c
    rm -f version.h
    ./mkversion -v ../VERSION -a ../dist/tiff.alpha version.h

Otherwise, when building the library on a non-UNIX system be
sure to consult the files :file:`tiffcomp.h` and :file:`tiffconf.h`.
The former contains system compatibility definitions while the
latter is provided so that the software configuration can be
controlled on systems that do not support the make facility for
building the software.

Systems without a 32-bit compiler may not be able to handle some
of the codecs in the library; especially the Group 3 and 4 decoder.
If you encounter problems try disabling support for a particular
codec; consult the :doc:`internals`.

Programs in the tools directory are written to assume an ANSI C
compilation environment. There may be a few POSIX'isms as well. The
code in the :file:`port` directory is provided to emulate routines
that may be missing on some systems. On UNIX systems the
:program:`configure` script automatically figures out which routines
are not present on a system and enables the use of the equivalent
emulation routines from the :file:`port` directory. It may be
necessary to manually do this work on a non-UNIX system.


Testing the software
--------------------

You can try
:doc:`/tools/tiffinfo` to display the file metadata.  See the
:doc:`images` section on obtaining the test images.
Otherwise, you can do a cursory check of the library
with the :doc:`/tools/tiffcp` program. For example,

.. code-block:: shell

    tiffcp -lzw cramps.tif x.tif



LibTIFF source files
--------------------

The following files make up the core library:

  .. list-table:: Core library source files
    :widths: 5 15
    :header-rows: 1

    * - File
      - Description

    * - :file:`libtiff/tiff.h`
      - TIFF spec definitions
    * - :file:`libtiff/tiffconf.h`
      - non-UNIX configuration definitions
    * - :file:`libtiff/tiffio.h`
      - public TIFF library definitions
    * - :file:`libtiff/tiffiop.h`
      - private TIFF library definitions
    * - :file:`libtiff/t4.h`
      - CCITT Group 3/4 code tables+definitions
    * - :file:`libtiff/tif_dir.h`
      - private defs for TIFF directory handling
    * - :file:`libtiff/tif_fax3.h`
      - CCITT Group 3/4-related definitions
    * - :file:`libtiff/tif_predict.h`
      - private defs for Predictor tag support
    * - :file:`libtiff/tiffvers.h`
      - version string
    * - :file:`libtiff/uvcode.h`
      - LogL/LogLuv codec-specific definitions
    * - :file:`libtiff/tif_aux.c`
      - auxiliary directory-related functions
    * - :file:`libtiff/tif_close.c`
      - close an open TIFF file
    * - :file:`libtiff/tif_codec.c`
      - configuration table of builtin codecs
    * - :file:`libtiff/tif_color.c`
      - colorspace transforms
    * - :file:`libtiff/tif_compress.c`
      - compression scheme support
    * - :file:`libtiff/tif_dir.c`
      - directory tag interface code
    * - :file:`libtiff/tif_dirinfo.c`
      - directory known tag support code
    * - :file:`libtiff/tif_dirread.c`
      - directory reading code
    * - :file:`libtiff/tif_dirwrite.c`
      - directory writing code
    * - :file:`libtiff/tif_dumpmode.c`
      - "no" compression codec
    * - :file:`libtiff/tif_error.c`
      - library error handler
    * - :file:`libtiff/tif_fax3.c`
      - CCITT Group 3 and 4 codec
    * - :file:`libtiff/tif_fax3sm.c`
      - G3/G4 state tables (generated by mkg3states)
    * - :file:`libtiff/tif_flush.c`
      - i/o and directory state flushing
    * - :file:`libtiff/tif_getimage.c`
      - :doc:`/functions/TIFFRGBAImage` support
    * - :file:`libtiff/tif_jbig.c`
      - JBIG codec
    * - :file:`libtiff/tif_jpeg.c`
      - JPEG codec (interface to the IJG distribution)
    * - :file:`libtiff/tif_jpeg_12.c`
      - 12-bit JPEG codec (interface to the IJG distribution)
    * - :file:`libtiff/tif_lerc.c`
      - LERC codec
    * - :file:`libtiff/tif_luv.c`
      - SGI LogL/LogLuv codec
    * - :file:`libtiff/tif_lzma.c`
      - LZMA codec
    * - :file:`libtiff/tif_lzw.c`
      - LZW codec
    * - :file:`libtiff/tif_next.c`
      - NeXT 2-bit scheme codec (decoding only)
    * - :file:`libtiff/tif_ojpeg.c`
      - Old JPEG codec (obsolete, decoding only)
    * - :file:`libtiff/tif_open.c`
      - open and simply query code
    * - :file:`libtiff/tif_packbits.c`
      - Packbits codec
    * - :file:`libtiff/tif_pixarlog.c`
      - Pixar codec
    * - :file:`libtiff/tif_predict.c`
      - Predictor tag support
    * - :file:`libtiff/tif_print.c`
      - directory printing support
    * - :file:`libtiff/tif_read.c`
      - image data reading support
    * - :file:`libtiff/tif_strip.c`
      - some strip-related code
    * - :file:`libtiff/tif_swab.c`
      - byte and bit swapping support
    * - :file:`libtiff/tif_thunder.c`
      - Thunderscan codec (decoding only)
    * - :file:`libtiff/tif_tile.c`
      - some tile-related code
    * - :file:`libtiff/tif_unix.c`
      - UNIX-related OS support
    * - :file:`libtiff/tif_version.c`
      - library version support
    * - :file:`libtiff/tif_warning.c`
      - library warning handler
    * - :file:`libtiff/tif_win32.c`
      - Win32 (Windows)-related OS support
    * - :file:`libtiff/tif_write.c`
      - image data writing support
    * - :file:`libtiff/tif_zip.c`
      - Deflate codec
    * - :file:`libtiff/tif_zstd.c`
      - ZSTD codec

    * - :file:`libtiff/mkg3states.c`
      - program to generate G3/G4 decoder state tables
    * - :file:`libtiff/mkspans.c`
      - program to generate black-white span tables
