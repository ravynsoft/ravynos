TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

#
# Test that `__swift_FORCE_LOAD*` symbols are linked correctly against a target
# library when it and other symbols are redirected to different libraries.
# Regression test case for rdar://99457165.
#

all:
	${CC} -dynamiclib lib.c -o liblib.dylib
	${FAIL_IFF_SUCCESS} ${CC} client.c 2>&1 | grep "Undefined symbols" | ${FAIL_IF_EMPTY}
	${FAIL_IFF_ERROR} ${CC} client.c -llib -L.
	${FAIL_IF_BAD_MACHO} a.out
	# swift_FORCE_LOAD should be in the symbol table
	nm -nm a.out | grep "swift_FORCE_LOAD" | ${FAIL_IF_EMPTY}
	# but there shouldn't be any binds to it as of rdar://84530285
	${DYLD_INFO} -fixups a.out | ${FAIL_IF_SUCCESS} grep "swift_FORCE_LOAD"
	# executable should link against redirected `we_redirected_the_force_load` library
	${OTOOL} -L a.out | ${FAIL_IF_ERROR} grep we_redirected_the_force_load 
	# executable should link against redirected `we_redirected_some_other_global_variable` library
	${OTOOL} -L a.out | ${FAIL_IF_ERROR} grep we_redirected_some_other_global_variable 
	${PASS_IFF} true

clean:
	rm -rf a.out liblib.dylib
