
TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

#
# Verify that __objc_selrefs and __objc_methname are sorted by selector names.
# rdar://100542900 (Missing sorting of selector strings)
#

run: all

all:
	${CC} ${CCFLAGS} main.m -framework Foundation
	${OBJDUMP} --section __objc_selrefs --macho a.out | egrep "(bar|foo)" \
		| head -n1 | grep bar | ${FAIL_IF_EMPTY}
	${OBJDUMP} --section __objc_methname --macho a.out | egrep "(bar|foo)" \
		| head -n1 | grep bar | ${FAIL_IF_EMPTY}
	${PASS_IFF} true

clean:
	rm -rf a.out
