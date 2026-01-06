
TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

#
# Test that fixup offsets in the -fixup_chains_section_vm option don't include
# a custom pagezero size.
# rdar://98188451 (Target offsets for -fixup_chains_section_vm seem incorrect when __PAGEZERO exists)
#

all:
	${CC} ${CCFLAGS} test.c -static -o test -nostdlib \
		-static -Wl,-e,_main,-pagezero_size,0x400000000,-pie,-fixup_chains_section_vm
	${DYLD_INFO} -fixup_chain_details test | grep "0x004" | ${FAIL_IF_STDIN} 
	${PASS_IFF} true

clean:
	rm -rf test
