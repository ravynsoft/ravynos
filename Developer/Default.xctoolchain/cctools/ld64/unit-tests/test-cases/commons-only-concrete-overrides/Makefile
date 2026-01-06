TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

#
# Test that only concrete definitions can override tentative symbols and trigger
# loading of object files.
# This is to ensure consistency, otherwise if multiple static libraries would
# export tentative symbols not all of them would be necessarily loaded.
#

run: all

all:
	${CC} ${CCFLAGS} libfoo.c -o libfoo.o -c
	libtool libfoo.o -static -o libfoo.a
	${CC} ${CCFLAGS} libfoo_tent.c -o libfoo_tent.o -c
	# Use -c to place tentative definitions in the archive's index
	libtool -c libfoo_tent.o -static -o libfoo_tent.a
	${CC} ${CCFLAGS} main.c -o main.o -c
	${CC} main.o -L. -lfoo_tent -lfoo
	# Even though -lfoo_tent is earlier on the command line and it exports both
	# _foo and _bar it won't be loaded, because it exports only a tentative defintion.
	# -lfoo on the other hand will be loaded, because it has a concrete definition of _bar.
	nm -m a.out | ${FAIL_IF_ERROR} grep "__data.*_bar"
	nm -m a.out | ${FAIL_IF_SUCCESS} grep "tent_marker"
	# Verify that commons from foo_tent are still used when they're not defined at all
	# in main.
	${CC} ${CCFLAGS} main_extern.c -o main.o -c
	${CC} main.o -L. -lfoo_tent
	${PASS_IFF} true

clean:
	rm -rf libfoo_tent.o libfoo_tent.a libfoo.a libfoo.o main.o a.out
