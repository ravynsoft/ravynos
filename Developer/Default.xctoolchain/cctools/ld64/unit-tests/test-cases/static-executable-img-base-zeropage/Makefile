
TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

#
# Test that __PAGEZERO starts at 0x0 vm address when linking static executable
# with a custom base address.
# rdar://98186354 (Using -image_base with static execs will move __PAGEZERO)
#

all:
	${CC} ${CCFLAGS} test.c -static -o test -nostdlib \
		-Wl,-e,_main,-pagezero_size,0x100000000,-no_pie,-image_base,0x80000000000
	${SIZE} -mlx test | grep __PAGEZERO | grep "vmaddr 0x0" | ${FAIL_IF_EMPTY}
	${PASS_IFF} true

clean:
	rm -rf test
