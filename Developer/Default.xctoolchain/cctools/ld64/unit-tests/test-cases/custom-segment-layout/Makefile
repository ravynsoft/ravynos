
TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

#
# Check that a large page zero works
#
ifeq (,${findstring "macosx","$(VERSION_NEW_LINKEDIT)"})
	SEG_ADDR = 0x20000000
else
	SEG_ADDR = 0xb8000000
endif

run: all

all:
	${CC} ${CCFLAGS} main.c zero.s -o main -pagezero_size 0 -segaddr __TEXT $(SEG_ADDR) -segaddr __MYZEROPAGE 0
	${PASS_IFF_GOOD_MACHO} main

clean:
	rm -f main
