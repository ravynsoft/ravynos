TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

SHELL = bash # use bash shell so we can redirect just stderr


run: all

all:
	${CC} ${CCFLAGS} foo.c bar.c -Os -o foo -framework CoreFoundation
	${PASS_IFF_GOOD_MACHO} foo

clean:
	rm -rf foo 
