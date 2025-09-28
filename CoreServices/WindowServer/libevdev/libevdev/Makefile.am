lib_LTLIBRARIES=libevdev.la

AM_CPPFLAGS = $(GCC_CFLAGS) $(GCOV_CFLAGS) -I$(top_srcdir)/include -I$(top_srcdir)
AM_LDFLAGS = $(GCOV_LDFLAGS)

libevdev_la_SOURCES = \
                   libevdev.h \
                   libevdev-int.h \
                   libevdev-util.h \
                   libevdev-uinput.c \
                   libevdev-uinput.h \
                   libevdev-uinput-int.h \
                   libevdev.c \
                   libevdev-names.c \
		   ../include/linux/input.h \
		   ../include/linux/uinput.h \
		   ../include/linux/@OS@/input-event-codes.h \
		   ../include/linux/@OS@/input.h \
		   ../include/linux/@OS@/uinput.h

libevdev_la_LDFLAGS = \
        $(AM_LDFLAGS) \
	-version-info $(LIBEVDEV_LT_VERSION) \
	-Wl,--version-script="$(srcdir)/libevdev.sym" \
	$(GNU_LD_FLAGS)

EXTRA_libevdev_la_DEPENDENCIES = $(srcdir)/libevdev.sym

libevdevincludedir = $(includedir)/libevdev-1.0/libevdev
libevdevinclude_HEADERS = libevdev.h libevdev-uinput.h

event-names.h: Makefile make-event-names.py
	$(PYTHON) $(srcdir)/make-event-names.py $(top_srcdir)/include/linux/@OS@/input.h $(top_srcdir)/include/linux/@OS@/input-event-codes.h  > $@


EXTRA_DIST = make-event-names.py libevdev.sym
CLEANFILES = event-names.h
BUILT_SOURCES = event-names.h

if GCOV_ENABLED
CLEANFILES += *.gcno
endif
