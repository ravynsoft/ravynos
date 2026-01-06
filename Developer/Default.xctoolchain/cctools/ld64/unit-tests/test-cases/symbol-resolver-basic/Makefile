
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
	${CC} ${CCFLAGS} foo.c -c -o foo.o
	${CC} ${CCFLAGS} foo.o -dynamiclib -o libfoo.dylib 
	${DYLDINFO} -export libfoo.dylib | grep _foo | grep resolver | ${FAIL_IF_EMPTY}
	${LD} -arch ${ARCH} -r foo.o -o foo-r.o
	${CC} ${CCFLAGS} foo-r.o -dynamiclib -o libfoo-r.dylib 
	${DYLDINFO} -export libfoo-r.dylib | grep _foo | grep resolver | ${FAIL_IF_EMPTY}
	${PASS_IFF_GOOD_MACHO} libfoo-r.dylib

all-ppc:
	echo "PASS"

clean:
	rm  -f foo.o  libfoo.dylib foo-r.o libfoo-r.dylib
