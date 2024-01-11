AM_CPPFLAGS = \
	-I$(top_srcdir) \
	$(CODE_COVERAGE_CPPFLAGS) \
	$(DBUS_STATIC_BUILD_CPPFLAGS) \
	$(DBUS_X_CFLAGS) \
	-DDBUS_COMPILATION \
	-DDBUS_MACHINE_UUID_FILE=\""$(localstatedir)/lib/dbus/machine-id"\" \
	$(NULL)

AM_CFLAGS = \
	$(CODE_COVERAGE_CFLAGS) \
	$(SANITIZE_CFLAGS) \
	$(NULL)

# if assertions are enabled, improve backtraces
AM_LDFLAGS = @R_DYNAMIC_LDFLAG@

bin_PROGRAMS = \
	dbus-launch \
	dbus-monitor \
	dbus-run-session \
	dbus-send \
	dbus-test-tool \
	dbus-update-activation-environment \
	$(NULL)

if DBUS_UNIX
bin_PROGRAMS += \
	dbus-cleanup-sockets \
	dbus-uuidgen \
	$(NULL)
endif

dbus_send_SOURCES=				\
	dbus-print-message.c			\
	dbus-print-message.h			\
	dbus-send.c \
	tool-common.c \
	tool-common.h \
	$(NULL)

dbus_monitor_SOURCES = \
	dbus-monitor.c \
	dbus-print-message.c \
	dbus-print-message.h \
	tool-common.c \
	tool-common.h \
	$(NULL)

if DBUS_WIN
dbus_launch_SOURCES=				\
	dbus-launch-win.c			\
	dbus-launch.h
else
dbus_launch_SOURCES=				\
	dbus-launch.c				\
	dbus-launch-x11.c			\
	dbus-launch.h \
	tool-common.c \
	tool-common.h \
	$(NULL)
endif

dbus_launch_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	$(NULL)
dbus_launch_LDADD = \
	$(CODE_COVERAGE_LIBS) \
	$(top_builddir)/dbus/libdbus-1.la \
	$(DBUS_X_LIBS) \
	$(NULL)

dbus_run_session_SOURCES =			\
	dbus-run-session.c \
	tool-common.c \
	tool-common.h
	$(NULL)

dbus_run_session_LDADD = \
	$(CODE_COVERAGE_LIBS) \
	$(top_builddir)/dbus/libdbus-internal.la \
	$(NULL)

dbus_cleanup_sockets_SOURCES=			\
	dbus-cleanup-sockets.c

dbus_uuidgen_SOURCES=				\
	dbus-uuidgen.c

dbus_send_LDADD = \
	$(CODE_COVERAGE_LIBS) \
	$(top_builddir)/dbus/libdbus-1.la \
	$(NULL)

dbus_monitor_LDADD = \
	$(CODE_COVERAGE_LIBS) \
	$(top_builddir)/dbus/libdbus-1.la \
	$(NETWORK_libs) \
	$(NULL)

dbus_uuidgen_LDADD = \
	$(CODE_COVERAGE_LIBS) \
	$(top_builddir)/dbus/libdbus-1.la \
	$(NULL)

examplesdir = ${docdir}/examples
dist_examples_SCRIPTS = \
	GetAllMatchRules.py \
	$(NULL)

dbus_test_tool_SOURCES = \
	dbus-echo.c \
	dbus-spam.c \
	tool-common.c \
	tool-common.h \
	test-tool.c \
	test-tool.h \
	$(NULL)
dbus_test_tool_LDADD = \
	$(CODE_COVERAGE_LIBS) \
	$(top_builddir)/dbus/libdbus-1.la \
	$(NULL)

dbus_update_activation_environment_SOURCES = \
	dbus-update-activation-environment.c \
	tool-common.c \
	tool-common.h \
	$(NULL)
dbus_update_activation_environment_LDADD = \
	$(CODE_COVERAGE_LIBS) \
	$(top_builddir)/dbus/libdbus-1.la \
	$(NULL)

if DBUS_WIN
SUFFIXES = .rc

.rc.o:
	$(RC) $< -o $@

nodist_dbus_update_activation_environment_SOURCES = disable-uac.rc

disable-uac.rc: Win32.Manifest
	echo '1 24 "$<"' > $@
endif

EXTRA_DIST = run-with-tmp-session-bus.sh strtoll.c strtoull.c Win32.Manifest
CLEANFILES = 				\
	run-with-tmp-session-bus.conf \
	$(nodist_dbus_update_activation_environment_SOURCES)

# create the /var/lib/dbus directory for dbus-uuidgen
install-data-local:
	$(MKDIR_P) $(DESTDIR)$(localstatedir)/lib/dbus

installcheck-local:
	test -d $(DESTDIR)$(localstatedir)/lib/dbus
