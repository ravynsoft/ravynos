function(add_kext_bundle name)
    cmake_parse_arguments(SL "KERNEL_PRIVATE" "MACOSX_VERSION_MIN;INFO_PLIST;BUNDLE_IDENTIFIER;BUNDLE_VERSION;MAIN_FUNCTION;ANTIMAIN_FUNCTION" "" ${ARGN})

    add_library(${name} SHARED)

    string(REGEX REPLACE "^lib" "" libname ${name})
    string(REGEX REPLACE "\.kext\$" "" libname ${libname})
    set(bname ${libname})
    string(APPEND bname ".kext")

    set_target_properties(${name} PROPERTIES
        SUFFIX "" PREFIX "" OUTPUT_NAME "${libname}"
        OSX_ARCHITECTURES ${CpuArch}
        CMAKE_OSX_DEPLOYMENT_TARGET ${CMAKE_MACOSX_MIN_VERSION}
        NO_SONAME true BUILD_WITH_INSTALL_NAME_DIR false)

    target_compile_definitions(${name} PRIVATE KERNEL)
    target_compile_options(${name} PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:-fapple-kext>
        --sysroot=${RAVYN_SDKROOT}
        -I${RAVYN_SDKROOT}/usr/include
        -I${RAVYN_SDKROOT}/usr/local/include
        -I${RAVYN_SDKROOT}/usr/local/include/kernel)
    if(SL_INSTALL_NAME_DIR)
        target_link_options(${name} PRIVATE -install_name "${SL_INSTALL_NAME_DIR}/$<TARGET_FILE_NAME:${name}>")
    elseif(SL_INSTALL_NAME)
        target_link_options(${name} PRIVATE -install_name ${SL_INSTALL_NAME})
    endif()

    foreach(rpath IN LISTS SL_RPATHS)
        target_link_options(${name} PRIVATE "SHELL:-rpath ${rpath}")
    endforeach()

    target_link_options(${name} PRIVATE -nostdlib
        -Wl,-bundle -Wl,-undefined,dynamic_lookup
        -Wl,-kext -Wl,-segalign,0x1000)

    if(SL_KERNEL_PRIVATE)
        target_compile_definitions(${name} PRIVATE KERNEL_PRIVATE)
        target_link_libraries(${name} PRIVATE ${ROOT_BINARY_DIR}/Kernel/libkmod/libkmod.a)
    endif()

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

    add_custom_command(TARGET ${name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/${bname}/Contents/MacOS
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${name}> ${CMAKE_CURRENT_BINARY_DIR}/${bname}/Contents/MacOS/
        COMMAND ${CMAKE_COMMAND} -E copy ${SL_INFO_PLIST} ${CMAKE_CURRENT_BINARY_DIR}/${bname}/Contents/Info.plist
    )
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
