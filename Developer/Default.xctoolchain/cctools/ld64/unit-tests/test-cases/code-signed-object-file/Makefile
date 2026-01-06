
TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

#
# Test that an object file can be code signed
#
# <rdar://problem/5881324> when linking a kext the static linker should leave a pad in the headers to allow code signing
#

CODE_SIGN_ARCH = ${ARCH}
ifeq (${ARCH},ppc)
	CODE_SIGN_ARCH = ppc7400
endif


run: all

all:
	${CC} foo.c -c -o foo.o
	${LD} -r foo.o -o foo2.o
	codesign_allocate -i foo2.o -a ${CODE_SIGN_ARCH} 256 -o foo3.o
	${PASS_IFF} true 
	
clean:
	rm foo.o foo2.o foo3.o
