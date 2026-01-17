find_package(ZLIB REQUIRED)

if(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
    include_directories(../../include)
    add_compile_options(
        -w
        -Wno-error=incompatible-pointer-types
        -Wno-error=implicit-function-declaration
        -Wno-error=int-conversion
        -Wno-error=implicit-int
    )
endif()

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
set_property(TARGET host_ctfconvert PROPERTY CXX_STANDARD 17)

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
set_property(TARGET host_ctfdump PROPERTY CXX_STANDARD 17)

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
target_compile_definitions(host_ctfmerge PRIVATE __APPLE_CPP__ __LITTLE_ENDIAN__)
set_property(TARGET host_ctfmerge PROPERTY CXX_STANDARD 17)

if(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
    target_link_libraries(host_ctfconvert PRIVATE -lLLVM -ldispatch)
    target_link_libraries(host_ctfdump PRIVATE -lLLVM -ldispatch)
    target_link_libraries(host_ctfmerge PRIVATE -lLLVM -ldispatch)
endif()