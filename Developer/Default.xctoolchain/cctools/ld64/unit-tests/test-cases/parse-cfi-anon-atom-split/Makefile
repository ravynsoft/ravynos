TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

#
# Test parsing anonymous atoms split by CFI start addresses.
#
# This is a regression test case for
# rdar://97545344 (SEED: ld64 doesn't always correctly split atoms at CFI start addresses).
# A bug introduced in rdar://93130909 (Tweak parsing and stabs generation of empty atoms)
# caused MachO parser not to split atoms at some CFI start addresses. The
# way GCC generates code triggered an inconsistency in parsing and an assertion.
#

ifeq ($(ARCH),i386)
all:
	echo "PASS"

else
all:
	${CC} ${CCFLAGS} test.s -c -o test.o
	${FAIL_IF_ERROR} ${CC} ${CCFLAGS} test.o -r -o merged.o -g
	${FAIL_IF_ERROR} ${CXX} ${CCFLAGS} merged.o -o a.out -g -e foo

	${PASS_IFF_GOOD_MACHO} a.out
endif

clean:
	rm -rf test.o merged.o a.out
