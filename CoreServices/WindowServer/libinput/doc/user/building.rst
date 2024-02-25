.. _building_libinput:

==============================================================================
libinput build instructions
==============================================================================


.. contents::
    :local:
    :backlinks: entry

Instructions on how to build libinput and its tools and how to build against
libinput.

The build instruction on this page detail how to overwrite your
system-provided libinput with one from the git repository, see
see :ref:`reverting_install` to revert to the previous state.

.. _distribution_repos:

------------------------------------------------------------------------------
Distribution repositories for libinput from git
------------------------------------------------------------------------------

Some distributions provide package repositories for users that want to test
the latest libinput without building it manually.

.. note:: The list below is provided for convenience. The libinput community
   cannot provide any guarantees that the packages in those repositories are
   correct, up-to-date and/or unmodified from the git branch. Due dilligence
   is recommended.

The following repositories provide an up-to-date package for libinput:

- **Arch:** https://aur.archlinux.org/packages/libinput-git/
- **Fedora:** https://copr.fedorainfracloud.org/coprs/whot/libinput-git/

Please follow the respective repositories for instructions on how to enable
the repository and install libinput.


.. _building:

------------------------------------------------------------------------------
Building libinput
------------------------------------------------------------------------------

libinput uses `meson <https://www.mesonbuild.com>`_ and
`ninja <https://www.ninja-build.org>`_. A build is usually the three-step
process below.  A successful build requires the
:ref:`building_dependencies` to be installed before running meson.


::

     $> git clone https://gitlab.freedesktop.org/libinput/libinput
     $> cd libinput
     $> meson setup --prefix=/usr builddir/
     $> ninja -C builddir/
     $> sudo ninja -C builddir/ install


When running libinput versions 1.11.x or earlier, you must run

::

     $> sudo systemd-hwdb update


Additional options may also be specified. For example:

::

     $> meson setup --prefix=/usr -Ddocumentation=false builddir/


We recommend that users disable the documentation, it's not usually required
for testing and reduces the number of dependencies needed.

The ``prefix`` or other options can be changed later with the
``meson configure`` command. For example:

::

     $> meson configure builddir/ -Dprefix=/some/other/prefix -Ddocumentation=true
     $> ninja -C builddir
     $> sudo ninja -C builddir/ install


Running ``meson configure builddir/`` with no other arguments lists all
configurable options meson provides.

To rebuild from scratch, simply remove the build directory and run meson
again:

::

     $> rm -r builddir/
     $> meson setup --prefix=....


.. _verifying_install:

..............................................................................
Verifying the install
..............................................................................

To verify the install worked correctly, check that libinput.so.x.x.x is in
the library path and that all symlinks point to the new library.

::

     $> ldconfig -p | grep libinput | awk '{print $NF}' | xargs ls -l
     lrwxrwxrwx 1 root root      14 lug 22 13:06 /usr/lib/x86_64-linux-gnu/libinput.so -> libinput.so.10
     lrwxrwxrwx 1 root root      19 lug 22 13:06 /usr/lib/x86_64-linux-gnu/libinput.so.10 -> libinput.so.10.13.0
     -rwxr-xr-x 1 root root 1064144 lug 22 13:06 /usr/lib/x86_64-linux-gnu/libinput.so.10.13.0

.. _reverting_install:

..............................................................................
Reverting to the system-provided libinput package
..............................................................................

The recommended way to revert to the system install is to use the package
manager to reinstall the libinput package. In some cases, this may leave
files in the system (e.g. ``/usr/lib/libinput.la``) but these files are
usually harmless. To definitely remove all files, run the following command
from the libinput source directory:


::

     $> sudo ninja -C builddir/ uninstall
     # WARNING: Do not restart the computer/X/the Wayland compositor after
     # uninstall, reinstall the system package immediately!


The following commands reinstall the current system package for libinput,
overwriting manually installed files.

- **Debian/Ubuntu** based distributions: ``sudo apt-get install --reinstall libinput``
- **Fedora 22** and later: ``sudo dnf reinstall libinput``
- **RHEL/CentOS/Fedora 21** and earlier: ``sudo yum reinstall libinput``
- **openSUSE**: ``sudo zypper install --force libinput10``
- **Arch**: ``sudo pacman -S libinput``

.. _building_selinux:

..............................................................................
SELinux adjustments
..............................................................................

.. note:: This section only applies to meson version < 0.42.0

On systems with SELinux, overwriting the distribution-provided package with
a manually built libinput may cause SELinux denials. This usually manifests
when gdm does not start because it is denied access to libinput. The journal
shows a log message in the form of:


::

     May 25 15:28:42 localhost.localdomain audit[23268]: AVC avc:  denied  { execute } for  pid=23268 comm="gnome-shell" path="/usr/lib64/libinput.so.10.12.2" dev="dm-0" ino=1709093 scontext=system_u:system_r:xdm_t:s0-s0:c0.c1023 tcontext=unconfined_u:object_r:user_home_t:s0 tclass=file permissive=0
     May 25 15:28:42 localhost.localdomain org.gnome.Shell.desktop[23270]: /usr/bin/gnome-shell: error while loading shared libraries: libinput.so.10: failed to map segment from shared object


The summary of this error message is that gdm's gnome-shell runs in the
``system_u:system_r:xdm_t`` context but libinput is installed with the
context ``unconfined_u:object_r:user_home_t``.

To avoid this issue, restore the SELinux context for any system files.


::

     $> sudo restorecon /usr/lib*/libinput.so.*


This issue is tracked in https://github.com/mesonbuild/meson/issues/1967.

.. _building_dependencies:

------------------------------------------------------------------------------
Build dependencies
------------------------------------------------------------------------------

libinput has a few build-time dependencies that must be installed prior to
running meson.

.. hint:: The build dependencies for some distributions can be found in the
	`GitLab Continuous Integration file <https://gitlab.freedesktop.org/libinput/libinput/blob/main/.gitlab-ci.yml>`_.
	Search for **FEDORA_PACKAGES** in the **variables:** definition
	and check the list for an entry for your distribution.

In most cases, it is sufficient to install the dependencies that your
distribution uses to build the libinput package.  These can be installed
with one of the following commands:

- **Debian/Ubuntu** based distributions: ``sudo apt-get build-dep libinput``
- **Fedora 22** and later: ``sudo dnf builddep libinput``
- **RHEL/CentOS/Fedora 21** and earlier: ``sudo yum-builddep libinput``
- **openSUSE**: ::

     $> sudo zypper modifyrepo --enable ``zypper repos | grep source | awk '{print $5}'``
     $> sudo zypper source-install -d libinput10
     $> sudo zypper install autoconf automake libtool
     $> sudo zypper modifyrepo --disable ``zypper repos | grep source | awk '{print $5}'``


- **Arch**: ::

     $> sudo pacman -S asp
     $> cd $(mktemp -d)
     $> asp export libinput
     $> cd libinput
     $> makepkg --syncdeps --nobuild --noextract



If dependencies are missing, meson shows a message ``No package 'foo'
found``.  See
`this blog post here <https://who-t.blogspot.com/2018/07/meson-fails-with-native-dependency-not-found.html>`_
for instructions on how to fix it.

..............................................................................
Build dependencies per distribution
..............................................................................


.. include:: dependencies.rst


.. _building_conditional:

------------------------------------------------------------------------------
Conditional builds
------------------------------------------------------------------------------

libinput supports several meson options to disable parts of the build. See
the ``meson_options.txt`` file in the source tree for a full list of
available options. The default build enables most options and thus requires
more build dependencies. On systems where build dependencies are an issue,
options may be disabled with this meson command: ::

    meson setup --prefix=/usr -Dsomefeature=false builddir

Where ``-Dsomefeature=false`` may be one of:

- ``-Ddocumentation=false``
    Disables the documentation build (this website). Building the
    documentation is only needed on the maintainer machine.
- ``-Dtests=false``
    Disables the test suite. The test suite is only needed on developer
    systems.
- ``-Ddebug-gui=false``
    Disables the ``libinput debug-gui`` helper tool (see :ref:`tools`),
    dropping GTK and other build dependencies. The debug-gui is only
    required for troubleshooting.
- ``-Dlibwacom=false``
    libwacom is required by libinput's tablet code to gather additional
    information about tablets that is not available from the kernel device.
    It is not recommended to disable libwacom unless libinput is used in an
    environment where tablet support is not required. libinput provides tablet
    support even without libwacom, but some features may be missing or working
    differently.

.. _building_against:

------------------------------------------------------------------------------
Building against libinput
------------------------------------------------------------------------------

libinput provides a
`pkg-config <https://www.freedesktop.org/wiki/Software/pkg-config/>`_ file.
Software that uses autotools should use the ``PKG_CHECK_MODULES`` autoconf
macro: ::

    PKG_CHECK_MODULES(LIBINPUT, "libinput")

Software that uses meson should use the ``dependency()`` function: ::

    pkgconfig = import('pkgconfig')
    dep_libinput = dependency('libinput')

Software that uses CMake should use: ::

    find_package(Libinput)
    target_link_libraries(myprogram PRIVATE Libinput::Libinput)

Otherwise, the most rudimentary way to compile and link a program against
libinput is:


::

         gcc -o myprogram myprogram.c ``pkg-config --cflags --libs libinput``


For further information on using pkgconfig see the pkg-config documentation.
