tv_app_add_generic(tvdemo)

if (WIN32)
    target_link_libraries(tvdemo PRIVATE
        psapi # Required by 'GetProcessMemoryInfo' (gadgets.cpp)
    )
endif()
