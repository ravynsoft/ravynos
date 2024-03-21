# This file is in the public domain.
#
# Makefile configuration - processed by automake.

# General automake options.
AUTOMAKE_OPTIONS = 1.13 foreign no-dependencies
ACLOCAL_AMFLAGS = -I../../m4 -I../../gnulib-m4

# The list of programs that are built.
bin_PROGRAMS = hello

# The source files of the 'hello' program.
hello_SOURCES = hello.c
noinst_HEADERS = names.c

# Define which styles to install and where.
stylesdir = $(datadir)/color-hello/styles
styles_DATA = hello-default.css

# Define a C macro STYLESDIR indicating where styles will be installed.
AM_CPPFLAGS = -DSTYLESDIR=\"$(stylesdir)\"


# Link with libtextstyle.
hello_LDADD = @LIBTEXTSTYLE@

# Additional files to be distributed.
EXTRA_DIST = autogen.sh autoclean.sh $(styles_DATA)
