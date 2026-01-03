function(add_kext_bundle name)
    cmake_parse_arguments(SL "KERNEL_PRIVATE" "MACOSX_VERSION_MIN;INFO_PLIST;BUNDLE_IDENTIFIER;BUNDLE_VERSION;MAIN_FUNCTION;ANTIMAIN_FUNCTION" "" ${ARGN})

    if(SL_MACOSX_VERSION_MIN)
        add_darwin_shared_library(${name} MODULE MACOSX_VERSION_MIN ${SL_MACOSX_VERSION_MIN})
    else()
        add_darwin_shared_library(${name} MODULE)
    endif()

    set_property(TARGET ${name} PROPERTY PREFIX "")
    set_property(TARGET ${name} PROPERTY SUFFIX "")

    target_compile_definitions(${name} PRIVATE TARGET_OS_OSX KERNEL)
    target_compile_options(${name} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fapple-kext>
        --sysroot=${RAVYN_SDKROOT}
        -I${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Versions/A/PrivateHeaders
        -I${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Versions/A/Headers
        -I${ROOT_BINARY_DIR}/Kernel/xnu/bsd
        -I${RAVYN_SDKROOT}/usr/include
)

    target_link_options(${name} PRIVATE "LINKER:-bundle")
    target_link_options(${name} PRIVATE "SHELL:-undefined dynamic_lookup")
    target_link_options(${name} PRIVATE -Wl,-kext -Wl,-segalign,0x1000)

    if(SL_KERNEL_PRIVATE)
        target_compile_definitions(${name} PRIVATE KERNEL_PRIVATE)
        target_link_libraries(${name} PRIVATE ${CMAKE_BINARY_DIR}/Kernel/libkmod/libkmod.a)
    endif()

#    target_link_libraries(${name} PRIVATE xnu_kernel_headers)

    set_property(TARGET ${name} PROPERTY BUNDLE TRUE)
    set_property(TARGET ${name} PROPERTY BUNDLE_EXTENSION kext)

    if(SL_INFO_PLIST)
        get_filename_component(SL_INFO_PLIST ${SL_INFO_PLIST} ABSOLUTE)
        set_property(TARGET ${name} PROPERTY MACOSX_BUNDLE_INFO_PLIST ${SL_INFO_PLIST})
    else()
        message(SEND_ERROR "INFO_PLIST argument must be provided to add_darwin_kext()")
    endif()

    if(SL_BUNDLE_IDENTIFIER)
        set_property(TARGET ${name} PROPERTY MACOSX_BUNDLE_GUI_IDENTIFIER ${SL_BUNDLE_IDENTIFIER})
    endif()
    if(SL_BUNDLE_VERSION)
        set_property(TARGET ${name} PROPERTY MACOSX_BUNDLE_BUNDLE_VERSION ${SL_BUNDLE_VERSION})
    endif()

    add_kmod_info(${name} MAIN_FUNCTION ${SL_MAIN_FUNCTION} ANTIMAIN_FUNCTION ${SL_ANTIMAIN_FUNCTION})
endfunction()

function(add_kmod_info target)
    cmake_parse_arguments(KEXT "" "IDENTIFIER;VERSION;MAIN_FUNCTION;ANTIMAIN_FUNCTION" "" ${ARGN})

    if(NOT KEXT_IDENTIFIER)
        get_property(KEXT_IDENTIFIER TARGET ${target} PROPERTY MACOSX_BUNDLE_GUI_IDENTIFIER)
    endif()
    if(NOT KEXT_IDENTIFIER)
        message(SEND_ERROR "MACOSX_BUNDLE_GUI_IDENTIFIER is not set on kext target ${target}")
        return()
    endif()

    if(NOT KEXT_VERSION)
        get_property(KEXT_VERSION TARGET ${target} PROPERTY MACOSX_BUNDLE_BUNDLE_VERSION)
    endif()
    if(NOT KEXT_VERSION)
        message(SEND_ERROR "MACOSX_BUNDLE_BUNDLE_VERSION is not set on kext target ${target}")
        return()
    endif()

    if(KEXT_MAIN_FUNCTION)
        set(KEXT_MAIN_FUNCTION_DECL "extern kern_return_t ${KEXT_MAIN_FUNCTION}(kmod_info_t *ki, void *data);")
    else()
        set(KEXT_MAIN_FUNCTION "0")
    endif()

    if(KEXT_ANTIMAIN_FUNCTION)
        set(KEXT_ANTIMAIN_FUNCTION_DECL "extern kern_return_t ${KEXT_ANTIMAIN_FUNCTION}(kmod_info_t *ki, void *data);")
    else()
        set(KEXT_ANTIMAIN_FUNCTION "0")
    endif()

    configure_file(${ROOT_SOURCE_DIR}/Developer/cmake/templates/kmod_info.c.in ${CMAKE_CURRENT_BINARY_DIR}/kmod_info.c)
    target_sources(${target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/kmod_info.c)
    target_link_libraries(${target} PRIVATE ${ROOT_BINARY_DIR}/Kernel/libkmod/libkmod.a)
endfunction()
