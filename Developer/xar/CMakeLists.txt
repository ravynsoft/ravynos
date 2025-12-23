find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(LibXml2 REQUIRED)

add_library(host_libxar STATIC)
target_sources(host_libxar PRIVATE
    lib/archive.c
    lib/arcmod.c
    lib/b64.c
    lib/bzxar.c
    lib/darwinattr.c
    lib/data.c
    lib/ea.c
    lib/err.c
    lib/ext2.c
    lib/fbsdattr.c
    lib/filetree.c
    lib/hash.c
    lib/io.c
    lib/linuxattr.c
    lib/lzmaxar.c
    lib/macho.c
    lib/script.c
    lib/signature.c
    lib/stat.c
    lib/subdoc.c
    lib/util.c
    lib/zxar.c
)
set_property(TARGET host_libxar PROPERTY OUTPUT_NAME xar)
target_include_directories(host_libxar PUBLIC include /usr/local/include)
target_include_directories(host_libxar PRIVATE ${OPENSSL_INCLUDE_DIR} ${LIBXML2_INCLUDE_DIR})
target_link_libraries(host_libxar PRIVATE ZLIB::ZLIB LibXml2::LibXml2)
target_compile_options(host_libxar PRIVATE -w)

add_executable(host_xar)
target_sources(host_xar PRIVATE
    src/xar.c
)
set_property(TARGET host_xar PROPERTY OUTPUT_NAME xar)
target_link_libraries(host_xar PRIVATE
    host_libxar 
    OpenSSL::SSL OpenSSL::Crypto 
    ZLIB::ZLIB 
    LibXml2::LibXml2
    host_libstuff
)
target_compile_options(host_xar PRIVATE -w)
if("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
    target_link_libraries(host_xar PRIVATE -lacl)
endif()