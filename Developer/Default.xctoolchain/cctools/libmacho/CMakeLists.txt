add_library(host_libmacho STATIC)
target_sources(host_libmacho PRIVATE
    arch.c getsecbyname.c getsegbyname.c
)
set_property(TARGET host_libmacho PROPERTY OUTPUT_NAME macho)
target_include_directories(host_libmacho PRIVATE
    ${DEVEL}/Default.xctoolchain/dtrace_ctf/include
)
