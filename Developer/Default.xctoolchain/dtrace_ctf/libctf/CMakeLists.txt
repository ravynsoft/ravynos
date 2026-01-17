if("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
    set(LINUX_SRCS qsort.c fls.c)
    add_compile_options(-Wno-error -std=gnu99)
endif()

add_library(libctf.host STATIC)
target_sources(libctf.host PRIVATE
    ctf_create.c
    ctf_decl.c
    ctf_error.c
    ctf_hash.c
    ctf_labels.c
    ctf_lib.c
    ctf_lookup.c
    ctf_open.c
    ctf_subr.c
    ctf_types.c
    ctf_util.c
    ${LINUX_SRCS}

    ctf_impl.h
    ../include/libctf.h
)

set_property(TARGET libctf.host PROPERTY OUTPUT_NAME ctf)
target_include_directories(libctf.host PRIVATE ../include ../include/sys)
target_compile_definitions(libctf.host PRIVATE __APPLE_CPP__ __LITTLE_ENDIAN__)
target_compile_definitions(libctf.host PRIVATE CTF_OLD_VERSIONS)
