## Process this file with automake to produce Makefile.in
SUBDIRS = . devhelp examples

nobase_dist_doc_DATA = \
	tutorial/apa.html \
	tutorial/apb.html \
	tutorial/apc.html \
	tutorial/apd.html \
	tutorial/ape.html \
	tutorial/apf.html \
	tutorial/apg.html \
	tutorial/aph.html \
	tutorial/api.html \
	tutorial/ar01s02.html \
	tutorial/ar01s03.html \
	tutorial/ar01s04.html \
	tutorial/ar01s05.html \
	tutorial/ar01s06.html \
	tutorial/ar01s07.html \
	tutorial/ar01s08.html \
	tutorial/ar01s09.html \
	tutorial/images/blank.png \
	tutorial/images/callouts/1.png \
	tutorial/images/callouts/10.png \
	tutorial/images/callouts/2.png \
	tutorial/images/callouts/3.png \
	tutorial/images/callouts/4.png \
	tutorial/images/callouts/5.png \
	tutorial/images/callouts/6.png \
	tutorial/images/callouts/7.png \
	tutorial/images/callouts/8.png \
	tutorial/images/callouts/9.png \
	tutorial/images/caution.png \
	tutorial/images/draft.png \
	tutorial/images/home.png \
	tutorial/images/important.png \
	tutorial/images/next.png \
	tutorial/images/note.png \
	tutorial/images/prev.png \
	tutorial/images/tip.png \
	tutorial/images/toc-blank.png \
	tutorial/images/toc-minus.png \
	tutorial/images/toc-plus.png \
	tutorial/images/up.png \
	tutorial/images/warning.png \
	tutorial/includeaddattribute.c \
	tutorial/includeaddkeyword.c \
	tutorial/includeconvert.c \
	tutorial/includegetattribute.c \
	tutorial/includekeyword.c \
	tutorial/includexpath.c \
	tutorial/index.html \
	tutorial/ix01.html \
	xmlcatalog.html \
	xmllint.html

dist_man_MANS = xml2-config.1 xmllint.1 xmlcatalog.1

EXTRA_DIST = \
	apibuild.py \
	libxml2-api.xml \
	xmlcatalog.xml \
	xmllint.xml

DOCBOOK_HTML = http://docbook.sourceforge.net/release/xsl/current/html/docbook.xsl

rebuild:
	cd $(srcdir) && ./apibuild.py
	cd $(srcdir) && $(XSLTPROC) --nonet xmllint.xml
	cd $(srcdir) && $(XSLTPROC) --nonet -o xmllint.html $(DOCBOOK_HTML) xmllint.xml
	cd $(srcdir) && $(XSLTPROC) --nonet xmlcatalog.xml
	cd $(srcdir) && $(XSLTPROC) --nonet -o xmlcatalog.html $(DOCBOOK_HTML) xmlcatalog.xml
	cd devhelp && $(MAKE) rebuild
	cd examples && $(MAKE) rebuild
	cd .. && $(MAKE) rebuild_testapi

.PHONY: rebuild
