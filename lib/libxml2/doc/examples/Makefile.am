AM_CPPFLAGS = -I$(top_builddir)/include -I$(top_srcdir)/include
LDADD = $(top_builddir)/libxml2.la

CLEANFILES = *.tmp

rebuild:
	cd $(srcdir) && $(PYTHON) index.py
	$(MAKE) Makefile
	cd $(srcdir) && xsltproc examples.xsl examples.xml
	-cd $(srcdir) && xmllint --valid --noout index.html

.PHONY: rebuild

install-data-local: 
	$(MKDIR_P) $(DESTDIR)$(docdir)/examples
	-$(INSTALL) -m 0644 $(srcdir)/*.html $(srcdir)/*.c $(DESTDIR)$(docdir)/examples/

uninstall-local:
	rm -f $(DESTDIR)$(docdir)/examples/*.c
	rm -f $(DESTDIR)$(docdir)/examples/*.html
	-rmdir $(DESTDIR)$(docdir)/examples

clean-local:
	test -f Makefile.am || rm -f test?.xml

EXTRA_DIST = \
	examples.xml \
	examples.xsl \
	index.html \
	index.py \
	test1.xml \
	test2.xml \
	test3.xml

check_PROGRAMS = \
	io1 \
	io2 \
	parse1 \
	parse2 \
	parse3 \
	parse4 \
	reader1 \
	reader2 \
	reader3 \
	reader4 \
	testWriter \
	tree1 \
	tree2 \
	xpath1 \
	xpath2

io1_SOURCES = io1.c

io2_SOURCES = io2.c

parse1_SOURCES = parse1.c

parse2_SOURCES = parse2.c

parse3_SOURCES = parse3.c

parse4_SOURCES = parse4.c

reader1_SOURCES = reader1.c

reader2_SOURCES = reader2.c

reader3_SOURCES = reader3.c

reader4_SOURCES = reader4.c

testWriter_SOURCES = testWriter.c

tree1_SOURCES = tree1.c

tree2_SOURCES = tree2.c

xpath1_SOURCES = xpath1.c

xpath2_SOURCES = xpath2.c

valgrind: 
	$(MAKE) CHECKER='valgrind' check

check-local:
	@test -f Makefile.am || test -f test1.xml || $(LN_S) $(srcdir)/test?.xml .
	@(echo '## examples regression tests')
	@$(CHECKER) ./io1 >/dev/null
	@$(CHECKER) ./io2 >/dev/null
	@$(CHECKER) ./parse1 test1.xml
	@$(CHECKER) ./parse2 test2.xml
	@$(CHECKER) ./parse3
	@$(CHECKER) ./parse4 test3.xml
	@$(CHECKER) ./reader1 test2.xml >/dev/null
	@$(CHECKER) ./reader2 test2.xml >/dev/null
	@$(CHECKER) ./reader3 >/dev/null
	@$(CHECKER) ./reader4 test1.xml test2.xml test3.xml >/dev/null
	@$(CHECKER) ./testWriter
	@$(CHECKER) ./tree1 test2.xml >/dev/null
	@$(CHECKER) ./tree2 >/dev/null
	@$(CHECKER) ./xpath2 test3.xml '//discarded' discarded >/dev/null
	@rm -f *.tmp
