man3_MANS = libevdev.man

if HAVE_DOXYGEN

noinst_DATA = html/index.html

header_files = \
	$(top_srcdir)/libevdev/libevdev.h \
	$(top_srcdir)/libevdev/libevdev-uinput.h

html/index.html: libevdev.doxygen style/libevdevdoxygen.css $(header_files)
	$(AM_V_GEN)$(DOXYGEN) $<

clean-local:
	$(AM_V_at)rm -rf html

doc_src= $(shell find html -type f -printf "html/%P\n" 2>/dev/null)
style_src = \
	style/bootstrap.css \
	style/customdoxygen.css \
	style/doxy-boot.js \
	style/dynsections.js \
	style/footer.html \
	style/header.html \
	style/layout.xml \
	style/libevdevdoxygen.css \
	style/LICENSE \
	style/README.md \
	style/style.css

EXTRA_DIST = html/index.html $(doc_src) $(style_src)

endif

# make sure doc was built before running dist
dist-hook:
	@test -f $(distdir)/html/index.html || (\
		echo "******************************************************" && \
		echo "Couldn't find documentation files, refusing make dist." && \
		echo "Install doxygen to build documentation for tarball." && \
		echo "******************************************************" && \
		test )
