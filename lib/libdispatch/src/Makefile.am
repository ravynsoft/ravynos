#
#
#

lib_LTLIBRARIES=libdispatch.la

libdispatch_la_SOURCES=		\
	apply.c					\
	benchmark.c				\
	data.c					\
	init.c					\
	io.c					\
	object.c				\
	once.c					\
	queue.c					\
	semaphore.c				\
	source.c				\
	time.c					\
	transform.c				\
	protocol.defs			\
	provider.d				\
	data_internal.h			\
	internal.h				\
	io_internal.h			\
	object_internal.h		\
	queue_internal.h		\
	semaphore_internal.h	\
	shims.h					\
	source_internal.h		\
	trace.h					\
	shims/atomic.h			\
	shims/getprogname.h		\
	shims/hw_config.h		\
	shims/perfmon.h			\
	shims/time.h			\
	shims/tsd.h

AM_CPPFLAGS=-I$(top_builddir) -I$(top_srcdir) \
	-I$(top_srcdir)/private -I$(top_srcdir)/os

AM_CFLAGS=-Wall $(VISIBILITY_FLAGS) $(OMIT_LEAF_FP_FLAGS) \
	$(MARCH_FLAGS) $(CBLOCKS_FLAGS) $(KQUEUE_CFLAGS)

libdispatch_la_LDFLAGS=-avoid-version

if HAVE_DARWIN_LD
libdispatch_la_LDFLAGS+=-Wl,-compatibility_version,1 \
	-Wl,-current_version,$(VERSION) -Wl,-dead_strip
endif

if USE_OBJC
libdispatch_la_SOURCES+=object.m
libdispatch_la_OBJCFLAGS=$(AM_CFLAGS) -fobjc-gc
libdispatch_la_LDFLAGS+=-Wl,-upward-lobjc -Wl,-upward-lauto \
	-Wl,-order_file,$(top_srcdir)/xcodeconfig/libdispatch.order \
	-Wl,-alias_list,$(top_srcdir)/xcodeconfig/libdispatch.aliases \
	-Wl,-unexported_symbols_list,$(top_srcdir)/xcodeconfig/libdispatch.unexport
endif

CLEANFILES=
DISTCLEANFILES=System objc

if USE_MIG
BUILT_SOURCES=				\
	protocolUser.c			\
	protocol.h				\
	protocolServer.c		\
	protocolServer.h

nodist_libdispatch_la_SOURCES=$(BUILT_SOURCES)
CLEANFILES+=$(BUILT_SOURCES)

%User.c %.h %Server.c %Server.h: $(abs_srcdir)/%.defs
	$(MIG) -user $*User.c -header $*.h \
	    -server $*Server.c -sheader $*Server.h $<
endif
