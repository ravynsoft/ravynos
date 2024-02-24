add_library(microatf-c atf-c.c)
target_include_directories(microatf-c
                           PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>)

add_executable(microatf-translate-signal translate-signal.c)
