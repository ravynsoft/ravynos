if($ENV{CI})
    execute_process(OUTPUT_VARIABLE BUILD_STAMP
        COMMAND /bin/sh -c "echo ${CIRRUS_CHANGE_IN_REPO}|cut -c1-7|tr '[a-z]' '[A-Z]'"
    )
else()
    execute_process(OUTPUT_VARIABLE BUILD_STAMP
        COMMAND /bin/sh -c "cd ${CMAKE_SOURCE_DIR} && git log -1|head -1|cut -c8-14|tr '[a-z]' '[A-Z]'"
    )
endif()

string(STRIP "${BUILD_STAMP}" BUILD_STAMP)
file(MAKE_DIRECTORY "${SYSROOT_DIR}/System/Library/CoreServices")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/SystemVersion.plist.in
    ${SYSROOT_DIR}/System/Library/CoreServices/SystemVersion.plist
    @ONLY
)
