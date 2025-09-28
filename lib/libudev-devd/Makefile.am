ACLOCAL_AMFLAGS =	-I m4

lib_LTLIBRARIES =	libudev.la
include_HEADERS =	libudev.h

libudev_la_SOURCES =	tree.h			\
			udev.c			\
			udev.h			\
			udev-device.c		\
			udev-device.h		\
			udev-enumerate.c	\
			udev-filter.c		\
			udev-filter.h		\
			udev-hwdb.c		\
			udev-list.c		\
			udev-list.h		\
			udev-monitor.c		\
			udev-queue.c		\
			udev-utils.c		\
			udev-utils.h		\
			utils.c			\
			utils.h

if ENABLE_GPL
libudev_la_SOURCES +=	utils-gpl.c		\
			utils-gpl.h
endif

libudev_la_LDFLAGS =	-pthread
libudev_la_CFLAGS =	-I$(top_srcdir) -Wall -Werror -fvisibility=hidden

udev_test_SOURCES = udev-test.c
udev_test_LDADD = libudev.la
noinst_PROGRAMS = udev-test

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libudev.pc

EXTRA_DIST =		README			\
			meson.build
