add_library(host_libmacho STATIC)
target_sources(host_libmacho PRIVATE
    arch.c
)
set_property(TARGET host_libmacho PROPERTY OUTPUT_NAME macho)
target_include_directories(host_libmacho PRIVATE ../include ../include/foreign)
if("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
    target_include_directories(host_libmacho PRIVATE
        ${CMAKE_SOURCE_DIR}/Kernel/xnu/osfmk
    )
endif()
