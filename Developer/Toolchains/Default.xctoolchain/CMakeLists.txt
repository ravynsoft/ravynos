add_library(BootstrapTools STATIC stub.c)
add_dependencies(BootstrapTools
    host_libmacho host_libstuff
    host_checksyms host_lipo host_size host_strings host_nm host_libtool
    host_redo_prebinding host_seg_addr_table host_seg_hack host_install_name_tool
    host_indr host_strip host_segedit host_pagestuff host_codesign_allocate
    host_bitcode_strip host_ctf_insert host_check_dylib host_cmpdylib host_inout
    host_vtool host_nmedit host_ranlib host_migcom host_xcbuild dtrace_ctf_host
    host_clang host_ld host_xar host_libxar
)

file(MAKE_DIRECTORY ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/libexec)
file(MAKE_DIRECTORY ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/include)
file(MAKE_DIRECTORY ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/lib)
file(MAKE_DIRECTORY ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/bin)
file(COPY_FILE ${CMAKE_SOURCE_DIR}/mig/mig.sh
    ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/bin/mig ONLY_IF_DIFFERENT)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/ToolchainInfo.plist
    DESTINATION ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain)
file(CREATE_LINK Default.xctoolchain
    ${ROOT_BINARY_DIR}/Developer/Toolchains/XcodeDefault.xctoolchain SYMBOLIC)

add_custom_command(TARGET BootstrapTools
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_xar> ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_libxar> ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/lib/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_ld> ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_checksyms> ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_lipo> ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_size> ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_strings> ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_nm> ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_libtool> ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_redo_prebinding> ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_seg_addr_table> ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_seg_hack> ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_install_name_tool> ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_indr> ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_strip> ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_segedit> ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_pagestuff> ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_codesign_allocate> ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_bitcode_strip> ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_ctf_insert> ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_check_dylib> ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_cmpdylib> ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_inout> ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_vtool> ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_nmedit> ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_ranlib> ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:unifdef> ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host_migcom> ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/libexec/migcom
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/dtrace_ctf_host/tools/ctfconvert ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/dtrace_ctf_host/tools/ctfdump ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/dtrace_ctf_host/tools/ctfmerge ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E create_symlink llvm-otool ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/bin/otool
    COMMAND ${CMAKE_COMMAND} -E create_symlink llvm-as ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/bin/as
    COMMAND ${CMAKE_COMMAND} -E create_symlink ld ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/bin/ld64
    COMMAND ${CMAKE_COMMAND} -E create_symlink clang ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/bin/cc
    COMMAND ${CMAKE_COMMAND} -E create_symlink clang++ ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/bin/c++
    COMMAND ${CMAKE_COMMAND} -E rm -f ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/cmake_install.cmake
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/libBootstrapTools.a
)