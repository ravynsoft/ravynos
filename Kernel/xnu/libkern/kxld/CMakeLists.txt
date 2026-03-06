cmake_minimum_required(VERSION 4.0.0)
project(ravynOS_kxld C)

if(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
    set(IIG /bin/true)
    #file(GLOB IIG_HDRS ../../iokit/DriverKit/*.h)
    #file(COPY ${IIG_HDRS} DESTINATION 
    #    ${CMAKE_CURRENT_BINARY_DIR}/xnu/${KERNEL_CONFIGS}_${ARCH_CONFIGS}/iokit/DriverKit/)
endif()

add_library(kxld STATIC
    kxld.c
    kxld_array.c
    kxld_copyright.c
    kxld_demangle.c
    kxld_dict.c
    kxld_kext.c
    kxld_object.c
    kxld_reloc.c
    kxld_sect.c
    kxld_seg.c
    kxld_srcversion.c
    kxld_splitinfolc.c
    kxld_sym.c
    kxld_symtab.c
    kxld_util.c
    kxld_uuid.c
    kxld_vtable.c
    kxld_versionmin.c
)

set_target_properties(kxld PROPERTIES C_STANDARD 99 OUTPUT_NAME "kxld")

target_compile_definitions(kxld PRIVATE
    __DARWIN_BYTE_ORDER=__DARWIN_LITTLE_ENDIAN
    "__unused=__attribute__((unused))"
    "__header_always_inline=inline"
)

target_include_directories(kxld PRIVATE
    ../libkern
    ../../EXTERNAL_HEADERS
    ../../../../Developer/Default.xctoolchain/include
    ..
)

target_compile_options(kxld PRIVATE -DPRIVATE -g
    -include../../bsd/sys/queue.h
)
