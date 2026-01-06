TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

#
# Test code dedup pass and ordering of dedupped functions.
#

ifeq (${ARCH},arm64)
run: all
else ifeq (${ARCH},x86_64)
run: all
else
run:
	${PASS_IFF} echo "code dedup supports only arm64 and x86_64"
endif

all:
	${CC} ${CCFLAGS} foo.cpp -c
	${CC} ${CCFLAGS} bar.cpp -c
	${CC} ${CCFLAGS} baz.cpp -c
	${LD} foo.o baz.o bar.o ${LDFLAGS} -lSystem
	# Verify all constructor were dedupped - they're at the same address
	nm a.out | grep "C1E" | awk '{print $$1}' | sort | uniq | wc -l | grep 1 | ${FAIL_IF_EMPTY}
	# Verify order of dedupped functions
	# Baz/Bar are aliases to Foo so they must precede it.
	# Order of aliases should follow order of files on the command line
	nm -p a.out | grep "C1E" | head -n1 | grep BazC1 | ${FAIL_IF_EMPTY}
	nm -p a.out | grep "C1E" | head -n2 | grep BarC1 | ${FAIL_IF_EMPTY}
	# Foo should be last, it's first on the command line so it's concrete implementation will be preserved
	nm -p a.out | grep "C1E" | tail -n1 | grep FooC1 | ${FAIL_IF_EMPTY}
	${PASS_IFF_GOOD_MACHO} a.out

clean:
	rm -rf a.out foo.o bar.o baz.o

