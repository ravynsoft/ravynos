TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

#
# Regression test case for
# 	rdar://97955721 (ld64 incorrectly coalesces ThinLTO weak-def atoms with different attributes)
# ld64 was adding already coalesced LTO atoms to the symbol table.
# Depending on the name collision rules, we could then try to use a coalesced
# symbol over a concrete implementation.
# Prior to the fix the test case failed with:
# ```
# ld: reference to symbol (which has not been assigned an address) __ZN8foo_implIiEC1Ev in '__Z3foov'
# ```
#

run: all

all:
	# _bar and _foo use foo_impl's constructor. They're be compiled with
	# ThinLTO and the first constructor implementation needs to be coalesced.
	${CC} foo.cpp -c -flto=thin -fvisibility=hidden
	${CC} bar.cpp -c -flto=thin -fvisibility=hidden
	# _main also uses foo_impl, but it doesn't have hidden visibility.
	# Here the constructor will have auto-hide property, but name collision rules
	# prefer atoms without auto-hide. The bug in rdar://97955721 was that symbol
	# table choose the coalesced implementation.
	${CC} main.cpp -c
	# -flto-codegen-only is necessary for ThinLTO to emit multiple weak defs,
	#  without it coalescing will be done by libLTO.
	${CC} main.o foo.o bar.o  -Wl,-dead_strip -Wl,-flto-codegen-only
	${PASS_IFF} true

clean:
	rm -rf foo.o bar.o main.o a.out
