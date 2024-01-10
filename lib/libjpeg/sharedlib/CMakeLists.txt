# Anything that must be linked against the shared C library on Windows must
# be built in this subdirectory, because CMake doesn't allow us to override
# the compiler flags for each build type except at directory scope.  Note
# to CMake developers:  Add a COMPILE_FLAGS_<CONFIG> target property, or
# better yet, provide a friendly way of configuring a Windows target to use the
# static C library.

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/..)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/..)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/..)

if(MSVC)
  # Build all configurations against shared C library
  foreach(var CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
    CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
    if(${var} MATCHES "/MT")
      string(REGEX REPLACE "/MT" "/MD" ${var} "${${var}}")
    endif()
  endforeach()
endif()

foreach(src ${JPEG_SOURCES})
  set(JPEG_SRCS ${JPEG_SRCS} ../${src})
endforeach()

if(WITH_SIMD AND (MSVC_IDE OR XCODE))
  # This tells CMake that the "source" files haven't been generated yet
  set_source_files_properties(${SIMD_OBJS} PROPERTIES GENERATED 1)
endif()

if(WIN32)
  set(DEFFILE ../win/jpeg${SO_MAJOR_VERSION}.def)
endif()
if(MSVC)
  configure_file(${CMAKE_SOURCE_DIR}/win/jpeg.rc.in
    ${CMAKE_BINARY_DIR}/win/jpeg.rc)
  set(JPEG_SRCS ${JPEG_SRCS} ${CMAKE_BINARY_DIR}/win/jpeg.rc)
endif()
add_library(jpeg SHARED ${JPEG_SRCS} ${DEFFILE} ${SIMD_TARGET_OBJECTS}
  ${SIMD_OBJS} $<TARGET_OBJECTS:jpeg12> $<TARGET_OBJECTS:jpeg16>)

set_target_properties(jpeg PROPERTIES SOVERSION ${SO_MAJOR_VERSION}
  VERSION ${SO_MAJOR_VERSION}.${SO_AGE}.${SO_MINOR_VERSION})
if(APPLE AND (NOT CMAKE_OSX_DEPLOYMENT_TARGET OR
              CMAKE_OSX_DEPLOYMENT_TARGET VERSION_GREATER 10.4))
  if(NOT CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG)
    set(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG "-Wl,-rpath,")
  endif()
  set_target_properties(jpeg PROPERTIES MACOSX_RPATH 1)
endif()
if(MAPFLAG)
  set_target_properties(jpeg PROPERTIES
    LINK_FLAGS "${MAPFLAG}${CMAKE_CURRENT_BINARY_DIR}/../libjpeg.map")
endif()
if(MSVC)
  set_target_properties(jpeg PROPERTIES
    RUNTIME_OUTPUT_NAME jpeg${SO_MAJOR_VERSION})
  # The jsimd_*.c file is built using /MT, so this prevents a linker warning.
  set_target_properties(jpeg PROPERTIES LINK_FLAGS
    "/NODEFAULTLIB:LIBCMT /NODEFAULTLIB:LIBCMTD")
elseif(MINGW)
  set_target_properties(jpeg PROPERTIES SUFFIX -${SO_MAJOR_VERSION}.dll)
endif()

if(WIN32)
  set(USE_SETMODE "-DUSE_SETMODE")
endif()
set(CDJPEG_COMPILE_FLAGS
  "-DBMP_SUPPORTED -DGIF_SUPPORTED -DPPM_SUPPORTED -DTARGA_SUPPORTED ${USE_SETMODE}")

# Compile a separate version of these source files with 12-bit and 16-bit data
# precision.
add_library(cjpeg12 OBJECT ../rdgif.c ../rdppm.c)
set_property(TARGET cjpeg12 PROPERTY COMPILE_FLAGS
  "-DBITS_IN_JSAMPLE=12 -DGIF_SUPPORTED -DPPM_SUPPORTED")
add_library(cjpeg16 OBJECT ../rdgif.c ../rdppm.c)
set_property(TARGET cjpeg16 PROPERTY COMPILE_FLAGS
  "-DBITS_IN_JSAMPLE=16 -DGIF_SUPPORTED -DPPM_SUPPORTED")
add_executable(cjpeg ../cjpeg.c ../cdjpeg.c ../rdbmp.c ../rdgif.c ../rdppm.c
  ../rdswitch.c ../rdtarga.c $<TARGET_OBJECTS:cjpeg12>
  $<TARGET_OBJECTS:cjpeg16>)
set_property(TARGET cjpeg PROPERTY COMPILE_FLAGS ${CDJPEG_COMPILE_FLAGS})
target_link_libraries(cjpeg jpeg)

# Compile a separate version of these source files with 12-bit and 16-bit data
# precision.
add_library(djpeg12 OBJECT ../rdcolmap.c ../wrgif.c ../wrppm.c)
set_property(TARGET djpeg12 PROPERTY COMPILE_FLAGS
  "-DBITS_IN_JSAMPLE=12 -DGIF_SUPPORTED -DPPM_SUPPORTED")
add_library(djpeg16 OBJECT ../wrppm.c)
set_property(TARGET djpeg16 PROPERTY COMPILE_FLAGS
  "-DBITS_IN_JSAMPLE=16 -DPPM_SUPPORTED")
add_executable(djpeg ../djpeg.c ../cdjpeg.c ../rdcolmap.c ../rdswitch.c
  ../wrbmp.c ../wrgif.c ../wrppm.c ../wrtarga.c $<TARGET_OBJECTS:djpeg12>
  $<TARGET_OBJECTS:djpeg16>)
set_property(TARGET djpeg PROPERTY COMPILE_FLAGS ${CDJPEG_COMPILE_FLAGS})
target_link_libraries(djpeg jpeg)

add_executable(jpegtran ../jpegtran.c ../cdjpeg.c ../rdswitch.c ../transupp.c)
target_link_libraries(jpegtran jpeg)
set_property(TARGET jpegtran PROPERTY COMPILE_FLAGS "${USE_SETMODE}")

add_executable(example ../example.c)
target_link_libraries(example jpeg)

add_executable(jcstest ../jcstest.c)
target_link_libraries(jcstest jpeg)

install(TARGETS jpeg EXPORT ${CMAKE_PROJECT_NAME}Targets
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(TARGETS cjpeg djpeg jpegtran
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
if(NOT CMAKE_VERSION VERSION_LESS "3.1" AND MSVC AND
  CMAKE_C_LINKER_SUPPORTS_PDB)
  install(FILES "$<TARGET_PDB_FILE:jpeg>"
    DESTINATION ${CMAKE_INSTALL_BINDIR} OPTIONAL)
endif()
