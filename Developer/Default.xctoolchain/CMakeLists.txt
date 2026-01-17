add_library(BootstrapTools STATIC stub.c)
set_property(TARGET BootstrapTools PROPERTY OUTPUT_DIRECTORY ${TOOLCHAIN})

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/llvm/tapi/include/tapi/Version.inc.in
    ${CMAKE_CURRENT_BINARY_DIR}/tapi/Version.inc
)

include_directories(./include)

add_subdirectory(cctools)
add_subdirectory(mig)
add_subdirectory(unifdef)
add_subdirectory(xar)
add_subdirectory(dtrace_ctf)

ExternalProject_Add(host_xcbuild
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/xcbuild
    CMAKE_ARGS -Wno-dev
        -DROOT_SOURCE_DIR=${ROOT_SOURCE_DIR}
        -DROOT_BINARY_DIR=${ROOT_BINARY_DIR}
        -DCMAKE_BUILD_RPATH=${RPATH}/../lib 
        -DCMAKE_INSTALL_RPATH=${RPATH}/../lib
        -DCMAKE_INSTALL_PREFIX=${TOOLCHAIN}
)

ExternalProject_Add(host_clang DEPENDS host_migcom
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/llvm/llvm
    BINARY_DIR llvm_host
    USES_TERMINAL_BUILD
    CMAKE_ARGS
        ${EXTRA_ARGS} -Wno-dev
        -DCMAKE_INSTALL_PREFIX=${TOOLCHAIN}/usr -DCMAKE_BUILD_TYPE=Release
        -DLLVM_ENABLE_LIBCXX=ON -DLLVM_APPEND_VC_REV=OFF
        -DLLVM_TARGETS_TO_BUILD=${BuildArch} -DLLVM_ENABLE_LTO=OFF
        -DLLVM_BUILD_DOCS=OFF -DLLVM_INCLUDE_TESTS=OFF
        -DLLVM_INSTALL_BINUTILS_SYMLINKS=ON -DLLVM_NATIVE_TOOL_DIR=${TOOLS}
        -DLLVM_DEFAULT_TARGET_TRIPLE=${TARGET_TRIPLE}
        -DLLVM_INCLUDE_BENCHMARKS=OFF -DLLVM_ENABLE_WARNINGS=OFF
        -DLLVM_ENABLE_PROJECTS=clang$<SEMICOLON>tapi$<SEMICOLON>lld
        -DLLVM_BUILD_RUNTIMES=OFF -DLLVM_BUILD_BUILTINS=OFF
)

set(GNU_CONFIGURE_ARGS
    --prefix=/usr --sysconfdir=/etc --bindir=/bin --sbindir=/sbin --localstatedir=/var
)
add_custom_target(host_gmake
    COMMAND 
        if [ ! -f ${CMAKE_CURRENT_BINARY_DIR}/gmake/make ] $<SEMICOLON> then
          ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/gmake
          && cd ${CMAKE_CURRENT_BINARY_DIR}/gmake
          && ${CMAKE_CURRENT_SOURCE_DIR}/gmake/configure ${GNU_CONFIGURE_ARGS}
          && ./build.sh
          && ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/gmake/make
            ${TOOLCHAIN}/usr/bin/ $<SEMICOLON>
        fi
)

add_dependencies(BootstrapTools
    host_libmacho host_libstuff host_gmake
    host_checksyms host_lipo host_size host_strings host_nm host_libtool
    host_redo_prebinding host_seg_addr_table host_seg_hack host_install_name_tool
    host_indr host_strip host_segedit host_pagestuff host_codesign_allocate
    host_bitcode_strip host_ctf_insert host_check_dylib host_cmpdylib host_inout
    host_vtool host_nmedit host_ranlib host_migcom host_xcbuild host_ctfconvert
    host_ctfmerge host_ctfdump host_clang host_ld host_xar host_libxar
)

file(MAKE_DIRECTORY ${TOOLCHAIN}/usr/libexec)
file(MAKE_DIRECTORY ${TOOLCHAIN}/usr/include)
file(MAKE_DIRECTORY ${TOOLCHAIN}/usr/lib)
file(MAKE_DIRECTORY ${TOOLCHAIN}/usr/bin)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/ToolchainInfo.plist DESTINATION ${TOOLCHAIN})
file(CREATE_LINK Default.xctoolchain
    ${ROOT_BINARY_DIR}/Developer/Toolchains/XcodeDefault.xctoolchain SYMBOLIC)

file(COPY_FILE ${CMAKE_CURRENT_SOURCE_DIR}/mig/mig.sh ${TOOLS}/mig)
# "MIG" variable already set in Tools.cmake

add_custom_command(TARGET BootstrapTools
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_xar> ${TOOLCHAIN}/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_libxar> ${TOOLCHAIN}/usr/lib/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_ld> ${TOOLCHAIN}/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_checksyms> ${TOOLCHAIN}/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_lipo> ${TOOLCHAIN}/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_size> ${TOOLCHAIN}/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_strings> ${TOOLCHAIN}/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_nm> ${TOOLCHAIN}/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_libtool> ${TOOLCHAIN}/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_redo_prebinding> ${TOOLCHAIN}/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_seg_addr_table> ${TOOLCHAIN}/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_seg_hack> ${TOOLCHAIN}/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_install_name_tool> ${TOOLCHAIN}/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_indr> ${TOOLCHAIN}/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_strip> ${TOOLCHAIN}/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_segedit> ${TOOLCHAIN}/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_pagestuff> ${TOOLCHAIN}/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_codesign_allocate> ${TOOLCHAIN}/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_bitcode_strip> ${TOOLCHAIN}/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_ctf_insert> ${TOOLCHAIN}/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_check_dylib> ${TOOLCHAIN}/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_cmpdylib> ${TOOLCHAIN}/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_inout> ${TOOLCHAIN}/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_vtool> ${TOOLCHAIN}/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_nmedit> ${TOOLCHAIN}/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_ranlib> ${TOOLCHAIN}/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:unifdef> ${TOOLCHAIN}/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_migcom> ${TOOLCHAIN}/usr/libexec/migcom
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_ctfconvert> ${TOOLCHAIN}/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_ctfdump> ${TOOLCHAIN}/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_ctfmerge> ${TOOLCHAIN}/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E create_symlink llvm-otool ${TOOLCHAIN}/usr/bin/otool
    COMMAND ${CMAKE_COMMAND} -E create_symlink llvm-as ${TOOLCHAIN}/usr/bin/as
    COMMAND ${CMAKE_COMMAND} -E create_symlink ld ${TOOLCHAIN}/usr/bin/ld64
    COMMAND ${CMAKE_COMMAND} -E create_symlink clang ${TOOLCHAIN}/usr/bin/cc
    COMMAND ${CMAKE_COMMAND} -E create_symlink clang++ ${TOOLCHAIN}/usr/bin/c++
    COMMAND ${CMAKE_COMMAND} -E rm -f ${TOOLCHAIN}/cmake_install.cmake
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${TOOLCHAIN}/CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${TOOLCHAIN}/libBootstrapTools.a
)
