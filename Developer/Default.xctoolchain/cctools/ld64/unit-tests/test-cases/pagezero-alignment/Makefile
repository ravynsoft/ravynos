
TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

run: all

# -pagezero_size is not supported on iOS, so test only x86_64
ifeq (${ARCH},x86_64)
all:
	# pagezero_size is rounded up when not aligned, sanity check that 0 pagezero
	# size is still supported and not rounded up
	${CC} ${CCFLAGS} main.c -o main -pagezero_size 0
	${OTOOL} -l main | ${FAIL_IF_SUCCESS} grep __PAGEZERO

	# pagezero size 4k, with 16k alignment, pagezero should be aligned to 16k
	${CC} ${CCFLAGS} main.c -o main -pagezero_size 0x1000 \
		-Xlinker -segalign -Xlinker 0x4000 2>&1 \
		| grep "pagezero_size not aligned, rounded up to: 0x4000" | ${FAIL_IF_EMPTY}
	${FAIL_IF_BAD_MACHO} main
	${SIZE} -mlx main | grep "PAGEZERO: 0x4000" | ${FAIL_IF_EMPTY}

	# pagezero size 4k, with 16k alignment, __TEXT at 8k
	# This should fail because __TEXT overlaps with aligned pagezero
	${FAIL_IF_SUCCESS} ${CC} ${CCFLAGS} main.c -o main -pagezero_size 0x1000 \
		-Xlinker -segalign -Xlinker 0x4000 -segaddr __TEXT 0x2000 2>/dev/null

	# pagezero size 4k, with 16k alignment, __TEXT at 16k
	# This should work because __TEXT doesn't overlap with aligned pagezero
	${FAIL_IF_ERROR} ${CC} ${CCFLAGS} main.c -o main -pagezero_size 0x1000 \
		-Xlinker -segalign -Xlinker 0x4000 -segaddr __TEXT 0x4000 2>&1 \
		| grep "pagezero_size not aligned, rounded up to: 0x4000" | ${FAIL_IF_EMPTY}

	# pagezero size 4k, with 4k alignment, pagezero size should be 4k
	${CC} ${CCFLAGS} main.c -o main -pagezero_size 0x1000 \
		-Xlinker -segalign -Xlinker 0x1000 2>&1 \
		| ${FAIL_IF_SUCCESS} grep "pagezero_size not aligned"
	${SIZE} -mlx main | grep "PAGEZERO: 0x1000" | ${FAIL_IF_EMPTY}

	${PASS_IFF_GOOD_MACHO} main
else
all:
	echo "unsupported arch"
	${PASS_IFF} true
endif

clean:
	rm -f main
