
TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

#
# Verify that pointers located at a pages boundary are diagnosed and not allowed
# when fixup chains are enabled.
#

run: all

ifeq (${ARCH},x86_64)
all:
	# On macOS we allow unaligned pointers and disable chained fixups when they're found.
	${FAIL_IF_ERROR} ${CC} page_overlap_align.s main.c -Wl,-fixup_chains -Wl,-data_const \
		2>&1 | ${FAIL_IF_ERROR} grep "disabling chained fixups because of unaligned pointers"
	# However, we can only do this consistently when atoms are aligned to at least
	# the pointer size. If they're not, we won't disable chained fixups, because
	# pointers may or may not end up aligned.
	${FAIL_IF_SUCCESS} ${CC} page_overlap.s main.c -Wl,-fixup_chains -Wl,-data_const \
		2>&1 | ${FAIL_IF_ERROR} grep "pointer not aligned at page boundary address"
	# Pointers at a page boundary are allowed for kexts, this is a problem only when
	# dyld loads the binary.
	${FAIL_IF_ERROR} ${CC} page_overlap.s main.c -r -o main.o
	${FAIL_IF_ERROR} ${LD} main.o -fixup_chains -data_const -kext

	${PASS_IFF} true

else ifeq (${ARCH},arm64)
all:
	# On all other platforms we expect all pointers to be aligned and error out if they aren't.
	${FAIL_IF_SUCCESS} ${CC} page_overlap_align.s main.c -Wl,-fixup_chains -Wl,-data_const \
		2>&1 | ${FAIL_IF_ERROR} grep "unaligned pointer(s) for architecture"
	${FAIL_IF_SUCCESS} ${CC} page_overlap.s main.c -Wl,-fixup_chains -Wl,-data_const \
		2>&1 | ${FAIL_IF_ERROR} grep "pointer not aligned at page boundary address"
	${FAIL_IF_ERROR} ${CC} page_overlap.s main.c -r -o main.o
	${FAIL_IF_ERROR} ${LD} main.o -fixup_chains -data_const -kext
	# Pointers at a page boundary should be still allowed when fixup chains are disabled.
	${FAIL_IF_ERROR} ${CC} page_overlap.s main.c -Wl,-no_fixup_chains 2> /dev/null

	${PASS_IFF} true

else
all:
	${PASS_IFF} true
endif

clean:
	rm -rf a.out main.o
