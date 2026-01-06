
TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

#
# Check linker options work for -needed-l
#

run: all

all:
	${CC} ${CCFLAGS} foo.c -dynamiclib -o libfoo.dylib
	${CC} ${CCFLAGS} main.c -c -o main.o
	${LD} -r main.o -add_linker_option -needed-lfoo -o main2.o
	${CC} ${CCFLAGS} main2.o -o main -L.
	${DYLDINFO} -dylibs main | grep libfoo
	${PASS_IFF_GOOD_MACHO} main

clean:
	rm -f main libfoo.dylib main.o main2.o
