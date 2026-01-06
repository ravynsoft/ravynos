
TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

run: all

all:
	${CC} ${CCFLAGS} bar.c -shared -o libbar.dylib
	${CC} ${CCFLAGS} foo.c -c -o foo.o
	${CXX} ${CXXFLAGS} main.cpp -c -o main.o
	${FAIL_IF_SUCCESS} ${CXX} ${CXXFLAGS} foo.o main.o -o main -lbar -L. 2> stderr.log

	grep "(found _foo in foo.o, declaration possibly missing extern \"C\")" stderr.log \
		| ${FAIL_IF_EMPTY}

	grep "(found _bar in ./libbar.dylib, declaration possibly missing extern \"C\")" stderr.log \
		| ${FAIL_IF_EMPTY}

	${PASS_IFF_SUCCESS} true

clean:
	rm -f main foo.o main.o libbar.dylib stderr.log
