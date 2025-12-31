
# This file is included from Kernel/CmakeLists.txt

set(ENV{DERIVED_DATA_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(libplatform)
add_subdirectory(libmalloc)
# add_subdirectory(libdispatch)
add_subdirectory(libpthread)
# add_subdirectory(Libc)
# add_subdirectory(CommonCrypto)
# add_subdirectory(Libsystem)
