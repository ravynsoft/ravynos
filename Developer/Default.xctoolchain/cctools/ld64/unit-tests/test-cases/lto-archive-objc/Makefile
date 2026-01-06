
TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

#
# Check that -ObjC works with LTO
#

run: all

all:
	${CC} ${CCFLAGS} -flto foo.c -c -o foo.o
	${CC} ${CCFLAGS} -flto bar.m -c -o bar.o
	${CC} ${CCFLAGS} -flto baz.m -c -o baz.o
	libtool -static foo.o bar.o baz.o -o libstuff.a
	${CC} ${CCFLAGS} -flto main.c -c -o main.o
	${CC} ${CCFLAGS} main.o -o main libstuff.a -framework Foundation -ObjC 
	nm main | grep _Bar | ${FAIL_IF_EMPTY}
	nm main | grep MyCategory | grep doit | ${FAIL_IF_EMPTY}
	${PASS_IFF_GOOD_MACHO} main
	
clean:
	rm -rf main *.o libstuff.a main.o main
