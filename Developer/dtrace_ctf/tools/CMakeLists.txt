find_package(ZLIB REQUIRED)

add_executable(host_ctfconvert)
target_sources(host_ctfconvert PRIVATE
    alist.cpp
    array.c
    atom.cpp
    barrier.c
    ctf.c
    ctfconvert.c
    darwin_shim.c
    dwarf.c
    fifo.c
    hash.c
    iidesc.c
    input.c
    list.c
    memory.c
    merge.c
    output.c
    stack.c
    strtab.cpp
    symbol.c
    tdata.c
    traverse.c
    util.c
)

set_property(TARGET host_ctfconvert PROPERTY OUTPUT_NAME "ctfconvert")
target_include_directories(host_ctfconvert PRIVATE ../include ../include/sys ../libelf)
target_link_libraries(host_ctfconvert PRIVATE libelf.host libdwarf.host libctf.host ${ZLIB_LIBRARY_RELEASE})
set_property(TARGET host_ctfconvert PROPERTY CXX_STANDARD 14)
suppress_all_warnings(host_ctfconvert)

add_executable(host_ctfdump)
target_sources(host_ctfdump PRIVATE
    alist.cpp
    array.c
    atom.cpp
    ctf.c
    darwin_shim.c
    dump.c
    fifo.c
    hash.c
    iidesc.c
    input.c
    list.c
    memory.c
    output.c
    stack.c
    strtab.cpp
    symbol.c
    tdata.c
    traverse.c
    util.c
    utils.c
)

set_property(TARGET host_ctfdump PROPERTY OUTPUT_NAME "ctfdump")
target_include_directories(host_ctfdump PRIVATE ../include ../include/sys ../libelf)
target_link_libraries(host_ctfdump PRIVATE libelf.host libctf.host ${ZLIB_LIBRARY_RELEASE})
set_property(TARGET host_ctfdump PROPERTY CXX_STANDARD 14)
suppress_all_warnings(host_ctfdump)

add_executable(host_ctfmerge)
target_sources(host_ctfmerge PRIVATE
    alist.cpp
    array.c
    atom.cpp
    barrier.c
    ctf.c
    ctfmerge.c
    darwin_shim.c
    dwarf.c
    fifo.c
    hash.c
    iidesc.c
    input.c
    list.c
    memory.c
    merge.c
    output.c
    stack.c
    strtab.cpp
    symbol.c
    tdata.c
    traverse.c
    util.c
    utils.c
)

set_property(TARGET host_ctfmerge PROPERTY OUTPUT_NAME ctfmerge)
target_include_directories(host_ctfmerge PRIVATE ../include ../include/sys ../libelf)
target_link_libraries(host_ctfmerge PRIVATE libelf.host libdwarf.host libctf.host ${ZLIB_LIBRARY_RELEASE})
set_property(TARGET host_ctfmerge PROPERTY CXX_STANDARD 14)
suppress_all_warnings(host_ctfmerge)

if("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
    target_compile_options(host_ctfconvert PRIVATE -w)
    target_compile_options(host_ctfmerge PRIVATE -w)
    target_compile_options(host_ctfdump PRIVATE -w)
    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/../../llvm-project/llvm/include
        ${ROOT_SOURCE_DIR}/Libraries/libdispatch
    )
endif()