include(${CMAKE_CURRENT_LIST_DIR}/tapi-defaults.cmake)

set(PACKAGE_VENDOR Apple CACHE STRING "" FORCE)
set(BUG_REPORT_URL "http://developer.apple.com/bugreporter/" CACHE STRING "" FORCE)

set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
set(LLVM_ENABLE_LTO Thin CACHE BOOL "" FORCE)
