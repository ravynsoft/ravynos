
TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

#
# Test that when using -all_load, all objects from different CLI options are included
#

run: all

all:
	${CC} ${CCFLAGS} foo.c -c -o foo.o
	libtool -static foo.o -o libfoo.a
	${CC} ${CCFLAGS} bar.c -c -o bar.o
	libtool -static bar.o -o bar.lib
	${CC} ${CCFLAGS} baz.c -c -o baz.o
	libtool -static baz.o -o baz.lib
	${CC} ${CCFLAGS} main.c -c -o main.o

	${LD} ${LDFLAGS} -all_load main.o -L. -lfoo bar.lib -filelist filelist -lSystem
	nm a.out | grep _foo | ${FAIL_IF_EMPTY}
	nm a.out | grep _bar | ${FAIL_IF_EMPTY}
	nm a.out | grep _baz | ${FAIL_IF_EMPTY}
	nm a.out | grep _main | ${FAIL_IF_EMPTY}
	${PASS_IFF} true

clean:
	rm -rf foo.o libfoo.a bar.o bar.lib baz.o baz.lib main.o a.out
