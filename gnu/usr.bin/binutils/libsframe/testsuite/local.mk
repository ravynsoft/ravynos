# Setup the testing framework
EXPECT = expect
RUNTEST = runtest
RUNTESTFLAGS =

check-DEJAGNU: site.exp
	srcroot=`cd $(srcdir) && pwd`; export srcroot; \
	r=`pwd`; export r; \
	LC_ALL=C; export LC_ALL; \
	EXPECT=$(EXPECT); export EXPECT; \
	runtest=$(RUNTEST); \
	if $(SHELL) -c "$$runtest --version" > /dev/null 2>&1; then \
	  $$runtest --tool $(DEJATOOL) --srcdir $${srcroot}/testsuite \
		CC="$(CC)" \
		CROSS_COMPILE="$(CROSS_COMPILE)" \
		COMPAT_DEJAGNU="$(COMPAT_DEJAGNU)" \
		CFLAGS="$(CFLAGS) -I$(top_srcdir)/../include -I$(top_srcdir) -I$(top_builddir)" \
		$(RUNTESTFLAGS); \
	else echo "WARNING: could not find \`runtest'" 1>&2; :;\
	fi

# libsframe encoder/decoder/find testsuite
include %D%/libsframe.decode/local.mk
include %D%/libsframe.encode/local.mk
include %D%/libsframe.find/local.mk
