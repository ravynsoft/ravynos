

TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

#
# Check -function_starts
#

run: all

all:
	# as main executable
	${CC} ${CCFLAGS} main.c -o main -Wl,-function_starts
	${DYLDINFO} -function_starts main | grep _bar | ${FAIL_IF_EMPTY}
	# as dylib
	${CC} ${CCFLAGS} main.c -dynamiclib -o libmain.dylib -Wl,-function_starts
	${DYLDINFO} -function_starts libmain.dylib | grep _bar | ${FAIL_IF_EMPTY}
	# as dylib with prefered load address
	${CC} ${CCFLAGS} main.c -dynamiclib -o libmain2.dylib -seg1addr 0x200000 -Wl,-function_starts
	${DYLDINFO} -function_starts libmain2.dylib | grep _bar | ${FAIL_IF_EMPTY}
	# as dylib with aliases
	${CC} ${CCFLAGS} main.c -dynamiclib -o libmain3.dylib -Wl,-function_starts -Wl,-alias,_mid,midalias
	${DYLDINFO} -function_starts libmain3.dylib | grep _bar | ${FAIL_IF_EMPTY}
	# as static main executable
	${CC} ${CCFLAGS} main.c -static -e _main -o main_static -nostdlib -Wl,-new_linker -Wl,-alias,_myexit,_exit
	${DYLDINFO} -function_starts main_static | grep _bar | ${FAIL_IF_STDIN}
	${CC} ${CCFLAGS} main.c -static -e _main -o main_static -nostdlib -Wl,-new_linker -Wl,-alias,_myexit,_exit -Wl,-function_starts
	${DYLDINFO} -function_starts main_static | grep _bar | ${FAIL_IF_EMPTY}


	${PASS_IFF_GOOD_MACHO} main
	

clean:
	rm  main libmain.dylib libmain2.dylib libmain3.dylib main_static
