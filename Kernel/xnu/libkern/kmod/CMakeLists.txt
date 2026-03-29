set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_C_COMPILER ${CC})
set(CMAKE_ASM_COMPILER ${CC})
set(CMAKE_CXX_COMPILER ${CXX})
set(CMAKE_LINK_DEPENDS_USE_LINKER OFF)

add_library(libkmod STATIC c_start.c c_stop.c)
set_property(TARGET libkmod PROPERTY OUTPUT_NAME "kmod")
add_dependencies(libkmod xnu_headers xnu)
target_compile_options(libkmod PRIVATE
     --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=10.15
     -I${RAVYN_SDKROOT}/usr/local/include/kernel
     -I${CMAKE_SOURCE_DIR}/Frameworks
     -I${ROOT_SOURCE_DIR}/Frameworks
)

