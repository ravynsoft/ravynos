
TESTROOT = ../..
include ${TESTROOT}/include/common.makefile


#
# Verify an Objective-C object file when run through ld -r is unaltered.
#
#

run: all

all:
	${CC} ${CCFLAGS} test.m -dynamiclib -framework Foundation -o libtest.dylib
	${PASS_IFF_GOOD_MACHO} libtest.dylib
	
clean:
	rm -rf libtest.dylib
