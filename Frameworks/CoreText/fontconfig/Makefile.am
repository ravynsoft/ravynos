# 
#  fontconfig/Makefile.am
# 
#  Copyright © 2003 Keith Packard
# 
#  Permission to use, copy, modify, distribute, and sell this software and its
#  documentation for any purpose is hereby granted without fee, provided that
#  the above copyright notice appear in all copies and that both that
#  copyright notice and this permission notice appear in supporting
#  documentation, and that the name of the author(s) not be used in
#  advertising or publicity pertaining to distribution of the software without
#  specific, written prior permission.  The authors make no
#  representations about the suitability of this software for any purpose.  It
#  is provided "as is" without express or implied warranty.
# 
#  THE AUTHOR(S) DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
#  INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
#  EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY SPECIAL, INDIRECT OR
#  CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
#  DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
#  TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
#  PERFORMANCE OF THIS SOFTWARE.

SUBDIRS=fontconfig fc-case fc-lang src \
	fc-cache fc-cat fc-conflist fc-list fc-match \
	fc-pattern fc-query fc-scan fc-validate conf.d \
	its po po-conf test
if ENABLE_DOCS
SUBDIRS += doc
endif

ACLOCAL_AMFLAGS = -I m4

MESON_FILES = \
	conf.d/link_confs.py \
	conf.d/write-35-lang-normalize-conf.py \
	doc/edit-sgml.py \
	doc/extract-man-list.py \
	doc/run-quiet.py \
	fc-case/fc-case.py \
	fc-lang/fc-lang.py \
	meson.build \
	meson_options.txt \
	src/cutout.py \
	src/fcstdint.h.in \
	src/fcwindows.h \
	src/fontconfig.def.in \
	src/makealias.py \
	$(wildcard $(srcdir)/*/meson.build) \
	$(wildcard $(srcdir)/meson-cc-tests/*) \
	$(wildcard $(srcdir)/subprojects/*.wrap)

EXTRA_DIST = config.rpath  \
	fontconfig.pc.in \
	fonts.conf.in \
	fonts.dtd \
	fontconfig-zip.in \
	config-fixups.h \
	$(MESON_FILES)
CLEANFILES = fonts.conf
DISTCLEANFILES = config.cache doltcompile
MAINTAINERCLEANFILES = \
	$(srcdir)/aclocal.m4 \
	$(srcdir)/autoscan.log \
	$(srcdir)/compile \
	$(srcdir)/config.guess \
	$(srcdir)/config.h.in \
	$(srcdir)/config.sub \
	$(srcdir)/configure.scan \
	$(srcdir)/depcomp \
	$(srcdir)/install-sh \
	$(srcdir)/ltmain.sh \
	$(srcdir)/missing \
	$(srcdir)/mkinstalldirs \
	$(srcdir)/test-driver \
	`find "$(srcdir)" -type f -name Makefile.in -print`

pkgconfig_DATA = fontconfig.pc

baseconfigdir = $(BASECONFIGDIR)
configdir = $(CONFIGDIR)

xmldir = $(XMLDIR)
xml_DATA = fonts.dtd

if !ENABLE_CACHE_BUILD
  RUN_FC_CACHE_TEST=false
else
  RUN_FC_CACHE_TEST=test -z "$(DESTDIR)"
endif

# Creating ChangeLog from git log:

MAINTAINERCLEANFILES += $(srcdir)/ChangeLog
EXTRA_DIST += ChangeLog
ChangeLog: $(srcdir)/ChangeLog
$(srcdir)/ChangeLog:
	if test -d "$(srcdir)/.git"; then \
	  (GIT_DIR=$(top_srcdir)/.git $(GIT) log --stat) | fmt --split-only > $@.tmp \
	  && mv -f $@.tmp $@ \
	  || ($(RM) $@.tmp; \
	      echo Failed to generate ChangeLog, your ChangeLog may be outdated >&2; \
	      (test -f $@ || echo git-log is required to generate this file >> $@)); \
	else \
	  test -f $@ || \
	  (echo A git checkout and git-log is required to generate ChangeLog >&2 && \
	  echo A git checkout and git-log is required to generate this file >> $@); \
	fi

.PHONY: ChangeLog

FC_CONFIGDIR = $(subst $(BASECONFIGDIR)/,,$(CONFIGDIR))

fonts.conf: fonts.conf.in Makefile
	sed \
		-e 's,@FC_CACHEDIR\@,$(FC_CACHEDIR),g' \
		-e 's,@FC_DEFAULT_FONTS\@,	$(FC_DEFAULT_FONTS),g' \
		-e 's,@FC_FONTPATH\@,$(FC_FONTPATH),g' \
		-e 's,@CONFIGDIR\@,$(FC_CONFIGDIR),g' \
		-e 's,@PACKAGE\@,$(PACKAGE),g' \
		-e 's,@VERSION\@,$(VERSION),g' \
		$(srcdir)/$@.in > $@.tmp && \
	mv $@.tmp $@

install-data-local: fonts.conf
	$(mkinstalldirs) $(DESTDIR)$(baseconfigdir) $(DESTDIR)$(fc_cachedir)
	if [ -f $(DESTDIR)$(baseconfigdir)/fonts.conf ]; then \
	  echo "backing up existing $(DESTDIR)$(baseconfigdir)/fonts.conf"; \
	  mv $(DESTDIR)$(baseconfigdir)/fonts.conf $(DESTDIR)$(baseconfigdir)/fonts.conf.bak; \
	fi
	if [ -f $(srcdir)/fonts.conf ]; then \
	  echo " $(INSTALL_DATA) $(srcdir)/fonts.conf $(DESTDIR)$(baseconfigdir)/fonts.conf"; \
	  $(INSTALL_DATA) $(srcdir)/fonts.conf $(DESTDIR)$(baseconfigdir)/fonts.conf; \
	else if [ -f fonts.conf ]; then \
	  echo " $(INSTALL_DATA) fonts.conf $(DESTDIR)$(baseconfigdir)/fonts.conf"; \
	  $(INSTALL_DATA) fonts.conf $(DESTDIR)$(baseconfigdir)/fonts.conf; \
	fi; fi
	@(if $(RUN_FC_CACHE_TEST); then \
	    echo "fc-cache$(EXEEXT) -s -f -v"; \
	    fc-cache/fc-cache$(EXEEXT) -s -f -v; \
	else \
	    echo "***"; \
	    echo "*** Warning: fonts.cache not built"; \
	    echo "***"; \
	    echo "*** Generate this file manually on host system using fc-cache"; \
	    echo "***"; \
	fi)

uninstall-local:
	if [ -f $(srcdir)/fonts.conf ]; then \
	  if cmp -s $(srcdir)/fonts.conf $(DESTDIR)$(baseconfigdir)/fonts.conf; then \
	     echo " uninstall standard $(DESTDIR)$(baseconfigdir)/fonts.conf"; \
	     $(RM) $(DESTDIR)$(baseconfigdir)/fonts.conf; \
	  fi; \
	else if [ -f fonts.conf ]; then \
	  if cmp -s fonts.conf $(DESTDIR)$(baseconfigdir)/fonts.conf; then \
	     echo " uninstall standard $(DESTDIR)$(baseconfigdir)/fonts.conf"; \
	     $(RM) $(DESTDIR)$(baseconfigdir)/fonts.conf; \
	  fi; \
	fi; fi

debuild debuild-signed: debuild-dirs
	(cd $(distdir)/debian && debuild)

debuild-unsigned: debuild-dirs
	(cd $(distdir)/debian && debuild -us -uc)

debuild-dirs: distdir
	$(RM) $(PACKAGE)_$(VERSION).orig.tar.gz
	$(RM) -r $(distdir).orig
	cp -a $(distdir) $(distdir).orig
	$(RM) -r $(distdir).orig/debian

DISTCHECK_CONFIGURE_FLAGS =

check-versions:
	@$(GREP) -e "^[[:space:]]*version[[:space:]]*:[[:space:]]*'$(VERSION)'," $(srcdir)/meson.build >/dev/null || { \
	  echo "======================================================================================"; \
	  echo "Meson version does not seem to match autotools version $(VERSION), update meson.build!"; \
	  echo "======================================================================================"; \
	  exit 1; \
	}

all-local: check-versions

-include $(top_srcdir)/git.mk
