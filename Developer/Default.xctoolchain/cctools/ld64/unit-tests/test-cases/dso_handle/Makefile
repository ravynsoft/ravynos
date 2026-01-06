
TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

SHELL = bash # use bash shell so we can redirect just stderr

#
# Verify the linker errors out nicely when __dso_handle is defined in source.
#

run: all

all:
	${CC} ${CCFLAGS} test.c -o test
	${FAIL_IF_BAD_MACHO} test
	${CC} ${CCFLAGS} test.c -DDSO_DEF=1 -o test-def 2>warnings.txt
	${FAIL_IF_BAD_MACHO} test-def
	${CC} ${CCFLAGS} test.c -DDSO_TENT=1 -o test-tent -Wl,-w
	${PASS_IFF_GOOD_MACHO} test-tent


clean:
	rm -f test test-def test-tent warnings.txt
