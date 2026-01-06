TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

#
# Test that atoms in different sections are correctly split by CFI start addresses.
#

ifeq ($(ARCH),arm64)
all:
	${CC} ${CCFLAGS} test.s -c -o test.o
	${FAIL_IF_ERROR} ${CC} ${CCFLAGS} test.o -r -o merged.o -g
	${FAIL_IF_ERROR} ${CXX} ${CCFLAGS} merged.o -o a.out -g -e foo
	# There should be two atoms in `__text_alt` - `__func_terminate` and
	# an anonymous atom for `L__func_terminate_two` because of a CFI start address.
	# A bug introduced in rdar://93130909 (Tweak parsing and stabs generation of empty atoms)
	# caused some CFI splits to be ignored, and resulted in these two atoms
	# being parsed as a single large atom.
	${OBJECTDUMP} merged.o | grep "__TEXT,__text_alt" | wc -l | grep 2 | ${FAIL_IF_EMPTY}

	${PASS_IFF_GOOD_MACHO} a.out

else
all:
	echo "PASS"
endif

clean:
	rm -rf test.o merged.o a.out
