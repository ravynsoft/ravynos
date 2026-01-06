
TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

run: all

# Test that `-segaddr . 0x0` works when output doesn't use zero page

all:
	${CC} ${CCFLAGS} main.c -o main -segaddr __TEXT 0x0 -shared
	${SIZE} -mlx main | ${FAIL_IF_ERROR} grep "__TEXT.*vmaddr 0x0"

	${FAIL_IF_BAD_MACHO} main

	# Verify this also works when not adding zero page explicitly
	${CC} ${CCFLAGS} main.c -o main -segaddr __TEXT 0x0 -pagezero_size 0x0 -shared
	${SIZE} -mlx main | ${FAIL_IF_ERROR} grep "__TEXT.*vmaddr 0x0"

	${PASS_IFF_GOOD_MACHO} main

clean:
	rm -f main
