AM_CPPFLAGS = \
	-I$(top_srcdir) \
	$(CODE_COVERAGE_CPPFLAGS) \
	$(DBUS_STATIC_BUILD_CPPFLAGS) \
	$(VALGRIND_CFLAGS) \
	-DDBUS_COMPILATION \
	$(NULL)

AM_CFLAGS = \
	$(CODE_COVERAGE_CFLAGS) \
	$(SANITIZE_CFLAGS) \
	$(NULL)

# if assertions are enabled, improve backtraces
AM_LDFLAGS = @R_DYNAMIC_LDFLAG@

TEST_EXTENSIONS = .sh

LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) $(top_srcdir)/build-aux/tap-driver.sh
SH_LOG_DRIVER = $(LOG_DRIVER)

LOG_COMPILER = \
	@abs_top_builddir@/tools/dbus-run-session$(EXEEXT) \
        --config-file=@abs_top_builddir@/test/data/valid-config-files/tmp-session.conf \
	--dbus-daemon=@abs_top_builddir@/bus/dbus-daemon$(EXEEXT) \
	-- \
	$(NULL)
SH_LOG_COMPILER = $(SHELL)

## note that TESTS has special meaning (stuff to use in make check)
## so if adding tests not to be run in make check, don't add them to 
## TESTS
TESTS =

if DBUS_ENABLE_EMBEDDED_TESTS
if DBUS_UNIX
AM_TESTS_ENVIRONMENT = \
        export DBUS_SESSION_BUS_ADDRESS=do-not-use-real-session:; \
        export DBUS_TOP_BUILDDIR=@abs_top_builddir@; \
        export DBUS_TOP_SRCDIR=@abs_top_srcdir@; \
        export PATH="@abs_top_builddir@/bus:$$PATH"; \
        export DBUS_TEST_DATA=@abs_top_builddir@/test/data; \
        export DBUS_TEST_DAEMON=@abs_top_builddir@/bus/dbus-daemon$(EXEEXT); \
        export DBUS_TEST_DBUS_LAUNCH=@abs_top_builddir@/tools/dbus-launch$(EXEEXT); \
        export XDG_RUNTIME_DIR=@abs_top_builddir@/test/XDG_RUNTIME_DIR; \
        unset DBUS_SESSION_BUS_ADDRESS; \
        unset DBUS_SESSION_BUS_PID; \
        $(NULL)

TESTS += \
	run-test.sh \
	run-test-systemserver.sh \
	test-ids \
	test-pending-call-disconnected \
	test-shutdown \
	$(NULL)

if ENABLE_TRADITIONAL_ACTIVATION
TESTS += \
	test-pending-call-dispatch \
	test-pending-call-timeout \
	test-privserver-client \
	test-threads-init \
	$(NULL)
endif
endif
endif

EXTRA_DIST=run-test.sh run-test-systemserver.sh

if DBUS_ENABLE_EMBEDDED_TESTS

## we use noinst_PROGRAMS not check_PROGRAMS for TESTS so that we
## build even when not doing "make check"
noinst_PROGRAMS=test-pending-call-dispatch test-pending-call-timeout test-pending-call-disconnected test-threads-init test-ids test-shutdown test-privserver-client test-autolaunch

test_pending_call_dispatch_LDADD = \
	$(CODE_COVERAGE_LIBS) \
	$(top_builddir)/dbus/libdbus-1.la \
	$(NULL)
test_pending_call_timeout_LDADD = \
	$(CODE_COVERAGE_LIBS) \
	$(top_builddir)/dbus/libdbus-1.la \
	$(NULL)
test_pending_call_disconnected_LDADD = \
	$(CODE_COVERAGE_LIBS) \
	$(top_builddir)/dbus/libdbus-1.la \
	$(NULL)
test_threads_init_LDADD = \
	$(CODE_COVERAGE_LIBS) \
	$(top_builddir)/dbus/libdbus-1.la \
	$(NULL)
test_ids_LDADD = \
	$(CODE_COVERAGE_LIBS) \
	$(top_builddir)/dbus/libdbus-1.la \
	$(NULL)

test_shutdown_LDADD = \
	$(CODE_COVERAGE_LIBS) \
	../libdbus-testutils.la \
	$(NULL)
test_privserver_client_LDADD = \
	$(CODE_COVERAGE_LIBS) \
	../libdbus-testutils.la \
	$(NULL)
test_autolaunch_LDADD = \
	$(CODE_COVERAGE_LIBS) \
	../libdbus-testutils.la \
	$(NULL)

endif
