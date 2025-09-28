## Process this file with automake to generate Makefile.in
#
#   Copyright (C) 2021-2023 Free Software Foundation, Inc.
#
# This file is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING3.  If not see
# <http://www.gnu.org/licenses/>.

AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I . -I ../.. 

CSOURCES = \
	gethrtime.c \
	dispatcher.c \
	iolib.c \
	mmaptrace.c \
	memmgr.c \
	tsd.c \
	profile.c \
	envmgmt.c \
	linetrace.c \
	libcol_hwcdrv.c \
	libcol_hwcfuncs.c \
	libcol-i386-dis.c \
	hwprofile.c \
	jprofile.c \
	unwind.c \
	libcol_util.c \
	collector.c \
	$(NULL)

AM_CFLAGS = $(GPROFNG_CFLAGS) $(GPROFNG_NO_NONNULL_COMPARE_CFLAGS)
AM_CPPFLAGS = $(GPROFNG_CPPFLAGS) -I.. -I$(srcdir) \
	-I$(srcdir)/../common -I$(srcdir)/../src \
	-I$(srcdir)/../../include
AM_LDFLAGS = -module -avoid-version \
	-Wl,--version-script,$(srcdir)/libgprofng.ver \
	$(LD_NO_AS_NEEDED) -Wl,-lrt -Wl,-ldl

myincludedir = @includedir@
myinclude_HEADERS = $(srcdir)/../../include/collectorAPI.h \
	$(srcdir)/../../include/libcollector.h \
	$(srcdir)/../../include/libfcollector.h

pkglib_LTLIBRARIES = libgp-collector.la libgp-collectorAPI.la libgp-heap.la \
	libgp-sync.la libgp-iotrace.la

libgp_collector_la_SOURCES = $(CSOURCES)
libgp_collector_la_CPPFLAGS = $(AM_CPPFLAGS) $(jdk_inc) \
	-I../../bfd -I$(srcdir)/../..
# Prevent libtool from reordering -Wl,--no-as-needed after -lrt by
# disguising -lrt as a linker flag.
libgp_collector_la_LDFLAGS = $(AM_LDFLAGS)
libgp_collector_la_LIBADD = $(CLOCK_GETTIME_LINK)

libgp_heap_la_SOURCES = heaptrace.c
libgp_heap_la_LDFLAGS = $(AM_LDFLAGS)

libgp_sync_la_SOURCES = synctrace.c
libgp_sync_la_LDFLAGS = $(AM_LDFLAGS)

libgp_iotrace_la_SOURCES = iotrace.c
libgp_iotrace_la_LDFLAGS = $(AM_LDFLAGS)

libgp_collectorAPI_la_SOURCES = collectorAPI.c
libgp_collectorAPI_la_LIBADD = -lc -ldl

.PHONY: install-data-local

install-data-local: install-pkglibLTLIBRARIES
	for f in libgp-collector libgp-heap libgp-sync libgp-iotrace ; do \
	  rm -f $(DESTDIR)/$(pkglibdir)/$$f.la $(DESTDIR)/$(pkglibdir)/$$f.a ; \
	done
