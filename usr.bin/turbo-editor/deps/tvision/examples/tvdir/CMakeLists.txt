tv_app_add_generic(tvdir)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(tvdir PRIVATE -Wno-stringop-truncation)
endif()
