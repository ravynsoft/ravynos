# This file is in the public domain.
#
# Makefile configuration - processed by automake.

# General automake options.
AUTOMAKE_OPTIONS = 1.13 foreign no-dependencies
ACLOCAL_AMFLAGS = -I../../m4 -I../../gnulib-m4

# The list of programs that are built.
bin_PROGRAMS = filter

# The source files of the 'filter' program.
filter_SOURCES = filter.c

# Define which styles to install and where.
stylesdir = $(datadir)/color-filter/styles
styles_DATA = filter-default.css

# Define a C macro STYLESDIR indicating where styles will be installed.
AM_CPPFLAGS = -DSTYLESDIR=\"$(stylesdir)\"


# Link with libtextstyle.
filter_LDADD = @LIBTEXTSTYLE@

# Additional files to be distributed.
EXTRA_DIST = autogen.sh autoclean.sh $(styles_DATA)
