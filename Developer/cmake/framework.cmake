function(add_framework_bundle name)
    cmake_parse_arguments(SL "" "VERSION;INFO_PLIST;INSTALL_FILE_NAME;INSTALL_NAME_DIR" "RPATHS;RESOURCES_DIRS;RESOURCES_FILES" ${ARGN})

    add_library(${name} SHARED)
    set_target_properties(${name} PROPERTIES PREFIX "" OUTPUT_NAME ${name} SUFFIX "" NO_SONAME true)
    add_dependencies(Foundation BUILD_PHASE_FRAMEWORKS objc)
    target_compile_options(${name} PRIVATE -mmacos-version-min=${MACOS_VERSION_MIN})
    target_link_options(${name} PRIVATE -mmacos-version-min=${MACOS_VERSION_MIN})

    if(SL_INSTALL_NAME_DIR)
        target_link_options(${name} PRIVATE -install_name "${SL_INSTALL_NAME_DIR}/$<TARGET_FILE_NAME:${name}>")
    elseif(SL_INSTALL_NAME)
        target_link_options(${name} PRIVATE -install_name ${SL_INSTALL_NAME})
    endif()

    foreach(rpath IN LISTS SL_RPATHS)
        target_link_options(${name} PRIVATE "SHELL:-rpath ${rpath}")
    endforeach()

    foreach(resource IN LISTS SL_RESOURCES_DIRS)
        cmake_path(REMOVE_FILENAME resource OUTPUT_VARIABLE path)
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${path}
            DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/${name}.framework/Versions/${SL_VERSION}/Resources/${path}/
            FILES_MATCHING PATTERN ${resource})
    endforeach()

    foreach(resource IN LISTS SL_RESOURCES_FILES)
        cmake_path(REMOVE_FILENAME resource OUTPUT_VARIABLE path)
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${path}
            DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/${name}.framework/Versions/${SL_VERSION}/Resources/
            FILES_MATCHING PATTERN ${resource})
    endforeach()

    foreach(hdr IN LISTS SL_HEADERS)
        cmake_path(REMOVE_FILENAME hdr OUTPUT_VARIABLE path)
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${path}
            DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/${name}.framework/Versions/${SL_VERSION}/Headers/
            FILES_MATCHING PATTERN ${hdr})
    endforeach()

    foreach(hdr IN LISTS SL_PRIVATE_HEADERS)
        cmake_path(REMOVE_FILENAME hdr OUTPUT_VARIABLE path)
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${path}
            DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/${name}.framework/Versions/${SL_VERSION}/PrivateHeaders/
            FILES_MATCHING PATTERN ${hdr})
    endforeach()

    if(SL_INFO_PLIST)
        get_filename_component(SL_INFO_PLIST ${SL_INFO_PLIST} ABSOLUTE)
        set_property(TARGET ${name} PROPERTY MACOSX_BUNDLE_INFO_PLIST ${SL_INFO_PLIST})
    else()
        message(SEND_ERROR "INFO_PLIST argument must be provided to add_framework_bundle()")
    endif()

    add_custom_command(OUTPUT ${name}_Bundle SYMBOLIC
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/${name}.framework/Versions/${SL_VERSION}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/${name}.framework/Versions/${SL_VERSION}/Resources
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/${name}.framework/Versions/${SL_VERSION}/Headers
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/${name}.framework/Versions/${SL_VERSION}/PrivateHeaders
        COMMAND ${CMAKE_COMMAND} -E create_symlink ${SL_VERSION} ${CMAKE_CURRENT_BINARY_DIR}/${name}.framework/Versions/Current
        COMMAND ${CMAKE_COMMAND} -E create_symlink Versions/Current/Headers ${CMAKE_CURRENT_BINARY_DIR}/${name}.framework/Headers
        COMMAND ${CMAKE_COMMAND} -E create_symlink Versions/Current/PrivateHeaders ${CMAKE_CURRENT_BINARY_DIR}/${name}.framework/PrivateHeaders
        COMMAND ${CMAKE_COMMAND} -E create_symlink Versions/Current/Resources ${CMAKE_CURRENT_BINARY_DIR}/${name}.framework/Resources
        COMMAND ${CMAKE_COMMAND} -E create_symlink Versions/Current/${name} ${CMAKE_CURRENT_BINARY_DIR}/${name}.framework/${name}
    )
    target_sources(${name} PRIVATE ${name}_Bundle)

    add_custom_command(TARGET ${name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${name}> ${CMAKE_CURRENT_BINARY_DIR}/${name}.framework/Versions/${SL_VERSION}/
        COMMAND ${CMAKE_COMMAND} -E copy ${SL_INFO_PLIST} ${CMAKE_CURRENT_BINARY_DIR}/${name}.framework/Versions/${SL_VERSION}/Resources/Info.plist
    )
endfunction()
