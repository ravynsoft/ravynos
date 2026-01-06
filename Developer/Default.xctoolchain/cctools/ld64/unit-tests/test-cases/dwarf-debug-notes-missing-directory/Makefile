
TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

#
# Check that ld doesn't crash when generating debug notes while either
#   filename or directory name is missing.
# Regression test case for rdar://93513409.
#

run: all

all:
	${CC} -emit-llvm -S foo.c -o foo.ll -g
	# Replace DI directory with an empty string
	# ... = !DIFile(filename: "...", directory: "...")
	sed -i .bak 's/directory:.*"/directory: ""/' foo.ll
	${CC} -r foo.ll -o foo.o

	${CC} -emit-llvm -S foo.c -o foo.ll -g
	# Replace DI filename with an empty string
	# ... = !DIFile(filename: "...", directory: "...")
	sed -i .bak 's/filename:.*",/filename: "",/' foo.ll
	${CC} -r foo.ll -o foo.o

	${CC} -emit-llvm -S foo.c -o foo.ll -g
	# Replace both with an empty strings
	# ... = !DIFile(filename: "...", directory: "...")
	sed -i .bak 's/filename:.*",/filename: "",/' foo.ll
	sed -i .bak 's/directory:.*"/directory: ""/' foo.ll
	${CC} -r foo.ll -o foo.o

	${PASS_IFF} true

clean:
	rm -f foo.ll foo.ll.bak foo.o
