include_directories(.)
add_compile_options(-mmacos-version-min=10.15 --sysroot=${RAVYN_SDKROOT})

add_library(libsbuf STATIC sbuf.c)
set_target_properties(libsbuf PROPERTIES PREFIX "lib" OUTPUT_NAME "sbuf")

add_library(libnv STATIC bsd_nvpair.c nvlist.c msgio.c)
set_target_properties(libnv PROPERTIES PREFIX "lib" OUTPUT_NAME "nv")
target_include_directories(libnv PRIVATE  ../openbsm/compat)
target_compile_definitions(libnv PRIVATE "__packed=__attribute__((packed))")

add_dependencies(libnv libc_static)
add_dependencies(libsbuf libc_static)
