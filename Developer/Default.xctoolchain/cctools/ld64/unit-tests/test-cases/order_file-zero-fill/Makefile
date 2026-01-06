
TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

#
# Check that -order_file will transform zero-fill (tentative def)
# symbols to __data symbols to achieve ordering.
#

run: all

all:
	${CC} ${CCFLAGS} main.c foo.c -Wl,-order_file,main.order -o main
	nm -nj main | grep _xyz_ > main.actual
	diff main.expected main.actual
	nm -nm main | grep _xyz_f4 | grep __data | ${FAIL_IF_EMPTY}
	nm -nm main | grep _other | grep __common | ${FAIL_IF_EMPTY}
	${PASS_IFF_GOOD_MACHO} main

clean:
	rm -f main  main.actual
