TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

#
# Verify that objc_stubs pass works when using dynamic symbol lookup
#

all:
	${CC} ${CCFLAGS} main.m -undefined dynamic_lookup
	${PASS_IFF_GOOD_MACHO} a.out
	
clean:
	rm -rf a.out
