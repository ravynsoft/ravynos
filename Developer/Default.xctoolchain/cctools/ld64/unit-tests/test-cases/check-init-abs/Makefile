
TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

SHELL = bash # use bash shell so we can redirect just stderr

#
# Verify machochecker catchs init routines to abolute addresses outside image
#

run: all

all:
	${CC} ${CCFLAGS} main.c init.s -o init.exe
	${FAIL_IF_SUCCESS} ${MACHOCHECK} init.exe 2>/dev/null
	${CC} ${CCFLAGS} main.c term.s -o term.exe
	${FAIL_IF_SUCCESS} ${MACHOCHECK} term.exe 2>/dev/null
	${PASS_IFF} /usr/bin/true
	
clean:
	rm -f init.exe term.exe
