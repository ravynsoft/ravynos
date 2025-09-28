# Tag Image File Format (TIFF) Software
#
# Copyright (C) 2004, Andrey Kiselev <dron@ak4719.spb.edu>
#
# Permission to use, copy, modify, distribute, and sell this software and 
# its documentation for any purpose is hereby granted without fee, provided
# that (i) the above copyright notices and this permission notice appear in
# all copies of the software and related documentation, and (ii) the names of
# Sam Leffler and Silicon Graphics may not be used in any advertising or
# publicity relating to the software without the specific, prior written
# permission of Sam Leffler and Silicon Graphics.
# 
# THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
# EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
# WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
# 
# IN NO EVENT SHALL SAM LEFFLER OR SILICON GRAPHICS BE LIABLE FOR
# ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
# OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
# WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
# LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
# OF THIS SOFTWARE.

# Process this file with automake to produce Makefile.in.

AUTOMAKE_OPTIONS = 1.12 color-tests parallel-tests foreign

LIBTIFF = $(top_builddir)/libtiff/libtiff.la

# Environment parameters to be used during tests
TESTS_ENVIRONMENT = \
	MAKE="$(MAKE)" \
	MAKEFLAGS="$(MAKEFLAGS)" \
	MEMCHECK="$(MEMCHECK)"

EXTRA_DIST = \
	$(REFFILES) \
	$(TESTSCRIPTS) \
	$(IMAGES_EXTRA_DIST) \
	CMakeLists.txt \
	common.sh \
	TiffSplitTest.cmake \
	TiffTestCommon.cmake \
	TiffTest.cmake

# All of the tests to execute via 'make check'
if TIFF_TESTS
TESTS = $(check_PROGRAMS) $(TESTSCRIPTS)
endif

# Tests which are expected to fail
XFAIL_TESTS =

# Extra files which should be cleaned by 'make clean'
CLEANFILES = test_packbits.tif o-*

if HAVE_JPEG
if TIFF_TOOLS
JPEG_DEPENDENT_CHECK_PROG=raw_decode
# No tiff2rgba tool anymore and therefore not JPEG_DEPENDENT_TESTSCRIPTS
JPEG_DEPENDENT_TESTSCRIPTS=
endif
else
JPEG_DEPENDENT_CHECK_PROG=
JPEG_DEPENDENT_TESTSCRIPTS=
endif

if BUILD_STATIC
STATIC_CHECK_PROGS=rational_precision2double test_write_read_tags test_transferfunction_write_read
endif

# Executable programs which need to be built in order to support tests
if TIFF_TESTS
check_PROGRAMS = \
	ascii_tag long_tag short_tag strip_rw rewrite custom_dir custom_dir_EXIF_231 \
	defer_strile_loading defer_strile_writing test_directory test_open_options \
	test_append_to_strip test_ifd_loop_detection testtypes test_signed_tags $(JPEG_DEPENDENT_CHECK_PROG) $(STATIC_CHECK_PROGS)
endif

# Test scripts to execute
if TIFF_TOOLS
TESTSCRIPTS = \
	tiffcp-g3.sh \
	tiffcp-g3-1d.sh \
	tiffcp-g3-1d-fill.sh \
	tiffcp-g3-2d.sh \
	tiffcp-g3-2d-fill.sh \
	tiffcp-g4.sh \
	tiffcp-logluv.sh \
	tiffcp-lzw-compat.sh \
	tiffcp-lzw-scanline-decode.sh \
	tiffdump.sh \
	tiffinfo.sh \
	tiffcp-split.sh \
	tiffcp-split-join.sh \
	$(JPEG_DEPENDENT_TESTSCRIPTS)
else
TESTSCRIPTS=
endif

# This list should contain the references files
# from the 'refs' subdirectory
REFFILES = \
	refs/o-tiff2ps-EPS1.ps \
	refs/o-tiff2ps-PS1.ps \
	refs/o-tiff2ps-PS2.ps \
	refs/o-tiff2ps-PS3.ps \
	refs/o-testfax4.tiff \
	refs/o-testfax3_bug_513.tiff \
	refs/o-deflate-last-strip-extra-data.tiff

# This list should contain all of the TIFF files in the 'images'
# subdirectory which are intended to be used as input images for
# tests.  All of these files should use the extension ".tiff".
TIFFIMAGES = \
	images/logluv-3c-16b.tiff \
	images/minisblack-1c-16b.tiff \
	images/minisblack-1c-8b.tiff \
	images/minisblack-2c-8b-alpha.tiff \
	images/miniswhite-1c-1b.tiff \
	images/palette-1c-1b.tiff \
	images/palette-1c-4b.tiff \
	images/palette-1c-8b.tiff \
	images/rgb-3c-16b.tiff \
	images/rgb-3c-8b.tiff \
	images/quad-tile.jpg.tiff \
	images/quad-lzw-compat.tiff \
	images/lzw-single-strip.tiff \
	images/ojpeg_zackthecat_subsamp22_single_strip.tiff \
	images/ojpeg_chewey_subsamp21_multi_strip.tiff \
	images/ojpeg_single_strip_no_rowsperstrip.tiff \
	images/testfax4.tiff \
	images/deflate-last-strip-extra-data.tiff \
	images/custom_dir_EXIF_GPS.tiff \
	images/testfax3_bug_513.tiff

PNMIMAGES = \
	images/minisblack-1c-8b.pgm \
	images/miniswhite-1c-1b.pbm \
	images/rgb-3c-16b.ppm \
	images/rgb-3c-8b.ppm

# This list should include all of the files in the 'images'
# subdirectory which are intended to be distributed.  This may include
# files which are not currently used by the tests.
IMAGES_EXTRA_DIST = \
	images/README.txt \
	images/miniswhite-1c-1b.g3 \
	images/test_float64_predictor2_le_lzw.tif \
	images/test_float64_predictor2_be_lzw.tif \
	images/tiff_with_subifd_chain.tif \
	images/test_ifd_loop_to_self.tif \
	images/test_ifd_loop_to_first.tif \
	images/test_two_ifds.tif \
	images/test_ifd_loop_subifd.tif \
	images/webp_lossless_rgba_alpha_fully_opaque.tif \
	$(PNMIMAGES) \
	$(TIFFIMAGES)

noinst_HEADERS = tifftest.h

test_signed_tags_SOURCES = test_signed_tags.c
test_signed_tags_LDADD = $(LIBTIFF)
ascii_tag_SOURCES = ascii_tag.c
ascii_tag_LDADD = $(LIBTIFF)
long_tag_SOURCES = long_tag.c check_tag.c
long_tag_LDADD = $(LIBTIFF)
short_tag_SOURCES = short_tag.c check_tag.c
short_tag_LDADD = $(LIBTIFF)
strip_rw_SOURCES = strip_rw.c strip.c test_arrays.c test_arrays.h
strip_rw_LDADD = $(LIBTIFF)
rewrite_SOURCES = rewrite_tag.c
rewrite_LDADD = $(LIBTIFF)
raw_decode_SOURCES = raw_decode.c
raw_decode_LDADD = $(LIBTIFF)
custom_dir_SOURCES = custom_dir.c
custom_dir_LDADD = $(LIBTIFF)

if BUILD_STATIC
rational_precision2double_SOURCES = rational_precision2double.c
rational_precision2double_LDADD = $(LIBTIFF)
rational_precision2double_LDFLAGS = -static

test_write_read_tags_SOURCES = test_write_read_tags.c
test_write_read_tags_LDADD = $(LIBTIFF)
test_write_read_tags_LDFLAGS = -static

test_transferfunction_write_read_SOURCES = test_transferfunction_write_read.c
test_transferfunction_write_read_LDADD = $(LIBTIFF)
test_transferfunction_write_read_LDFLAGS = -static
endif

custom_dir_EXIF_231_SOURCES = custom_dir_EXIF_231.c
custom_dir_EXIF_231_LDADD = $(LIBTIFF)
defer_strile_loading_SOURCES = defer_strile_loading.c
defer_strile_loading_LDADD = $(LIBTIFF)
defer_strile_writing_SOURCES = defer_strile_writing.c
defer_strile_writing_LDADD = $(LIBTIFF)
test_directory_SOURCES = test_directory.c
test_directory_LDADD = $(LIBTIFF)
test_open_options_SOURCES = test_open_options.c
test_open_options_LDADD = $(LIBTIFF)
test_append_to_strip_SOURCES = test_append_to_strip.c
test_append_to_strip_LDADD = $(LIBTIFF)
test_ifd_loop_detection_CFLAGS = -DSOURCE_DIR=\"@srcdir@\"
test_ifd_loop_detection_SOURCES = test_ifd_loop_detection.c
test_ifd_loop_detection_LDADD = $(LIBTIFF)

AM_CPPFLAGS = -I$(top_srcdir)/libtiff

# memcheck: valgrind's memory access checker.
#
# The suppressions which come with valgrind are sometimes insufficient
# to handle certain system library aspects which may be reported and
# which are unrelated to libtiff.  When first starting with a new
# system (or after a major system update), it is good to execute the
# test suite (known to already be passing!) like 'make memcheck
# VALGRIND_EXTRA_OPTS=--gen-suppressions=all' to create valgrind
# suppression entries in the test log.  Make sure that none of the
# suppressions are due to libtiff itself.  Tell valgrind about the
# suppressions by creating a .valgrindrc file with content like:
# --memcheck:suppressions=mysupp.supp
memcheck:
	$(MAKE) MEMCHECK='valgrind --tool=memcheck --leak-check=full --read-var-info=yes \
	--error-exitcode=2 --track-origins=yes --num-callers=12 \
	--quiet $(VALGRIND_EXTRA_OPTS)' check

# ptrcheck: valgrind's experimental pointer checking tool.
ptrcheck:
	$(MAKE) MEMCHECK='valgrind --tool=exp-ptrcheck --quiet $(VALGRIND_EXTRA_OPTS)' check

