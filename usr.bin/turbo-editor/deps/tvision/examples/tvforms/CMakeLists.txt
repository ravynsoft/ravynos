list(APPEND TVAPPS genparts genphone)

file(GLOB_RECURSE tvforms-src "${CMAKE_CURRENT_LIST_DIR}/*.cpp")
set(genform-src ${tvforms-src})
list(REMOVE_ITEM genform-src "${CMAKE_CURRENT_LIST_DIR}/tvforms.cpp")
list(REMOVE_ITEM tvforms-src "${CMAKE_CURRENT_LIST_DIR}/genform.cpp")

add_executable(genparts ${genform-src})
add_executable(genphone ${genform-src})

target_link_libraries(genparts PUBLIC ${PROJECT_NAME})
target_link_libraries(genphone PUBLIC ${PROJECT_NAME})

target_compile_definitions(genparts PRIVATE PARTS)
target_compile_definitions(genphone PRIVATE PHONENUM)

if (NOT CMAKE_CROSSCOMPILING)
    add_custom_command(TARGET genparts POST_BUILD COMMAND genparts WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    add_custom_command(TARGET genphone POST_BUILD COMMAND genphone WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

add_executable(tvforms ${tvforms-src})
target_link_libraries(tvforms PUBLIC ${PROJECT_NAME})

foreach(t tvforms genparts genphone)
    tv_app_common(${t})
endforeach()
