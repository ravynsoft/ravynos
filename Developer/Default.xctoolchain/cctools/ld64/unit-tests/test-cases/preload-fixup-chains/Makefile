
TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

#
# verify -fixup_chains_section works with -preload
#

run: all

all:
	${CC} ${CCFLAGS} main.c -c -o main.o -ffreestanding -nostdlib
	${LD} main.o -preload -fixup_chains_section -e _main -o main
	${DYLD_INFO} -fixups main | grep "__DATA.*__data.*rebase" | ${FAIL_IF_EMPTY}
	${PASS_IFF} true

clean:
	rm main main.o
