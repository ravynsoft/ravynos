TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

#
# Test building an umbrella framework with automatic dylib reexports
#

run: all

all:
	# build bootstrap dylib of with all code
	${CC} ${CCFLAGS} -dynamiclib a.c b.c c.c -o Big.stub  -install_name ${PWD}/Big.framework/Big
	# link each sub library against bootstrap to intra-Big symbols
	${CC} ${CCFLAGS} -dynamiclib a.c -o liba.dylib Big.stub -install_name ${PWD}/Big.framework/dylibs/liba.dylib -umbrella Big
	${CC} ${CCFLAGS} -dynamiclib b.c -o libb.dylib Big.stub -install_name ${PWD}/Big.framework/dylibs/libb.dylib -umbrella Big
	${CC} ${CCFLAGS} -dynamiclib c.c -o libc.dylib Big.stub -install_name ${PWD}/Big.framework/dylibs/libc.dylib -umbrella Big
	${CC} ${CCFLAGS} -dynamiclib liba.dylib libb.dylib libc.dylib -o Big -install_name ${PWD}/Big.framework/Big
	# re-link against correct dylibs now that everything is built
	${CC} ${CCFLAGS} -dynamiclib a.c -o liba.dylib libc.dylib -install_name ${PWD}/Big.framework/dylibs/liba.dylib -umbrella Big
	${DYLD_INFO} -fixups liba.dylib | grep c2 | grep libc | ${FAIL_IF_EMPTY}
	${CC} ${CCFLAGS} -dynamiclib b.c -o libb.dylib libc.dylib -install_name ${PWD}/Big.framework/dylibs/libb.dylib -umbrella Big
	${CC} ${CCFLAGS} -dynamiclib c.c -o libc.dylib liba.dylib -install_name ${PWD}/Big.framework/dylibs/libc.dylib -umbrella Big
	${DYLD_INFO} -fixups libc.dylib | grep a2 | grep liba | ${FAIL_IF_EMPTY}
	# dylibs listed on the command line should be automatically reexported as they were compiled with `-umbrella Big`
	mkdir -p Big.framework/dylibs
	${CC} ${CCFLAGS} -dynamiclib liba.dylib libb.dylib libc.dylib -o Big.framework/Big -install_name ${PWD}/Big.framework/Big
	mv *.dylib Big.framework/dylibs
	${CC} ${CCFLAGS} main.c -o main -F. -framework Big
	${PASS_IFF_GOOD_MACHO} main


clean:
	rm -rf Big.stub liba.dylib libb.dylib libc.dylib main Big.framework
