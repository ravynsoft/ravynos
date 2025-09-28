# Makefile for libxml2 python library

# We use a rule with multiple output files which creates problems with
# parallel builds.
.NOTPARALLEL:

SUBDIRS = . tests

EXTRA_DIST = 			\
	generator.py		\
	libxml.py		\
	libxml2-python-api.xml  \
	pyproject.toml

if WITH_PYTHON
AM_CPPFLAGS = \
	-I$(top_builddir)/include \
	-I$(top_srcdir)/include	\
	$(PYTHON_CFLAGS)

pyexec_LTLIBRARIES = libxml2mod.la

libxml2mod_la_SOURCES = libxml.c libxml_wrap.h types.c
nodist_libxml2mod_la_SOURCES = libxml2-py.h libxml2-py.c
libxml2mod_la_LDFLAGS = $(AM_LDFLAGS) $(PYTHON_LDFLAGS) -module -avoid-version
libxml2mod_la_LIBADD = $(top_builddir)/libxml2.la $(PYTHON_LIBS)

BUILT_SOURCES = libxml2-export.c libxml2-py.h libxml2-py.c

python_PYTHON = drv_libxml2.py
nodist_python_PYTHON = libxml2.py

API_DESC = $(top_srcdir)/doc/libxml2-api.xml $(srcdir)/libxml2-python-api.xml
GENERATED = libxml2class.py libxml2class.txt $(BUILT_SOURCES)
CLEANFILES = libxml2.py $(GENERATED)

all-local: libxml2.py

$(GENERATED): $(srcdir)/generator.py $(API_DESC)
	$(PYTHON) $(srcdir)/generator.py $(srcdir)

# libxml.c #includes libxml2-export.c
libxml.$(OBJEXT): libxml2-export.c

libxml2.py: $(srcdir)/libxml.py libxml2class.py
	cat $(srcdir)/libxml.py `test -f libxml2class.py || echo $(srcdir)/`libxml2class.py > $@

clean-local:
	rm -rf __pycache__ *.pyc

endif
