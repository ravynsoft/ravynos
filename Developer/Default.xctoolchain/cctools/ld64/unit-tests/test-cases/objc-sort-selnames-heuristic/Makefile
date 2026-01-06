
TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

#
# Test sorting heuristic of __objc_methname selector strings.
# The heuristic attempts to minimize the number of adjacent selectors whose
# length is a power of 2, because lower bits of selector strings are used by
# ObjC method cache.
# Given a set of input selectors:
#  aaa
#  aab
#  aac
#  aa
#  ab
#  ac
#  ad
# The selectors should be deterministcally sorted like this:
#  aaa
#  ab
#  aac
#  aa
#  aab
#  ac
#  ad
#

run: all

all:
	${CC} ${CCFLAGS} main.m -framework Foundation
	# selrefs should be sorted alphabetically
	${OBJDUMP} --section __objc_selrefs --macho a.out | grep "aa" \
		| head -n1 | grep aa | ${FAIL_IF_EMPTY}
	${OBJDUMP} --section __objc_selrefs --macho a.out | grep "aa" \
		| tail -n1 | grep aac | ${FAIL_IF_EMPTY}
	${OBJDUMP} --section __objc_methname --macho a.out | grep " a" \
		| head -n1 | grep aaa | ${FAIL_IF_EMPTY}
	${OBJDUMP} --section __objc_methname --macho a.out | grep " a" \
		| head -n2 | grep ab | ${FAIL_IF_EMPTY}
	${PASS_IFF} true

clean:
	rm -rf a.out
