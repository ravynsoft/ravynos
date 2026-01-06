
TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

#
# Test the we set the data execute bit for i386
#

run: run-${ARCH}

run-x86_64:
	${PASS_IFF} true
	
run-armv6: 
	${PASS_IFF} true

run-armv7: 
	${PASS_IFF} true

run-ppc:
	${PASS_IFF} true


run-i386:
	# Test with the flag
	${CC} ${CCFLAGS} main.c -o main-allow -Wl,-allow_heap_execute
	${FAIL_IF_BAD_MACHO} main-allow
	${OTOOL} -hv main-allow | grep MH_NO_HEAP_EXECUTION | ${FAIL_IF_STDIN}
	# Test without the flag
	${CC} ${CCFLAGS} main.c -o main
	${OTOOL} -hv main | grep MH_NO_HEAP_EXECUTION | ${FAIL_IF_EMPTY}
	${PASS_IFF_GOOD_MACHO} main

clean:
	rm -rf main main-allow
