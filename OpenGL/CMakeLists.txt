project(OpenGL)

include(darling_framework)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} \
	-include math.h \
	-include stdlib.h \
	-include CoreFoundation/CoreFoundation.h \
	-include Foundation/Foundation.h \
	-Wno-nonportable-include-path \
	-Wno-deprecated-objc-isa-usage \
	-mmacosx-version-min=10.10"
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include/OpenGL
)

set(OpenGL_sources
	OpenGL.m

	# weird, I know
	../AppKit/X11.subproj/CGLContext.m
)

set_source_files_properties(${OpenGL_sources} LANGUAGE C)

add_framework(OpenGL
	FAT
	CURRENT_VERSION
	SOURCES
		${OpenGL_sources}
	VERSION "A"
	DEPENDENCIES
		objc
		system
		CoreFoundation
		Foundation
		# native libraries
		GL
		X11
	CIRCULAR_DEPENDENCIES
		AppKit
)
