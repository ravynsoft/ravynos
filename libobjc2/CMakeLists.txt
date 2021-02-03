cmake_minimum_required(VERSION 3.1)

project(libobjc C ASM CXX)

INCLUDE (CheckCXXSourceCompiles)

set(CMAKE_C_FLAGS_DEBUG "-O0 -Xclang -fno-inline ${CMAKE_C_FLAGS_DEBUG}")
set(CMAKE_C_FLAGS_RELEASE "-O3 ${CMAKE_C_FLAGS_RELEASE}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

set(libobjc_VERSION 4.6)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Xclang -fexceptions -Xclang -fobjc-exceptions")
if (MSVC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /EHas")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHas")
	set(CMAKE_C_FLAGS_DEBUG "/Z7 ${CMAKE_C_FLAGS_DEBUG}")
	set(CMAKE_SHARED_LINKER_FLAGS "/DEBUG /INCREMENTAL:NO ${CMAKE_SHARED_LINKER_FLAGS}")
	set(CMAKE_EXE_LINKER_FLAGS "/DEBUG /INCREMENTAL:NO ${CMAKE_EXE_LINKER_FLAGS}")
	set(objc_LINK_FLAGS "/DEBUG /INCREMENTAL:NO ${objc_LINK_FLAGS}")
endif()
# Build configuration
add_definitions( -DGNUSTEP -D__OBJC_RUNTIME_INTERNAL__=1)

set(CMAKE_CXX_STANDARD 14)

set(libobjc_ASM_SRCS 
	block_trampolines.S
	objc_msgSend.S)
set(libobjc_OBJCXX_SRCS 
	arc.mm
	)
set(libobjc_OBJC_SRCS 
	NSBlocks.m
	Protocol2.m
	associate.m
	blocks_runtime.m
	properties.m)
set(libobjc_C_SRCS 
	alias_table.c
	block_to_imp.c
	caps.c
	category_loader.c
	class_table.c
	dtable.c
	encoding2.c
	hooks.c
	ivar.c
	loader.c
	mutation.m
	protocol.c
	runtime.c
	sarray2.c
	selector_table.c
	sendmsg2.c
	)
set(libobjc_HDRS
	objc/Availability.h
	objc/Object.h
	objc/Protocol.h
	objc/blocks_private.h
	objc/blocks_runtime.h
	objc/capabilities.h
	objc/developer.h
	objc/encoding.h
	objc/hooks.h
	objc/message.h
	objc/objc-api.h
	objc/objc-arc.h
	objc/objc-auto.h
	objc/objc-class.h
	objc/objc-runtime.h
	objc/objc-visibility.h
	objc/objc.h
	objc/runtime-deprecated.h
	objc/runtime.h
	objc/slot.h)
set(libBlocksRuntime_COMPATIBILITY_HDRS
	Block.h
	Block_private.h
	)
# Windows does not use DWARF EH
if (WIN32)
	list(APPEND libobjc_CXX_SRCS eh_win32_msvc.cc)
else ()
	list(APPEND libobjc_C_SRCS eh_personality.c)
endif (WIN32)

if (NOT EXISTS "${CMAKE_SOURCE_DIR}/third_party/robin-map/include/tsl/robin_map.h")
	message(FATAL_ERROR "Git submodules not present, please run:\n\n"
						" $ git submodule init && git submodule update\n\n"
						"If you did not checkout via git, you will need to"
						"fetch the submodule's contents from"
						"https://github.com/Tessil/robin-map/")
endif ()

# For release builds, we disable spamming the terminal with warnings about
# selector type mismatches
if (CMAKE_BUILD_TYPE STREQUAL Release)
	add_definitions(-DNO_SELECTOR_MISMATCH_WARNINGS)
else ()
	add_definitions(-DGC_DEBUG)
endif ()

set(TYPE_DEPENDENT_DISPATCH TRUE CACHE BOOL
	"Enable type-dependent dispatch")
if (TYPE_DEPENDENT_DISPATCH)
	add_definitions(-DTYPE_DEPENDENT_DISPATCH)
endif ()
set(ENABLE_TRACING FALSE CACHE BOOL
	"Enable tracing support (slower, not recommended for deployment)")
if (ENABLE_TRACING)
	add_definitions(-DWITH_TRACING=1)
endif (ENABLE_TRACING)

set(BOEHM_GC FALSE CACHE BOOL
	"Enable garbage collection support (not recommended)")
if (BOEHM_GC)
	include(FindPkgConfig)
	pkg_check_modules(GC REQUIRED bdw-gc)
	link_directories(${GC_LIBRARY_DIRS})
	# If there's a threaded version, use it
	find_library(LIBGC gc-threaded PATHS "${GC_LIBRARY_DIRS}")
	if (LIBGC)
	else ()
		find_library(LIBGC gc PATHS GC_LIBRARY_DIRS)
	endif ()
	message(STATUS "Using Boehm GC library: ${LIBGC}")
	include_directories(GC_INCLUDE_DIRS)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GC_CFLAGS}")
	set(CMAKE_OBJC_FLAGS "${CMAKE_OBJC_FLAGS} -fobjc-gc")
	set(objc_LINK_FLAGS "${objc_LINK_FLAGS} ${GC_CFLAGS}")
	add_definitions(-DENABLE_GC)
	list(APPEND libobjc_OBJC_SRCS gc_boehm.c)
else ()
	list(APPEND libobjc_OBJC_SRCS gc_none.c)
endif ()

if (WIN32)
	set(OLD_ABI_COMPAT_DEFAULT false)
else()
	set(OLD_ABI_COMPAT_DEFAULT true)
endif()

set(OLDABI_COMPAT ${OLD_ABI_COMPAT_DEFAULT} CACHE BOOL
	"Enable compatibility with GCC and old GNUstep ABIs")

set(LEGACY_COMPAT FALSE CACHE BOOL
	"Enable legacy compatibility features")

set(DEBUG_ARC_COMPAT FALSE CACHE BOOL
	"Log warnings for classes that don't hit ARC fast paths")

if (DEBUG_ARC_COMPAT)
	add_definitions(-DDEBUG_ARC_COMPAT)
endif()

if (OLDABI_COMPAT)
	list(APPEND libobjc_C_SRCS legacy.c abi_version.c statics_loader.c)
	add_definitions(-DOLDABI_COMPAT=1)
endif()

if (LEGACY_COMPAT)
	list(APPEND libobjc_C_SRCS legacy_malloc.c)
else ()
	add_definitions(-DNO_LEGACY)
endif ()

set(LIBOBJC_NAME "objc" CACHE STRING 
	"Name of the Objective-C runtime library (e.g. objc2 for libobjc2)")

set(INCLUDE_DIRECTORY "objc" CACHE STRING 
	"Subdirectory of the include path to install the headers.")


if (${CMAKE_C_COMPILER_ID} MATCHES Clang*)
	set(CMAKE_OBJC_FLAGS "${CMAKE_OBJC_FLAGS} -Wno-deprecated-objc-isa-usage -Wno-objc-root-class")
	if (${CMAKE_C_COMPILER_VERSION} VERSION_GREATER 3.1)
		set(CMAKE_OBJC_FLAGS "${CMAKE_OBJC_FLAGS} -fobjc-runtime=gnustep-2.0")
	endif ()
else (${CMAKE_C_COMPILER_ID} MATCHES Clang*)
	MESSAGE("WARNING: It is strongly recommended that you compile with clang")
endif (${CMAKE_C_COMPILER_ID} MATCHES Clang*)

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i686")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=i586")
endif ()

set(INSTALL_TARGETS objc)

# On Windows, CMake adds /TC to the clang-cl flags and doesn't provide a way to
# tell it not to.  We fix this by telling clang do disregard that option,
# unconditionally (which means that it still defaults to C for .c files).
set(ENV{CCC_OVERRIDE_OPTIONS} "x/TC x/Gm-")

set_source_files_properties(
	${libobjc_ASM_SRCS}
	LANGUAGE C
	COMPILE_FLAGS "${CMAKE_OBJC_FLAGS} -Xclang -x -Xclang assembler-with-cpp"
)

set_source_files_properties(
	${libobjc_CXX_SRCS}
	LANGUAGE CXX
	COMPILE_FLAGS "${CMAKE_CXX_FLAGS}"
)

set_source_files_properties(
	${libobjc_OBJC_SRCS}
	COMPILE_FLAGS "${CMAKE_OBJC_FLAGS} -Xclang -x -Xclang objective-c"
)

set_source_files_properties(
	${libobjc_OBJCXX_SRCS}
	COMPILE_FLAGS "${CMAKE_OBJC_FLAGS} -Xclang -x -Xclang objective-c++"
)


#
# C++ Runtime interaction
#


function(test_cxx CXX_RUNTIME_NAME IS_STDLIB)
	set(CXX_RUNTIME_NAME_FULL "${CMAKE_SHARED_LIBRARY_PREFIX}${CXX_RUNTIME_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
	find_library(CXX_RUNTIME_LIB NAMES ${CXX_RUNTIME_NAME_FULL} ${CXX_RUNTIME_NAME})
	if (CXX_RUNTIME_LIB)
		message(STATUS "Testing ${CXX_RUNTIME_LIB} as the C++ runtime library")
		if (ANDROID)
			# pass on Android toolchain flags
			set(CXX_RUNTIME_FLAGS "-DANDROID_NDK=${ANDROID_NDK}" "-DANDROID_ABI=${ANDROID_ABI}" "-DANDROID_ARM_MODE=${ANDROID_ARM_MODE}" "-DANDROID_ARM_NEON=${ANDROID_ARM_NEON}" "-DANDROID_LD=${ANDROID_LD}" "-DANDROID_PLATFORM=${ANDROID_PLATFORM}" "-DANDROID_STL=${ANDROID_STL}")
		endif()
		try_compile(USERUNTIME 
			"${CMAKE_BINARY_DIR}/CMake"
			"${CMAKE_CURRENT_LIST_DIR}/CMake"
			test_cxx_runtime
			CMAKE_FLAGS "-DCXX_RUNTIME=${CXX_RUNTIME_LIB}" ${CXX_RUNTIME_FLAGS})
		if (USERUNTIME)
			set(CXX_RUNTIME ${CXX_RUNTIME_LIB} PARENT_SCOPE)
		else ()
			unset(CXX_RUNTIME_LIB CACHE)
		endif()
	endif()
endfunction()

set(ENABLE_OBJCXX true CACHE BOOL
	"Enable support for Objective-C++")

set(CXXRT_IS_STDLIB false)

if(WIN32)
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(ASM_TARGET -m64)
	else()
		set(ASM_TARGET -m32)
	endif()
endif()


if (MSVC)
	set(ASSEMBLER ${CMAKE_ASM_COMPILER} CACHE STRING "Assembler to use with Visual Studio (must be gcc / clang!)")
	message(STATUS "Using custom build commands to work around CMake bugs")
	message(STATUS "ASM compiler: ${ASSEMBLER}")
	# CMake is completely broken when you try to build assembly files on Windows.
	add_custom_command(OUTPUT block_trampolines.obj
		COMMAND echo ${ASSEMBLER} ${ASM_TARGET} -c "${CMAKE_SOURCE_DIR}/block_trampolines.S" -o "${CMAKE_BINARY_DIR}/block_trampolines.obj"
		COMMAND ${ASSEMBLER} ${ASM_TARGET} -c "${CMAKE_SOURCE_DIR}/block_trampolines.S" -o "${CMAKE_BINARY_DIR}/block_trampolines.obj"
		MAIN_DEPENDENCY block_trampolines.S
	)
	add_custom_command(OUTPUT objc_msgSend.obj
		COMMAND echo ${ASSEMBLER} ${ASM_TARGET} -c "${CMAKE_SOURCE_DIR}/objc_msgSend.S" -o "${CMAKE_BINARY_DIR}/objc_msgSend.obj"
		COMMAND ${ASSEMBLER} ${ASM_TARGET} -c "${CMAKE_SOURCE_DIR}/objc_msgSend.S" -o "${CMAKE_BINARY_DIR}/objc_msgSend.obj"
		MAIN_DEPENDENCY objc_msgSend.S
		DEPENDS objc_msgSend.aarch64.S objc_msgSend.arm.S objc_msgSend.mips.S objc_msgSend.x86-32.S objc_msgSend.x86-64.S
	)
	set(libobjc_ASM_OBJS block_trampolines.obj objc_msgSend.obj)
endif()



if (ENABLE_OBJCXX)
	if (WIN32)
		message(STATUS "Using MSVC-compatible exception model")
	else ()
		message(STATUS "Testing C++ interop")
		# Try to find libcxxrt.so.  We can link to this to provide the C++ ABI
		# layer, if it exists.
		test_cxx(cxxrt false)
		# If it doesn't, then look for GNU libsupc++.so instead (either works,
		# they're ABI compatible).
		if (NOT CXX_RUNTIME)
			test_cxx(supc++ false)
		endif (NOT CXX_RUNTIME)
		if (NOT CXX_RUNTIME)
			list (FIND CMAKE_CXX_IMPLICIT_LINK_LIBRARIES "c++" _libcxx_index)
			if (${_libcxx_index} GREATER -1)
				test_cxx(c++abi false)
			endif()
		endif (NOT CXX_RUNTIME)

		# If we have a C++ ABI library, then we can produce a single libobjc that
		# works for Objective-C and Objective-C++.  If not, then we need to provide
		# a separate libobjcxx.
		if (CXX_RUNTIME)
			message(STATUS "Using ${CXX_RUNTIME} as the C++ runtime library")
		else()
			message(STATUS "Testing C++ standard library")
			message(STATUS "Using ${CMAKE_C_COMPILER}")
			try_compile(USERUNTIME 
				"${CMAKE_BINARY_DIR}/CMake"
				"${CMAKE_CURRENT_LIST_DIR}/CMake"
				test_cxx_stdlib)
			if (${USERUNTIME})
				message(STATUS "libobjc will depend on C++ standard library")
				set(CXXRT_IS_STDLIB true)
			else()
				message(STATUS "No useable C++ runtime found")
				set(ENABLE_OBJCXX false)
			endif()
		endif ()
		separate_arguments(EH_PERSONALITY_FLAGS NATIVE_COMMAND ${CMAKE_CXX_FLAGS})
		if (CMAKE_CXX_COMPILER_TARGET)
			list(APPEND EH_PERSONALITY_FLAGS "${CMAKE_CXX_COMPILE_OPTIONS_TARGET}${CMAKE_CXX_COMPILER_TARGET}")
		endif ()
		add_custom_command(OUTPUT eh_trampoline.s
			COMMAND ${CMAKE_CXX_COMPILER} ARGS ${EH_PERSONALITY_FLAGS} -fPIC -S "${CMAKE_SOURCE_DIR}/eh_trampoline.cc" -o - -fexceptions -fno-inline | sed "s/__gxx_personality_v0/test_eh_personality/g" > "${CMAKE_BINARY_DIR}/eh_trampoline.s"
			MAIN_DEPENDENCY eh_trampoline.cc)
		list(APPEND libobjc_ASM_SRCS eh_trampoline.s)
		list(APPEND libobjc_CXX_SRCS objcxx_eh.cc)

		# Find libm for linking, as some versions of libc++ don't link against it
		find_library(M_LIBRARY m)
	endif ()
endif (ENABLE_OBJCXX)

add_library(objc SHARED ${libobjc_C_SRCS} ${libobjc_ASM_SRCS} ${libobjc_OBJC_SRCS} ${libobjc_OBJCXX_SRCS} ${libobjc_ASM_OBJS})

if (ENABLE_OBJCXX)
	if (NOT CXXRT_IS_STDLIB)
		# We don't want to link the STL implementation (e.g. libstdc++) if
		# we have a separate C++ runtime.
		set(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES "")
		target_link_libraries(objc ${CXX_RUNTIME})
	endif()
	list(APPEND libobjc_CXX_SRCS ${libobjcxx_CXX_SRCS})
	target_sources(objc PRIVATE ${libobjc_CXX_SRCS})
else()
	add_definitions(-DNO_OBJCXX)
endif()


# Currently, we actually need pthreads, but we should use the platform's native
# threading implementation (we do for everything except thread-local storage)
set(CMAKE_THREAD_PREFER_PTHREAD)
include(FindThreads)
set(objc_LINK_FLAGS "${objc_LINK_FLAGS} ${CMAKE_THREAD_LIBS_INIT}")




set_target_properties(objc PROPERTIES
	LINKER_LANGUAGE C
	SOVERSION ${libobjc_VERSION}
	OUTPUT_NAME ${LIBOBJC_NAME}
	LINK_FLAGS "${objc_LINK_FLAGS}"
	)

set_property(TARGET PROPERTY NO_SONAME true)

set(BUILD_STATIC_LIBOBJC false CACHE BOOL
	"Build the static version of libobjc")
if (BUILD_STATIC_LIBOBJC)
	add_library(objc-static STATIC ${libobjc_C_SRCS} ${libobjc_ASM_SRCS} ${libobjc_OBJC_SRCS} ${libobjc_CXX_SRCS})
	set_target_properties(objc-static PROPERTIES
		POSITION_INDEPENDENT_CODE true
		OUTPUT_NAME ${LIBOBJC_NAME})
	list(APPEND INSTALL_TARGETS objc-static)
endif ()

# Explicitly link libgc if we are compiling with gc support.
if (LIBGC)
	target_link_libraries(objc ${LIBGC})
endif ()

# Explicitly link libm, as an implicit dependency of the C++ runtime
if (M_LIBRARY)
	target_link_libraries(objc ${M_LIBRARY})
endif ()

# Make weak symbols work on OS X
if (APPLE)
	set(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS
		"${CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS} -undefined dynamic_lookup")
	set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,-undefined,dynamic_lookup")
	set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -Wl,-undefined,dynamic_lookup")
endif ()

#
# Installation
#


find_program(GNUSTEP_CONFIG gnustep-config)
if (GNUSTEP_CONFIG)
	EXEC_PROGRAM(gnustep-config
		ARGS "--installation-domain-for=libobjc2"
		OUTPUT_VARIABLE DEFAULT_INSTALL_TYPE)
endif ()


# If we have GNUstep environment variables, then default to installing in the
# GNUstep local environment.
if (DEFAULT_INSTALL_TYPE)
else ()
	set(DEFAULT_INSTALL_TYPE "NONE")
endif ()

if (NOT CMAKE_INSTALL_LIBDIR)
	set(CMAKE_INSTALL_LIBDIR lib)
endif ()


set(GNUSTEP_INSTALL_TYPE ${DEFAULT_INSTALL_TYPE} CACHE STRING
	"GNUstep installation type.  Options are NONE, SYSTEM, NETWORK or LOCAL.")
if (${GNUSTEP_INSTALL_TYPE} STREQUAL "NONE")
	SET(LIB_INSTALL_PATH "${CMAKE_INSTALL_LIBDIR}" CACHE STRING
		"Subdirectory of the root prefix where libraries are installed.")
	SET(HEADER_INSTALL_PATH "include")
	SET(PC_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
else ()
	EXEC_PROGRAM(gnustep-config
		ARGS "--variable=GNUSTEP_${GNUSTEP_INSTALL_TYPE}_LIBRARIES"
		OUTPUT_VARIABLE LIB_INSTALL_PATH)
	EXEC_PROGRAM(gnustep-config
		ARGS "--variable=GNUSTEP_${GNUSTEP_INSTALL_TYPE}_HEADERS"
		OUTPUT_VARIABLE HEADER_INSTALL_PATH)
	SET(PC_INSTALL_PREFIX "/")
endif ()
message(STATUS "GNUstep install type set to ${GNUSTEP_INSTALL_TYPE}")

install(TARGETS ${INSTALL_TARGETS}
	RUNTIME DESTINATION ${LIB_INSTALL_PATH}
	LIBRARY DESTINATION ${LIB_INSTALL_PATH}
	ARCHIVE DESTINATION ${LIB_INSTALL_PATH})
install(FILES ${libobjc_HDRS}
	DESTINATION "${HEADER_INSTALL_PATH}/${INCLUDE_DIRECTORY}")
install(FILES ${libBlocksRuntime_COMPATIBILITY_HDRS}
	DESTINATION "${HEADER_INSTALL_PATH}")

set(CPACK_GENERATOR TGZ CACHE STRING
	"Installer types to generate.  Sensible options include TGZ, RPM and DEB")

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "GNUstep Objective-C Runtime")
set(CPACK_PACKAGE_VENDOR "The GNUstep Project")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set(CPACK_PACKAGE_VERSION_MAJOR "2")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_CONTACT "GNUstep Developer <gnustep-dev@gnu.org>")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}")
if (UNIX)
	set(CPACK_STRIP_FILES true CACHE BOOL "Strip libraries when packaging")
endif ()
include (CPack)

# pkg-config descriptor

if (BOEHM_GC)
	set(PC_REQUIRES_PRIVATE_BOEHM_GC "Requires.private: bdw-gc")
endif ()
if (ENABLE_OBJCXX)
	if (CXXRT_IS_STDLIB)
		set(PC_LIBS_PRIVATE ${CMAKE_CXX_IMPLICIT_LINK_LIBRARIES})
	else()
		list(APPEND PC_LIBS_PRIVATE ${CXX_RUNTIME})
		if (M_LIBRARY)
			list(APPEND PC_LIBS_PRIVATE ${M_LIBRARY})
		endif ()
	endif()
	list(REMOVE_DUPLICATES PC_LIBS_PRIVATE)
	string(REPLACE  ";" " -l" PC_LIBS_PRIVATE "${PC_LIBS_PRIVATE}")
	set(PC_LIBS_PRIVATE "Libs.private: -l${PC_LIBS_PRIVATE}")
endif()

configure_file("libobjc.pc.in" "libobjc.pc" @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libobjc.pc"
	DESTINATION "${LIB_INSTALL_PATH}/pkgconfig"
)


# uninstall target
configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
	IMMEDIATE @ONLY)

add_custom_target(uninstall
	COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)


set(TESTS TRUE CACHE BOOL
	"Enable building the tests")

if (TESTS)
	enable_testing()
	add_subdirectory(Test)
endif (TESTS)

CHECK_CXX_SOURCE_COMPILES("
	#include <stdlib.h>
	extern \"C\" {
	__attribute__((weak))
	void *__cxa_allocate_exception(size_t thrown_size) noexcept;
	}
	#include <exception>
	int main() { return 0; }" CXA_ALLOCATE_EXCEPTION_NOEXCEPT_COMPILES)

if (CXA_ALLOCATE_EXCEPTION_NOEXCEPT_COMPILES)
	add_definitions(-DCXA_ALLOCATE_EXCEPTION_SPECIFIER=noexcept)
else ()
	add_definitions(-DCXA_ALLOCATE_EXCEPTION_SPECIFIER=)
endif ()
