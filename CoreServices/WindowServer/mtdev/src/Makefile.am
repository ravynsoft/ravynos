lib_LTLIBRARIES = libmtdev.la

libmtdev_la_LDFLAGS = -version-info @LIB_VERSION@

libmtdev_la_SOURCES = \
	$(top_srcdir)/include/mtdev-mapping.h \
	$(top_srcdir)/include/mtdev-plumbing.h \
	common.h \
	evbuf.h \
	iobuf.h \
	match.h \
	state.h \
	caps.c \
	core.c \
	iobuf.c \
	match.c \
	match_four.c

AM_CFLAGS = $(CWARNFLAGS) #-DMTDEV_NO_LEGACY_ABI

AM_CPPFLAGS = -I$(top_srcdir)/include/ -I$(srcdir)

libmtdevincludedir = $(includedir)
libmtdevinclude_HEADERS = \
	$(top_srcdir)/include/mtdev-mapping.h \
	$(top_srcdir)/include/mtdev-plumbing.h \
	$(top_srcdir)/include/mtdev.h
