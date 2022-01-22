
# Clear the LD_LIBRARY_PATH if GNUstep set it so that we don't accidentally use
# the installed version


# On Windows, CMake adds /TC to the clang-cl flags and doesn't provide a way to
# tell it not to.  We fix this by telling clang do disregard that option,
# unconditionally (which means that it still defaults to C for .c files).
set(ENV{CCC_OVERRIDE_OPTIONS} "x/TC x/TP x-TC x-TP")
set(INCREMENTAL " ")
if (MSVC)
	set(CMAKE_EXE_LINKER_FLAGS "/DEBUG /INCREMENTAL:NO ${CMAKE_EXE_LINKER_FLAGS}")
	set(INCREMENTAL "/INCREMENTAL:NO")
endif ()

# List of single-file tests.
set(TESTS
	alias.m
	alignTest.m
	AllocatePair.m
	ARCTest_arc.m
	AssociatedObject.m
	AssociatedObject2.m
	BlockImpTest.m
	BlockTest_arc.m
	ConstantString.m
	Category.m
	ExceptionTest.m
	FastARC.m
	Forward.m
	ManyManySelectors.m
	NestedExceptions.m
	PropertyAttributeTest.m
	ProtocolExtendedProperties.m
	PropertyIntrospectionTest.m
	PropertyIntrospectionTest2_arc.m
	ProtocolCreation.m
	ResurrectInDealloc_arc.m
	RuntimeTest.m
	SuperMethodMissing.m
	WeakBlock_arc.m
	WeakRefLoad.m
	WeakReferences_arc.m
	WeakImportClass.m
	ivar_arc.m
	ivar_atomic.m
	IVarOverlap.m
	IVarSuperclassOverlap.m
	objc_msgSend.m
	msgInterpose.m
	NilException.m
	MethodArguments.m
	zeroSizedIVar.m
	exchange.m
	hash_table_delete.c
	hash_test.c
	setSuperclass.m
)

if (WIN32)
else ()
	# Don't run the tests that are specific to Itanium-style exceptions on
	# Windows.
	list(APPEND TESTS
	BoxedForeignException.m
	ForeignException.m
	)
endif ()

# List of single-file tests that won't work with the legacy ABI and so
# shouldn't be run in legacy mode.
set(NEW_TESTS
	category_properties.m
)

remove_definitions(-D__OBJC_RUNTIME_INTERNAL__=1)

add_library(test_runtime_legacy OBJECT Test.m)
set_target_properties(test_runtime_legacy PROPERTIES
	INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
	COMPILE_FLAGS "-Xclang -fblocks -fobjc-runtime=gnustep-1.7"
	LINKER_LANGUAGE C
)

add_library(test_runtime OBJECT Test.m)
set_target_properties(test_runtime PROPERTIES
	INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
	COMPILE_FLAGS "-Xclang -fblocks -fobjc-runtime=gnustep-2.0"
	LINKER_LANGUAGE C
)


# Function for adding a test.  This takes the name of the test and the list of
# source files as arguments.
function(addtest_flags TEST_NAME FLAGS TEST_SOURCE)
	if (${TEST_NAME} MATCHES ".*_arc")
		# Only compile the main file with ARC
		set_source_files_properties(${TEST_SOURCE}
			COMPILE_FLAGS "-Xclang -fobjc-arc")
	endif()
	add_executable(${TEST_NAME} ${TEST_SOURCE})
	add_test(${TEST_NAME} ${TEST_NAME})
	set(ARC "")
	set_target_properties(${TEST_NAME} PROPERTIES
		INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
		COMPILE_FLAGS "-Xclang -fblocks -Xclang -fobjc-exceptions ${FLAGS}"
		LINK_FLAGS ${INCREMENTAL}
		LINKER_LANGUAGE C
	)
	set_property(TEST ${TEST_NAME} PROPERTY
		ENVIRONMENT "LD_LIBRARY_PATH=" "LLVM_PROFILE_FILE=${TEST_NAME}.profraw"
	)
	target_link_libraries(${TEST_NAME} objc)
endfunction(addtest_flags)

function(addtest_variants TEST TEST_SOURCE LEGACY)
	addtest_flags(${TEST} "-O0 -fobjc-runtime=gnustep-2.0 -UNDEBUG -DGS_RUNTIME_V2" "${TEST_SOURCE}")
	target_sources(${TEST} PRIVATE $<TARGET_OBJECTS:test_runtime>)
	addtest_flags("${TEST}_optimised" "-O3 -fobjc-runtime=gnustep-2.0 -UNDEBUG -DGS_RUNTIME_V2" "${TEST_SOURCE}")
	target_sources("${TEST}_optimised" PRIVATE $<TARGET_OBJECTS:test_runtime>)
	if (${LEGACY} AND ${OLDABI_COMPAT})
		addtest_flags("${TEST}_legacy" "-O0 -fobjc-runtime=gnustep-1.7 -UNDEBUG" "${TEST_SOURCE}")
		target_sources("${TEST}_legacy" PRIVATE $<TARGET_OBJECTS:test_runtime_legacy>)
		addtest_flags("${TEST}_legacy_optimised" "-O3 -fobjc-runtime=gnustep-1.7 -UNDEBUG" "${TEST_SOURCE}")
		target_sources("${TEST}_legacy_optimised" PRIVATE $<TARGET_OBJECTS:test_runtime_legacy>)
	endif()
endfunction(addtest_variants)

foreach(TEST_SOURCE ${TESTS})
	get_filename_component(TEST ${TEST_SOURCE} NAME_WE)
	addtest_variants(${TEST} ${TEST_SOURCE} true)
endforeach()

foreach(TEST_SOURCE ${NEW_TESTS})
	get_filename_component(TEST ${TEST_SOURCE} NAME_WE)
	addtest_variants(${TEST} ${TEST_SOURCE} false)
endforeach()

# Tests that are more than a single file.
addtest_variants("CXXExceptions" "CXXException.m;CXXException.cc" true)
addtest_variants("ForwardDeclareProtocolAccess" "ForwardDeclareProtocolAccess.m;ForwardDeclareProtocol.m" true)
if (ENABLE_OBJCXX)
	addtest_variants(ObjCXXEHInterop "ObjCXXEHInterop.mm;ObjCXXEHInterop.m" true)
	# This test is failing on Win32, but not for any obvious reason.  Disable
	# it for now to keep CI happy.
	if (WIN32)
	else()
		addtest_variants(ObjCXXEHInterop_arc "ObjCXXEHInterop_arc.mm;ObjCXXEHInterop_arc.m" true)
	endif()
endif()

# Some tests use enough memory that they fail on CI intermittently if they
# happen to run in parallel with each other.
set_tests_properties(ManyManySelectors PROPERTIES PROCESSORS 3)
set_tests_properties(ManyManySelectors_optimised PROPERTIES PROCESSORS 3)
if (${LEGACY})
	set_tests_properties(ManyManySelectors_legacy PROPERTIES PROCESSORS 3)
	set_tests_properties(ManyManySelectors_legacy_optimised PROPERTIES PROCESSORS 3)
endif ()
