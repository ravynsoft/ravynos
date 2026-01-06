cmake_policy(SET CMP0114 NEW)

add_subdirectory(libmacho)
add_subdirectory(libstuff)
add_subdirectory(misc)
# add_subdirectory(otool) # disabled - doesn't build
add_subdirectory(ld64/src)
