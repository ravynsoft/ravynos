add_library(host_libmacho STATIC)
target_sources(host_libmacho PRIVATE
    arch.c
)
set_property(TARGET host_libmacho PROPERTY OUTPUT_NAME macho)
