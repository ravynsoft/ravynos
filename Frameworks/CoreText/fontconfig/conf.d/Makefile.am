#
#  fontconfig/conf.d/Makefile.am
#
#  Copyright © 2005 Keith Packard
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

NULL =
BUILT_SOURCES = 		\
	README			\
	35-lang-normalize.conf	\
	$(NULL)
DOC_SOURCES = README.in
DOC_FILES = $(DOC_SOURCES:.in=)

CONF_LINKS = \
	10-hinting-$(PREFERRED_HINTING).conf	\
	10-scale-bitmap-fonts.conf \
	10-yes-antialias.conf	\
	10-sub-pixel-$(PREFERRED_SUB_PIXEL_RENDERING).conf	\
	11-lcdfilter-default.conf \
	20-unhint-small-vera.conf \
	30-metric-aliases.conf \
	40-nonlatin.conf \
	45-generic.conf \
	45-latin.conf \
	48-spacing.conf \
	49-sansserif.conf \
	50-user.conf \
	51-local.conf \
	60-generic.conf \
	60-latin.conf \
	65-fonts-persian.conf \
	65-nonlatin.conf \
	69-unifont.conf \
	80-delicious.conf \
	90-synthetic.conf

EXTRA_DIST = $(template_DATA) $(DOC_SOURCES)
CLEANFILES = $(DOC_FILES)

configdir = $(CONFIGDIR)
config_DATA = $(DOC_FILES)

templatedir = $(TEMPLATEDIR)
template_DATA =				\
	05-reset-dirs-sample.conf	\
	09-autohint-if-no-hinting.conf	\
	10-autohint.conf		\
	10-hinting-full.conf		\
	10-hinting-medium.conf		\
	10-hinting-none.conf		\
	10-hinting-slight.conf		\
	10-no-antialias.conf		\
	10-scale-bitmap-fonts.conf	\
	10-sub-pixel-bgr.conf		\
	10-sub-pixel-none.conf		\
	10-sub-pixel-rgb.conf		\
	10-sub-pixel-vbgr.conf		\
	10-sub-pixel-vrgb.conf		\
	10-unhinted.conf		\
	10-yes-antialias.conf		\
	11-lcdfilter-default.conf	\
	11-lcdfilter-legacy.conf	\
	11-lcdfilter-light.conf		\
	20-unhint-small-vera.conf	\
	25-unhint-nonlatin.conf		\
	30-metric-aliases.conf		\
	35-lang-normalize.conf		\
	40-nonlatin.conf		\
	45-generic.conf			\
	45-latin.conf			\
	48-spacing.conf			\
	49-sansserif.conf		\
	50-user.conf			\
	51-local.conf			\
	60-generic.conf			\
	60-latin.conf			\
	65-fonts-persian.conf		\
	65-khmer.conf			\
	65-nonlatin.conf		\
	69-unifont.conf			\
	70-no-bitmaps.conf		\
	70-yes-bitmaps.conf		\
	80-delicious.conf		\
	90-synthetic.conf

README: $(srcdir)/README.in
	sed "s|\@TEMPLATEDIR\@|$(templatedir)|" $< > $@

35-lang-normalize.conf: ../fc-lang/Makefile.am
	cd ../fc-lang && $(MAKE) $(AM_MAKEFLAGS) $(top_builddir)/conf.d/35-lang-normalize.conf

install-data-hook:
	mkdir -p $(DESTDIR)$(configdir)
	@(echo cd $(DESTDIR)$(configdir);			\
	  cd $(DESTDIR)$(configdir);				\
	  for i in $(CONF_LINKS); do				\
	    echo $(RM) $$i";" ln -s $(templatedir)/$$i .;	\
	    $(RM) $$i;						\
	    ln -s $(templatedir)/$$i .;				\
	  done)
uninstall-local:
	@(echo cd $(DESTDIR)$(configdir);			\
	  cd $(DESTDIR)$(configdir);				\
	  for i in $(CONF_LINKS); do				\
	    echo $(RM) $$i;					\
	    $(RM) $$i;						\
	  done)

-include $(top_srcdir)/git.mk
