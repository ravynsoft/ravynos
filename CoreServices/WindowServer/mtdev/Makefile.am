SUBDIRS = src test

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = mtdev.pc

AM_CPPFLAGS = $(top_srcdir)/include/

.PHONY: ChangeLog INSTALL
INSTALL:
	$(INSTALL_CMD)
ChangeLog:
	git log > ChangeLog

dist-hook: ChangeLog INSTALL
