build_tests = test-compile-pedantic test-link

if ENABLE_STATIC_LINK_TEST
build_tests += test-static-link
endif

noinst_PROGRAMS = $(build_tests)

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/include -I$(top_builddir)/libevdev
AM_LDFLAGS =

test_compile_pedantic_SOURCES = test-compile-pedantic.c
test_compile_pedantic_CFLAGS = $(AM_CPPFLAGS) -pedantic -Werror -std=c89

test_link_SOURCES = test-link.c
test_link_CFLAGS = -I$(top_srcdir)
test_link_LDADD = $(top_builddir)/libevdev/libevdev.la

test_static_link_SOURCES = test-link.c
test_static_link_CFLAGS = -I$(top_srcdir)
test_static_link_LDADD = $(top_builddir)/libevdev/libevdev.la
test_static_link_LDFLAGS = $(AM_LDFLAGS) -static

check_local_deps =

if ENABLE_RUNTIME_TESTS
run_tests = \
	    test-libevdev \
	    test-kernel \
	    test-uinput \
	    test-event-codes \
	    test-libevdev-internals \
	    $(NULL)

.NOTPARALLEL:

noinst_PROGRAMS += $(run_tests)

TESTS = $(run_tests)

common_sources = \
		 test-common-uinput.c \
		 test-common-uinput.h \
		 test-common.c \
		 test-common.h

# include builddir for event-names.h
AM_CPPFLAGS += $(CHECK_CFLAGS) $(GCOV_CFLAGS)
AM_LDFLAGS += $(GCOV_LDFLAGS)

test_event_codes_SOURCES = \
			test-main.c \
			test-event-codes.c \
			test-event-names.c \
			test-context.c \
			$(common_sources)
test_event_codes_LDADD = $(CHECK_LIBS) $(top_builddir)/libevdev/libevdev.la
test_event_codes_LDFLAGS = -no-install

test_libevdev_internals_SOURCES = \
			test-main.c \
			test-int-queue.c \
			$(common_sources)
test_libevdev_internals_LDADD = $(CHECK_LIBS) $(top_builddir)/libevdev/libevdev.la
test_libevdev_internals_LDFLAGS = -no-install

test_uinput_SOURCES = \
			test-main.c \
			test-uinput.c \
			$(common_sources)
test_uinput_LDADD = $(CHECK_LIBS) $(top_builddir)/libevdev/libevdev.la
test_uinput_LDFLAGS = -no-install

test_libevdev_SOURCES = \
			test-main.c \
			test-libevdev-init.c \
			test-libevdev-has-event.c \
			test-libevdev-events.c \
			$(common_sources)

test_libevdev_LDADD = $(CHECK_LIBS) $(top_builddir)/libevdev/libevdev.la
test_libevdev_LDFLAGS = -no-install

test_kernel_SOURCES = \
		      test-main.c \
		      test-kernel.c \
		      $(common_sources)

test_kernel_CFLAGS = -I$(top_srcdir)
test_kernel_LDADD = $(CHECK_LIBS) $(top_builddir)/libevdev/libevdev.la

if HAVE_VALGRIND
VALGRIND_FLAGS=--leak-check=full \
		--quiet \
		--error-exitcode=3 \
		--suppressions=$(srcdir)/valgrind.suppressions

valgrind:
	        $(MAKE) check-TESTS CK_TIMEOUT_MULTIPLIER=10 LOG_COMPILER="$(VALGRIND)" LOG_FLAGS="$(VALGRIND_FLAGS)"

check_local_deps += valgrind

endif

if GCOV_ENABLED

CLEANFILES = gcov-reports/*.gcov gcov-reports/summary.txt *.gcno *.gcda

gcov-report: generate-gcov-report.sh check-TESTS
	$(AM_V_GEN)$(srcdir)/generate-gcov-report.sh gcov-reports $(top_builddir)/libevdev $(builddir)

gcov: gcov-report
	@cat gcov-reports/summary.txt

check_local_deps += gcov

else

gcov-report.txt:
	@true

gcov:
	@true


endif # GCOV_ENABLED

.PHONY: gcov gcov-clean gcov-report

endif # ENABLE_RUNTIME_TESTS

if ENABLE_STATIC_SYMBOL_LEAKS_TEST
static-symbol-leaks: test-static-link test-static-symbols-leak.sh
	$(AM_V_GEN) $(srcdir)/test-static-symbols-leak.sh $(builddir)

check_local_deps += static-symbol-leaks

endif # HAVE_NM

EXTRA_DIST = valgrind.suppressions  generate-gcov-report.sh test-static-symbols-leak.sh

check-local: $(check_local_deps)

