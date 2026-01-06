TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

#
# Test that coalesced symbols that are also dead strip roots
# are consistently dead stripped regardless of the processing order.
#
# In this test Foo::foo from main.cpp should be always preferred over the
# implementation in foo.cpp because of its favorable alignment. This means that
# Foo::foo and bar from foo.cpp can both be dead stripped.
# It should be the case regardless of the order of files on the command line.
#

run: all

all:
	${CXX} foo.cpp -o foo.o -c
	${CXX} main.cpp -o main.o -c
	# Use -export_dynamic to make the Foo::foo methods dead strip roots.
	${CXX} main.o foo.o -Wl,-dead_strip -Wl,-export_dynamic
	nm a.out | ${FAIL_IFF_SUCCESS} grep bar
	${CXX} foo.o main.o -Wl,-dead_strip -Wl,-export_dynamic
	nm a.out | ${FAIL_IFF_SUCCESS} grep bar

clean:
	rm -rf foo.o main.o a.out
