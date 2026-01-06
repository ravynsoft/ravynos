

TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

#
# Support mulitple labels on the same literal by cloning the literal
# into an atom per label.
#

run: all

all:
	${CC} ${ASMFLAGS} literals.s -c -o literals.o
	${OBJECTDUMP} literals.o > literals.o.dump
	${LD} -arch ${ARCH} -r literals.o -o literals-r.o
	${OBJECTDUMP} literals-r.o > literals-r.o.dump
	${PASS_IFF} diff literals.o.dump literals-r.o.dump
	
clean:
	rm -rf literals.o literals-r.o literals.o.dump  literals-r.o.dump
