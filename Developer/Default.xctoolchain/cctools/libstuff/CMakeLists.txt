add_library(host_libstuff STATIC)
target_sources(host_libstuff PRIVATE
    allocate.c
    apple_version.c
    arch_usage.c
    arch.c
    args.c
    best_arch.c
    breakout.c
    bytesex.c
    checkout.c
    coff_bytesex.c
    crc32.c
    dylib_table.c
    dylib_roots.c
    errors.c
    execute.c
    fatal_arch.c
    fatals.c
    get_arch_from_host.c
    get_toc_byte_sex.c
    guess_short_name.c
    hash_string.c
    hppa.c
    llvm.c
    lto.c
    macosx_deployment_target.c
    ofile_error.c
    ofile_get_word.c
    ofile.c
    port.c
    print.c
    reloc.c
    rnd.c
    seg_addr_table.c
    set_arch_flag_name.c
    swap_headers.c
    symbol_list.c
    SymLoc.c
    unix_standard_mode.c
    version_number.c
    vm_flush_cache.c
    write64.c
    writeout.c
)

set_property(TARGET host_libstuff PROPERTY OUTPUT_NAME stuff)
target_compile_definitions(host_libstuff PRIVATE
    __DARWIN_UNIX03
    PROGRAM_PREFIX=\"\"
    __LITTLE_ENDIAN__=1234
)
if(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
    target_compile_options(host_libstuff PRIVATE
        -w -Wno-error -std=gnu99 -UHAVE_STRMODE
    )
    target_compile_definitions(host_libstuff PRIVATE
        __private_extern__=extern
    )
    # target_include_directories(host_libstuff PRIVATE
    #     ${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk
    # )
endif()
