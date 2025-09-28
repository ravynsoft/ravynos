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

LIBPORT = $(top_builddir)/port/libport.la
LIBTIFF = $(top_builddir)/libtiff/libtiff.la

EXTRA_DIST = \
	CMakeLists.txt

(echo tifftoolsunsupported: $(TIFF_TOOLS_UNSUPPORTED))
if TIFF_TOOLS_UNSUPPORTED
bin_PROGRAMS = \
	tiff2pdf \
	tiff2ps
endif

EXTRA_PROGRAMS = 

if HAVE_RPATH
AM_LDFLAGS = $(LIBDIR)
endif

tiff2pdf_SOURCES = tiff2pdf.c
tiff2pdf_LDADD = $(LIBTIFF) $(LIBPORT)

tiff2ps_SOURCES = tiff2ps.c
tiff2ps_LDADD = $(LIBTIFF) $(LIBPORT)

AM_CPPFLAGS = -I$(top_srcdir)/libtiff -I$(top_srcdir)/port

echo:
	(echo $(CFLAGS))
	(echo tifftoolsunsupported: $(TIFF_TOOLS_UNSUPPORTED))
