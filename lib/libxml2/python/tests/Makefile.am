exampledir = $(docdir)/python/examples
dist_example_DATA = $(PYTESTS) $(XMLS) setup_test.py

PYTESTS=	\
    build.py	\
    attribs.py	\
    tst.py	\
    tstxpath.py	\
    xpathext.py	\
    push.py	\
    pushSAX.py	\
    pushSAXhtml.py \
    error.py	\
    serialize.py\
    validate.py	\
    tstURI.py	\
    cutnpaste.py\
    xpathret.py	\
    xpath.py	\
    outbuf.py	\
    inbuf.py	\
    input_callback.py \
    resolver.py \
    regexp.py	\
    reader.py	\
    reader2.py	\
    reader3.py	\
    reader4.py	\
    reader5.py	\
    reader6.py	\
    reader7.py	\
    reader8.py	\
    readernext.py	\
    walker.py	\
    nsdel.py	\
    ctxterror.py\
    readererr.py\
    relaxng.py	\
    schema.py	\
    thread2.py \
    sync.py \
    tstLastError.py \
    indexes.py \
    dtdvalid.py \
    tstmem.py	\
    validDTD.py	\
    validSchemas.py \
    validRNG.py \
    compareNodes.py \
    xpathns.py \
    xpathleak.py

XMLS=		\
    tst.xml	\
    valid.xml	\
    invalid.xml	\
    test.dtd

CLEANFILES = core tmp.xml *.pyc

if WITH_PYTHON
check-local:
	@for f in $(XMLS) ; do test -f $$f || $(LN_S) $(srcdir)/$$f . ; done
	@echo "## running Python regression tests"
	@(export PYTHONPATH="..:../.libs:$(srcdir)/..:$$PYTHONPATH" ; \
	  export LD_LIBRARY_PATH="$(top_builddir)/.libs:$$LD_LIBRARY_PATH" ; \
	  export DYLD_LIBRARY_PATH="$(top_builddir)/.libs:$$DYLD_LIBRARY_PATH" ; \
	  export PATH="$(top_builddir)/.libs:$$PATH" ; \
	  for test in $(PYTESTS) ; do \
	    log=`$(PYTHON) $(srcdir)/$$test` ; \
	    if [ "$$?" -ne 0 ] ; then \
	      echo "-- $$test" ; \
	      echo "$$log" ; \
	      exit 1 ; \
	    fi ; \
	  done)
endif
