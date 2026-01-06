# This file sets up a CMakeCache for Apple-style TAPI build.

include(${CMAKE_CURRENT_LIST_DIR}/tapi-defaults.cmake)

set(PACKAGE_VENDOR Apple CACHE STRING "" FORCE)
set(BUG_REPORT_URL "http://developer.apple.com/bugreporter/" CACHE STRING "" FORCE)
set(LLVM_EXTERNALIZE_DEBUGINFO ON CACHE BOOL "" FORCE)
set(LLVM_INCLUDE_TESTS OFF CACHE BOOL "" FORCE)
set(LLVM_INCLUDE_BENCHMARKS OFF CACHE BOOL "" FORCE)

set(LLVM_ENABLE_LTO Thin CACHE BOOL "" FORCE)

set(LLVM_DISTRIBUTION_COMPONENTS
  tapi-headers
  libtapi
  tapi-clang-resource-headers
  tapi
  tapi-docs
  tapi-sdkdb
  ${LLVM_TOOLCHAIN_TOOLS}
  CACHE STRING "" FORCE)
