
add_definitions(${DBUS_INTERNAL_CLIENT_DEFINITIONS})

include_directories(.)

set(DBUS_SESSION_BUS_LISTEN_ADDRESS ${TEST_LISTEN})

add_library(dbus-testutils STATIC
    disable-crash-handling.c
    disable-crash-handling.h
    test-utils.h
    test-utils.c
)
target_link_libraries(dbus-testutils ${DBUS_INTERNAL_LIBRARIES})

if(DBUS_ENABLE_EMBEDDED_TESTS)
    add_subdirectory( name-test )
endif()

set(manual-dir-iter_SOURCES
    manual-dir-iter.c
)

set(test-service_SOURCES
    test-service.c
)

set(test-names_SOURCES
    test-names.c
)

set(break_loader_SOURCES
    break-loader.c
)

set(test-shell-service_SOURCES
    test-shell-service.c
)

set(test-shell_SOURCES
    shell-test.c
)

set(test-atomic_SOURCES
    internals/atomic.c
)

set(test-spawn_SOURCES
    spawn-test.c
)

set(test-exit_SOURCES
    test-exit.c
)

# We have to compile a separate copy of disable-crash-handling.c for
# test-segfault rather than using the libdbus-testutils library, because
# otherwise it would fail to link when using the AddressSanitizer.
set(test-segfault_SOURCES
    disable-crash-handling.c
    disable-crash-handling.h
    test-segfault.c
)

set(test-sleep-forever_SOURCES
    test-sleep-forever.c
)

set(manual-tcp_SOURCES
    manual-tcp.c
)

set(manual-paths_SOURCES
    manual-paths.c
)

add_helper_executable(manual-dir-iter ${manual-dir-iter_SOURCES} ${DBUS_INTERNAL_LIBRARIES})
add_helper_executable(test-service ${test-service_SOURCES} dbus-testutils)
add_helper_executable(test-names ${test-names_SOURCES} dbus-testutils)
add_test_executable(test-shell ${test-shell_SOURCES} ${DBUS_INTERNAL_LIBRARIES})
add_test_executable(test-string internals/strings.c dbus-testutils)
add_test_executable(test-printf internals/printf.c dbus-testutils)
add_helper_executable(test-privserver test-privserver.c dbus-testutils)
add_helper_executable(test-shell-service ${test-shell-service_SOURCES} dbus-testutils)
if(NOT WINCE AND ENABLE_TRADITIONAL_ACTIVATION)
    add_test_executable(test-spawn-oom internals/spawn-oom.c dbus-testutils)
endif()
if(ENABLE_TRADITIONAL_ACTIVATION)
    add_helper_executable(test-spawn ${test-spawn_SOURCES} ${DBUS_INTERNAL_LIBRARIES})
endif()
add_helper_executable(test-exit ${test-exit_SOURCES} ${DBUS_INTERNAL_LIBRARIES})
# the second argument of add_helper_executable() is a whitespace-separated
# list of source files and the third and subsequent arguments are libraries
# to link, hence the quoting here
add_helper_executable(test-segfault "${test-segfault_SOURCES}")
add_helper_executable(test-sleep-forever ${test-sleep-forever_SOURCES} ${DBUS_INTERNAL_LIBRARIES})
add_helper_executable(manual-tcp ${manual-tcp_SOURCES} ${DBUS_INTERNAL_LIBRARIES})
add_helper_executable(manual-backtrace manual-backtrace.c dbus-1)
if(WIN32)
    add_helper_executable(manual-paths ${manual-paths_SOURCES} dbus-testutils)
endif()

if(DBUS_ENABLE_EMBEDDED_TESTS)
    add_test_executable(test-atomic ${test-atomic_SOURCES} dbus-testutils)
    add_test_executable(test-hash internals/hash.c dbus-testutils)
    set_target_properties(test-hash PROPERTIES COMPILE_FLAGS ${DBUS_INTERNAL_CLIENT_DEFINITIONS})

    set(SOURCES
        internals/dbus-marshal-recursive-util.c
        internals/dbus-marshal-recursive-util.h
        internals/marshal-recursive.c
    )
    add_test_executable(test-marshal-recursive "${SOURCES}" dbus-testutils)
    set_target_properties(test-marshal-recursive PROPERTIES COMPILE_FLAGS ${DBUS_INTERNAL_CLIENT_DEFINITIONS})

    set(SOURCES
        internals/dbus-marshal-recursive-util.c
        internals/dbus-marshal-recursive-util.h
        internals/dbus-message-factory.c
        internals/dbus-message-factory.h
        internals/dbus-message-util.c
        internals/dbus-message-util.h
        internals/message-internals.c
    )
    add_test_executable(test-message-internals "${SOURCES}" dbus-testutils)
    set_target_properties(test-message-internals PROPERTIES COMPILE_FLAGS ${DBUS_INTERNAL_CLIENT_DEFINITIONS})

    set(SOURCES
        internals/address.c
        internals/dbus-auth-script.c
        internals/dbus-auth-script.h
        internals/dbus-auth-util.c
        internals/dbus-credentials-util.c
        internals/dbus-marshal-byteswap-util.c
        internals/dbus-marshal-recursive-util.c
        internals/dbus-marshal-recursive-util.h
        internals/dbus-marshal-validate-util.c
        internals/dbus-string-util.c
        internals/dbus-sysdeps-util.c
        internals/mempool.c
        internals/misc-internals.c
        internals/misc-internals.h
        internals/sha.c
    )
    add_test_executable(test-misc-internals "${SOURCES}" dbus-testutils)
    set_target_properties(test-misc-internals PROPERTIES COMPILE_FLAGS ${DBUS_INTERNAL_CLIENT_DEFINITIONS})

    set(SOURCES bus/main.c)
    add_test_executable(test-bus "${SOURCES}" dbus-daemon-internal dbus-testutils ${EXPAT_LIBRARIES})
    set_target_properties(test-bus PROPERTIES COMPILE_FLAGS ${DBUS_INTERNAL_CLIENT_DEFINITIONS})

    set(SOURCES bus/dispatch.c)
    add_test_executable(test-bus-dispatch "${SOURCES}" dbus-daemon-internal dbus-testutils ${EXPAT_LIBRARIES})
    set_target_properties(test-bus-dispatch PROPERTIES COMPILE_FLAGS ${DBUS_INTERNAL_CLIENT_DEFINITIONS})

    set(SOURCES bus/dispatch-sha1.c)
    add_test_executable(test-bus-dispatch-sha1 "${SOURCES}" dbus-daemon-internal dbus-testutils ${EXPAT_LIBRARIES})
    set_target_properties(test-bus-dispatch-sha1 PROPERTIES COMPILE_FLAGS ${DBUS_INTERNAL_CLIENT_DEFINITIONS})

    if(NOT WIN32)
        add_test_executable(test-bus-system bus/system.c launch-helper-internal dbus-testutils)
        add_test_executable(test-counter internals/counter.c dbus-testutils)
        if(ENABLE_TRADITIONAL_ACTIVATION)
            add_test_executable(test-bus-launch-helper-oom bus/launch-helper-oom.c launch-helper-internal dbus-testutils)
            add_helper_executable(dbus-daemon-launch-helper-for-tests bus/launch-helper-for-tests.c launch-helper-internal)
        endif()
    endif()
endif()

if(DBUS_WITH_GLIB)
    message(STATUS "with glib test apps")

    add_library(dbus-testutils-glib STATIC
        test-utils-glib.h
        test-utils-glib.c
    )
    target_link_libraries(dbus-testutils-glib dbus-testutils ${DBUS_INTERNAL_LIBRARIES})

    add_definitions(
        ${GLIB2_DEFINITIONS}
    )
    include_directories(
        ${GLIB2_INCLUDE_DIR}
    )

    set(TEST_LIBRARIES ${DBUS_INTERNAL_LIBRARIES} dbus-testutils dbus-testutils-glib ${GLIB2_LIBRARIES})

    add_test_executable(test-assertions internals/assertions.c ${TEST_LIBRARIES})
    add_test_executable(test-corrupt corrupt.c ${TEST_LIBRARIES})
    add_test_executable(test-dbus-daemon dbus-daemon.c ${TEST_LIBRARIES})
    add_test_executable(test-dbus-daemon-eavesdrop dbus-daemon-eavesdrop.c ${TEST_LIBRARIES})
    add_test_executable(test-desktop-file internals/desktop-file.c ${TEST_LIBRARIES})
    add_test_executable(test-fdpass fdpass.c ${TEST_LIBRARIES})
    add_test_executable(test-loopback loopback.c ${TEST_LIBRARIES})
    add_test_executable(test-marshal marshal.c ${TEST_LIBRARIES})
    add_test_executable(test-monitor monitor.c ${TEST_LIBRARIES})
    add_test_executable(test-refs internals/refs.c ${TEST_LIBRARIES})
    add_test_executable(test-relay relay.c ${TEST_LIBRARIES})
    add_test_executable(test-server-oom internals/server-oom.c ${TEST_LIBRARIES})
    add_test_executable(test-syntax syntax.c ${TEST_LIBRARIES})
    add_test_executable(test-sysdeps internals/sysdeps.c ${TEST_LIBRARIES})
    add_test_executable(test-syslog internals/syslog.c ${TEST_LIBRARIES})
    add_test_executable(test-uid-permissions uid-permissions.c ${TEST_LIBRARIES})
    add_test_executable(test-userdb internals/userdb.c ${TEST_LIBRARIES})
    add_helper_executable(manual-authz manual-authz.c ${TEST_LIBRARIES})
    add_helper_executable(manual-test-thread-blocking thread-blocking.c ${TEST_LIBRARIES})
endif()

### keep these in creation order, i.e. uppermost dirs first 
set(TESTDIRS
    data
    data/invalid-messages
    data/auth
    data/sha-1
    data/systemd-activation
    data/valid-config-files
    data/valid-config-files/basic.d
    data/valid-config-files/session.d
    data/valid-config-files-system
    data/valid-config-files-system/system.d
    data/valid-messages
    data/valid-service-files
    data/valid-service-files-system
    data/invalid-config-files
    data/invalid-config-files-system
    data/invalid-messages
    data/invalid-service-files-system
    data/equiv-config-files
    data/equiv-config-files/basic
    data/equiv-config-files/basic/basic.d
    data/equiv-config-files/entities
    data/equiv-config-files/entities/basic.d
)

foreach(DIR ${TESTDIRS})
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${DIR})
endforeach()

### copy tests to builddir so that generated tests and static tests 
### are all in one place.
if(NOT ENABLE_VERBOSE_CONFIG)
    set(HINT " (use -DENABLE_VERBOSE_CONFIG=ON to see the list of files)")
endif()

message(STATUS "Copying test files to test directory${HINT}")

macro(dbus_configure_file _src _dest)
    configure_file(${_src} ${_dest} ${ARGN})
    if(ENABLE_VERBOSE_CONFIG)
        message("   generating ${_dest}")
    endif()
endmacro()

foreach(FILE_TYPE *.message-raw *.auth-script *.sha1 *.txt *.conf *.service)
    foreach(DIR ${TESTDIRS})
        file(GLOB FILES "${DIR}/${FILE_TYPE}" )
        foreach(FILE ${FILES})
            get_filename_component(FILENAME ${FILE} NAME)
            set(TARGET ${CMAKE_CURRENT_BINARY_DIR}/${DIR}/${FILENAME})
            dbus_configure_file(${FILE} ${TARGET} COPYONLY)
        endforeach()
    endforeach()
endforeach()

### generate test files
message(STATUS "Generating test files from templates into test directory${HINT}")

foreach(FILE_TYPE *.conf.in *.service.in)
    foreach(DIR ${TESTDIRS})
    file(GLOB FILES "${DIR}/${FILE_TYPE}" )
    foreach(FILE ${FILES})
        get_filename_component(FILENAME ${FILE} NAME)
        string(REGEX REPLACE "\\.in$" "" FILENAME ${FILENAME})
        set(TARGET ${CMAKE_CURRENT_BINARY_DIR}/${DIR}/${FILENAME})
        dbus_configure_file(${FILE} ${TARGET} @ONLY IMMEDIATE)
    endforeach()
    endforeach()
endforeach()

message(STATUS "Copying generated bus config files to test directory${HINT}")
dbus_configure_file(../bus/session.conf.in ${CMAKE_BINARY_DIR}/test/data/valid-config-files/session.conf @ONLY)
dbus_configure_file(../bus/system.conf.in ${CMAKE_BINARY_DIR}/test/data/valid-config-files-system/system.conf @ONLY)
