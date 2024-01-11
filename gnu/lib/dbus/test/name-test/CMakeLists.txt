add_definitions(${DBUS_INTERNAL_CLIENT_DEFINITIONS})

add_helper_executable(test-autolaunch test-autolaunch.c dbus-testutils)

add_session_test_executable(test-ids test-ids.c ${DBUS_INTERNAL_LIBRARIES})
add_session_test_executable(test-pending-call-disconnected test-pending-call-disconnected.c ${DBUS_INTERNAL_LIBRARIES})

if(ENABLE_TRADITIONAL_ACTIVATION)
    add_session_test_executable(test-pending-call-dispatch test-pending-call-dispatch.c ${DBUS_INTERNAL_LIBRARIES})
    add_session_test_executable(test-pending-call-timeout test-pending-call-timeout.c ${DBUS_INTERNAL_LIBRARIES})
endif()
add_session_test_executable(test-shutdown test-shutdown.c dbus-testutils)
if(ENABLE_TRADITIONAL_ACTIVATION)
    add_session_test_executable(test-privserver-client test-privserver-client.c dbus-testutils)
    add_session_test_executable(test-thread-init test-threads-init.c ${DBUS_INTERNAL_LIBRARIES})
endif()
