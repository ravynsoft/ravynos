.. _test-suite:

==============================================================================
libinput test suite
==============================================================================

libinput's primary test suite can be invoked with

::

	$ sudo ./builddir/libinput-test-suite

When developing libinput, the ``libinput-test-suite`` should always be
run to check for behavior changes and/or regressions. For quick iteration,
the number of tests to run can be filtered, see :ref:`test-filtering`.
This allows for developers to verify a subset of tests (e.g.
touchpad tap-to-click) while hacking on that specific feature and only run
the full suite when development is done finished.

.. note:: The test suite relies on udev and the kernel, specifically uinput.
	It creates virtual input devices and replays the events. This may
	interfere with your running session. The test suite is not suitable
	for running inside containers.

In addition, libinput ships with a set of (primarily janitorial) tests that
must pass for any merge request. These tests are invoked by calling
``meson test -C builddir`` (or ``ninja test``). The ``libinput-test-suite`` is
part of that test set by default.

The upstream CI runs all these tests but not the ``libinput-test-suite``.
This CI is run for every merge request.

.. _test-job-control:

------------------------------------------------------------------------------
Job control in the test suite
------------------------------------------------------------------------------

The test suite runner has a make-like job control enabled by the ``-j`` or
``--jobs`` flag and will fork off as many parallel processes as given by this
flag. The default if unspecified is 8. When debugging a specific test case
failure it is recommended to employ test filtures (see :ref:`test-filtering`)
and disable parallel tests. The test suite automatically disables parallel
make when run in gdb.

.. _test-config:

------------------------------------------------------------------------------
X.Org config to avoid interference
------------------------------------------------------------------------------

uinput devices created by the test suite are usually recognised by X as
input devices. All events sent through these devices will generate X events
and interfere with your desktop.

Copy the file ``$srcdir/test/50-litest.conf`` into your ``/etc/X11/xorg.conf.d``
and restart X. This will ignore any litest devices and thus not interfere
with your desktop.

.. _test-root:

------------------------------------------------------------------------------
Permissions required to run tests
------------------------------------------------------------------------------

Most tests require the creation of uinput devices and access to the
resulting ``/dev/input/eventX`` nodes. Some tests require temporary udev rules.
**This usually requires the tests to be run as root**. If not run as
root, the test suite runner will exit with status 77, an exit status
interpreted as "skipped".

.. _test-filtering:

------------------------------------------------------------------------------
Selective running of tests
------------------------------------------------------------------------------

litest's tests are grouped into test groups, test names and devices. A test
group is e.g.  "touchpad:tap" and incorporates all tapping-related tests for
touchpads. Each test function is (usually) run with one or more specific
devices. The ``--list`` commandline argument shows the list of suites and
tests. This is useful when trying to figure out if a specific test is
run for a device.


::

     $ ./builddir/libinput-test-suite --list
     ...
     pointer:left-handed:
	pointer_left_handed_during_click_multiple_buttons:
		trackpoint
		ms-surface-cover
		mouse-wheelclickcount
		mouse-wheelclickangle
		low-dpi-mouse
		mouse-roccat
		mouse-wheel-tilt
		mouse
		logitech-trackball
		cyborg-rat
		magicmouse
	pointer_left_handed_during_click:
		trackpoint
		ms-surface-cover
		mouse-wheelclickcount
		mouse-wheelclickangle
		low-dpi-mouse
		mouse-roccat
		mouse-wheel-tilt
		mouse
		logitech-trackball
		cyborg-rat
		litest-magicmouse-device
	pointer_left_handed:
		trackpoint
		ms-surface-cover
		mouse-wheelclickcount
		mouse-wheelclickangle
		low-dpi-mouse
		mouse-roccat
		mouse-wheel-tilt
		mouse
     ...


In the above example, the "pointer:left-handed" suite contains multiple
tests, e.g. "pointer_left_handed_during_click" (this is also the function
name of the test, making it easy to grep for). This particular test is run
for various devices including the trackpoint device and the magic mouse
device.

The "no device" entry signals that litest does not instantiate a uinput
device for a specific test (though the test itself may
instantiate one).

The ``--filter-test`` argument enables selective running of tests through
basic shell-style function name matching. For example:


::

     $ ./builddir/libinput-test-suite --filter-test="*1fg_tap*"


The ``--filter-device`` argument enables selective running of tests through
basic shell-style device name matching. The device names matched are the
litest-specific shortnames, see the output of ``--list``. For example:


::

     $ ./builddir/libinput-test-suite --filter-device="synaptics*"


The ``--filter-group`` argument enables selective running of test groups
through basic shell-style test group matching. The test groups matched are
litest-specific test groups, see the output of ``--list``. For example:


::

     $ ./builddir/libinput-test-suite --filter-group="touchpad:*hover*"


The ``--filter-device`` and ``--filter-group`` arguments can be combined with
``--list`` to show which groups and devices will be affected.

.. _test-verbosity:

------------------------------------------------------------------------------
Controlling test output
------------------------------------------------------------------------------

Each test supports the ``--verbose`` commandline option to enable debugging
output, see **libinput_log_set_priority()** for details. The ``LITEST_VERBOSE``
environment variable, if set, also enables verbose mode.


::

     $ ./builddir/libinput-test-suite --verbose
     $ LITEST_VERBOSE=1 meson test -C builddir

.. _test-installed:

------------------------------------------------------------------------------
Installing the test suite
------------------------------------------------------------------------------

If libinput is configured to install the tests, the test suite is available
as the ``libinput test-suite`` command. When run as installed binary, the
behavior of the test suite changes:

- the ``libinput.so`` used is the one in the library lookup paths
- no system-wide quirks are installed by the test suite, only those specific
  to the test devices
- test device-specific quirks are installed in the system-wide quirks
  directory, usually ``/usr/share/libinput/``.

It is not advisable to run ``libinput test-suite`` on a production machine.
Data loss may occur. The primary use-case for the installed test suite is
verification of distribution composes.

.. note:: The ``prefix`` is still used by the test suite. For verification
	of a system package, the test suite must be configured with the same prefix.

To configure libinput to install the tests, use the ``-Dinstall-tests=true``
meson option::

  $ meson setup builddir -Dtests=true -Dinstall-tests=true <other options>

.. _test-meson-suites:

------------------------------------------------------------------------------
Meson test suites
------------------------------------------------------------------------------

This section is primarily of interest to distributors that want to run test
or developers working on libinput's CI.

Tests invoked by ``meson test`` are grouped into test suites, the test suite
names identify when the respective test can be run:

- ``valgrind``: tests that can be run under valgrind (in addition to a
  normal run)
- ``root``: tests that must be run as root
- ``hardware``: tests that require a VM or physical machine
- ``all``: all tests, only needed because of
  `meson bug 5340 <https://github.com/mesonbuild/meson/issues/5340>`_

The suite names can be provided as filters to ``meson test
--suite=<suitename>`` or ``meson test --no-suite=<suitename>``.
For example, if running a container-based CI, you may specify the test
suites as:

::

   $ meson test --no-suite=machine  # only run container-friendly tests
   $ meson test --suite=valgrind --setup=valgrind  # run all valgrind-compatible tests
   $ meson test --no-suite=root  # run all tests not requiring root

These suites are subject to change at any time.
