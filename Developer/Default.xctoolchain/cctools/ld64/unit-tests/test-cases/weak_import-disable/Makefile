
TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

#
# Tests -no_weak_imports
#


run: all

all:
    # check weak_import works and triggers error with -no_weak_imports
	${CC} ${CCFLAGS} -dynamiclib foo.c -o libfoo.dylib 
	${FAIL_IF_BAD_MACHO} libfoo.dylib
	${CC} ${CCFLAGS} main.c libfoo.dylib -o main1a
	nm -m main1a | grep _foo | grep  weak | ${FAIL_IF_STDIN}
	${CC} ${CCFLAGS} main.c libfoo.dylib -o main1b -DMAKE_FOO_WEAK_IMPORT=1
	nm -m main1b | grep _foo | grep  weak | ${FAIL_IF_EMPTY}
	${CC} ${CCFLAGS} main.c libfoo.dylib -o main1c -DMAKE_FOO_WEAK_IMPORT=1 -Wl,-no_weak_imports 2>main1c.errors || true
	grep 'ld:' main1c.errors | grep _foo | ${FAIL_IF_EMPTY}
	# check $ld$sxx works with -no_weak_imports
	${CC} ${CCFLAGS} main2.c libfoo.dylib -o main2a -Wl,-no_weak_imports
	${CC} ${CCFLAGS} main2.c libfoo.dylib -o main2b -Wl,-no_weak_imports -mmacosx-version-min=10.7 2>main2b.errors || true
	grep _bar main2b.errors | ${FAIL_IF_EMPTY}
	${PASS_IFF_GOOD_MACHO} main1a
	
clean:
	rm -rf libfoo.dylib main1a main1b main1c main1c.errors main2a main2b main2b.errors
