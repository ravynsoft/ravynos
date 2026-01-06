
TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

#
# Verify that aligned pointers at unaligned offsets are linked correctly.
#
# Atom `_arr` is placed at 0x1, and there's a pointer fixup at `_arr+7`. Uneven
# fixup offset makes it seems like the pointer won't be aligned, but it is
# because of the location of the atom.
# Regression test case for rdar://94118705.
#

ifeq (${ARCH},x86_64)
run: all
else ifeq (${ARCH},arm64)
run: all
else
run: all.unsupported
endif

all:
	${FAIL_IF_ERROR} ${CC} foo_align.s -e_foo -Wl,-fixup_chains -Wl,-data_const \
		2>&1 | ${FAIL_IF_STDIN}
	${FAIL_IF_ERROR} ${CC} foo.s -e_foo -Wl,-fixup_chains -Wl,-data_const \
		2>&1 | ${FAIL_IF_ERROR} grep "alignment (1) of atom '_arr' is too small"

	${PASS_IFF} true

all.unsupported:
	${PASS_IFF} true

clean:
	rm -rf a.out
