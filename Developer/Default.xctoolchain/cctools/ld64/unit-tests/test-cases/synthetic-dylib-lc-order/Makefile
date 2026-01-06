
TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

#
# Regression test case for:
# 	rdar://100606133 (Order of load commands for synthetic dylibs from magic $ld$ symbols is indeterministic)
# where order of load commands for synthetic dylibs from magic `$ld$...` symbols
# was indeterministic, because we were comparing their install paths pointer values
# rather than the string contents.
#


run: all

all:
	# Two separate libraries, they will be parsed concurrently, so the allocation
	# order for synthetic dylib names will be indeterministic
	${CC} ${CCFLAGS} bar_moved.c -dynamiclib -o libbar_moved.dylib
	${CC} ${CCFLAGS} baz_moved.c -dynamiclib -o libbaz_moved.dylib
	${CC} ${CCFLAGS} main.c -L. -lbaz_moved -lbar_moved
	# Regardless of the indeterministic allocation of synthetic dylib names their
	# order in load commands should be alphabetical
	${OTOOL} -L a.out | grep libbar.dylib | ${FAIL_IF_EMPTY}
	${OTOOL} -L a.out | tail -n1 | grep libbaz.dylib | ${FAIL_IF_EMPTY}
	${PASS_IFF} true

clean:
	rm -rf a.out libbar_moved.dylib libbaz_moved.dylib
