noinst_PROGRAMS = mtdev-mapgen mtdev-matching mtdev-kernel
bin_PROGRAMS = mtdev-test

AM_CPPFLAGS=-I$(top_srcdir) -I$(top_srcdir)/include/

mtdev_mapgen_SOURCES = mtdev-mapgen.c

mtdev_matching_SOURCES = mtdev-matching.c
mtdev_matching_LDADD = $(top_builddir)/src/libmtdev.la

mtdev_test_SOURCES = mtdev-test.c
mtdev_test_LDADD = $(top_builddir)/src/libmtdev.la

mtdev_kernel_SOURCES = mtdev-kernel.c
mtdev_kernel_LDADD = $(top_builddir)/src/libmtdev.la
