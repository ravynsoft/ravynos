TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

#
# Test that a dynamically referenced symbol is always exported
#

run: all

all:
	${CC} foo.c -dynamiclib -o libfoo.dylib
	nm -m libfoo.dylib | grep _keep_global | grep "referenced dynamically" | ${FAIL_IF_EMPTY}
	nm -m libfoo.dylib | grep _keep_hidden | grep "referenced dynamically" | ${FAIL_IF_EMPTY}
	${PASS_IFF_GOOD_MACHO} libfoo.dylib

clean:
	rm  libfoo.dylib
