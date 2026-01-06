TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

#
# <rdar://96314882>
# Verify that objc class patching optimization handles indirectly bound
# class aliases (defined in separate object files).
#

run: all

ifeq (${ARCH},i386)
all:
	${PASS_IFF} echo "unsupported"

else
all:
	${CC} foo.m -c -o foo.o
	${CC} foo_alias.c -c -o foo_alias.o
	${CC} foo.o foo_alias.o -framework Foundation -shared -o libfoo.dylib \
		-Wl,-add_split_seg_info \
		-Wl,-platform_version,${PLATFORM},${VERSION_2022FALL},${SDK_VERSION} 2> /dev/null
	# _globalFoo should be indirectly bound to Foo class symbol
	${DYLD_INFO} -fixups libfoo.dylib | grep "__DATA.*__marker.*_OBJC_CLASS_\$$_Foo" \
		| ${FAIL_IF_EMPTY}
	${CC} foo.o foo_alias.o -framework Foundation -shared -o libfoo.dylib \
		-Wl,-add_split_seg_info -Wl,-exported_symbols_list,exported_symbols \
		-Wl,-platform_version,${PLATFORM},${VERSION_2022FALL},${SDK_VERSION} 2> /dev/null
	# _globalFoo should be indirectly bound to ConstantFooReference alias, because the
	# class symbol isn't exported
	${DYLD_INFO} -fixups libfoo.dylib | grep "__DATA.*__marker.*_ConstantFooReference" \
		| ${FAIL_IF_EMPTY}
	${PASS_IFF_GOOD_MACHO} libfoo.dylib
endif

clean:
	rm -rf foo.o foo_alias.o libfoo.dylib
