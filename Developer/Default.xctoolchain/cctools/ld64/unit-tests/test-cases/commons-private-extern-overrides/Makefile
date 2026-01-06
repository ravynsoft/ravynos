TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

#
# Test that common symbols are consistently overriden by concrete definitions
# regardless of dead stripping or their scope.
#

run: all

all:
	${CC} ${CCFLAGS} libfoo.c -o libfoo.o -c
	libtool libfoo.o -static -o libfoo.a
	${CC} ${CCFLAGS} main.c -o main.o -c
	${CC} main.o -L. -lfoo
	nm -m a.out | ${FAIL_IF_ERROR} grep "__data.*_bar"
	${CC} main.o -L. -lfoo -dead-strip
	nm -m a.out | ${FAIL_IF_ERROR} grep "__data.*_bar"

	${CC} ${CCFLAGS} main.c -o main.o -c -fvisibility=hidden
	${CC} main.o -L. -lfoo
	nm -m a.out | ${FAIL_IF_ERROR} grep "__data.*_bar"
	${CC} main.o -L. -lfoo -Wl,-dead_strip
	nm -m a.out | ${FAIL_IF_ERROR} grep "__data.*_bar"
	${PASS_IFF} true

clean:
	rm -rf a.out libfoo.a libfoo.o main.o a.out
