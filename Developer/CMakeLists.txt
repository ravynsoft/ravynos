# ------------------------------------------------------------------------
#  RAVYNOS BUILD SYSTEM - Bootstrap and Developer Tools
# ------------------------------------------------------------------------

cmake_minimum_required(VERSION 4.0.0)
project(ravynOS_Developer)

if(CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
    execute_process(
        COMMAND xcrun --sdk macosx --show-sdk-path
        OUTPUT_VARIABLE HOST_SDKROOT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(EXTRA_ARGS 
        -C ${CMAKE_SOURCE_DIR}/Default.xctoolchain/llvm/tapi/cmake/caches/apple-tapi.cmake
        -DCMAKE_OSX_SYSROOT=${HOST_SDKROOT} -DLLVM_BUILD_LLVM_DYLIB=ON
        -DLLVM_LINK_LLVM_DYLIB=ON -DLLVM_CREATE_XCODE_TOOLCHAIN=ON
    )
    set(RPATH @executable_path)
else()
    set(RPATH "$ORIGIN")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
endif()

add_subdirectory(ravynOS.sdk)
add_subdirectory(Default.xctoolchain)

