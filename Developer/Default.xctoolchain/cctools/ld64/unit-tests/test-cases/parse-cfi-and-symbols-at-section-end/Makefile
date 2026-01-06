TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

#
# Validate relinking an object file produced using `-r` works when input
# contains CFI information and an empty symbol at the end of a section.
#
# This is a regression test case for rdar://93130909.
# The problem was that linking using `-r` with a specific order of object files
# produced an object file with an atom at the end of the __text section.
# Because input object file where the symbol was located (here constant.o)
# had debug information ld added BNSYM and other stab symbols to the linked object.
# Later when creating a final image linking failed, because when parsing atoms
# from symbols and CFI we'd skip empty symbols at the end of sections.
# This lead to a crash because we had a BNSYM, ENSYM stabs, but no
# matching atom.
#

ifeq ($(ARCH),i386)
all:
	echo "PASS"

else
all:
	${CC} ${CCFLAGS} test.s -c -o test.o
	${CC} ${CCFLAGS} constant.s -c -o constant.o
	# The order of object files matters, the bug only triggered when `dummy`
	# from constant.o object file was at the end of the section.
	# Use a custom alignment for `__gcc_except_tab` to make sure there's a gap
	#		after __text. Otherwise there was no crash because `findAtomByAddress`
	#		when resolving stabs would return first atom from __gcc_except_tab.
	${CC} ${CCFLAGS} test.o constant.o -r -o merged.o -g -Wl,-sectalign,__TEXT,__gcc_except_tab,0x100 
	${CC} ${CCFLAGS} merged.o -r -o a.out
	nm a.out | grep dummy | ${FAIL_IF_EMPTY}

	# Test also a case when __gcc_except_tab startAdd == __text endAddr.
	${CC} ${CCFLAGS} test.o constant.o -r -o merged.o -g
	${CC} ${CCFLAGS} merged.o -r -o a.out
	nm a.out | grep dummy | ${FAIL_IF_EMPTY}

	${PASS_IFF} true
endif

clean:
	rm -rf test.o constant.o merged.o a.out
