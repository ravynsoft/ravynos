
TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

#
# Test a simple symbol resolver function
#
TARGET = all
ifeq (${ARCH},ppc)
	TARGET = all-ppc
endif


run: ${TARGET}

all:
	${CC} ${CCFLAGS} foo.c -dynamiclib -o libfoo.dylib -Wl,-w
	${DYLDINFO} -lazy_bind libfoo.dylib | grep _a | ${FAIL_IF_STDIN}
	${PASS_IFF_GOOD_MACHO} libfoo.dylib

all-ppc:
	echo "PASS"

clean:
	rm  -f  libfoo.dylib 
