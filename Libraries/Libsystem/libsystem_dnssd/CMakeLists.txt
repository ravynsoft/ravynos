add_compile_definitions(APPLE_OSX_mDNSResponder _DARWIN_C_SOURCE)

add_compile_options(-fno-common -Wno-incompatible-sysroot
    -Wno-nullability-completeness
    -mmacos-version-min=${MACOS_VERSION_MIN} -isysroot${RAVYN_SDKROOT})

include_directories(
    .
    ${RAVYN_SDKROOT}/usr/local/include/kernel ${RAVYN_SDKROOT}/usr/local/include
    ${ROOT_SOURCE_DIR}/BSD/usr.sbin/mDNSResponder/mDNSShared
    ${ROOT_SOURCE_DIR}/Libraries/Libsystem/liblaunch)

add_link_options(-mmacos-version-min=${MACOS_VERSION_MIN}
    -Wl,-all_load -Wl,-syslibroot,${RAVYN_SDKROOT}/usr/lib)

add_library(libsystem_dnssd SHARED
    ${ROOT_SOURCE_DIR}/BSD/usr.sbin/mDNSResponder/mDNSShared/dnssd_ipc.c
	${ROOT_SOURCE_DIR}/BSD/usr.sbin/mDNSResponder/mDNSShared/dnssd_clientlib.c
	${ROOT_SOURCE_DIR}/BSD/usr.sbin/mDNSResponder/mDNSMacOSX/DNSServiceDiscovery.c
	${ROOT_SOURCE_DIR}/BSD/usr.sbin/mDNSResponder/mDNSShared/dnssd_clientstub.c
)

set_target_properties(libsystem_dnssd PROPERTIES CXX_STANDARD 14 C_STANDARD 11
    PREFIX "libsystem_" OUTPUT_NAME "dnssd" SUFFIX ".dylib" NO_SONAME true)

target_compile_definitions(libsystem_dnssd PRIVATE __DARWIN_NON_CANCELABLE)
target_include_directories(libsystem_dnssd PRIVATE
    ${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders
    ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Versions/A/Headers)

target_link_options(libsystem_dnssd PRIVATE
    -umbrella System -L${RAVYN_SDKROOT}/usr/lib/system
    -L${RAVYN_SDKROOT}/usr/lib
    -Wl,-install_name,/usr/lib/system/libsystem_dnssd.dylib
    -Wl,-current_version,1096.100.3 -Wl,-compatibility_version,1
    -lsystem_platform -lsystem_asl)

add_custom_command(TARGET libsystem_dnssd POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libsystem_dnssd>
		${RAVYN_SDKROOT}/usr/lib/system/
)



add_library(dns_services SHARED
	${ROOT_SOURCE_DIR}/BSD/usr.sbin/mDNSResponder/mDNSMacOSX/Private/dns_services.c
	${ROOT_SOURCE_DIR}/BSD/usr.sbin/mDNSResponder/mDNSMacOSX/dnssd.c
	${ROOT_SOURCE_DIR}/BSD/usr.sbin/mDNSResponder/mDNSMacOSX/dnssd_object.m
	${ROOT_SOURCE_DIR}/BSD/usr.sbin/mDNSResponder/mDNSMacOSX/dnssd_xpc.c)

set_target_properties(dns_services PROPERTIES CXX_STANDARD 14 C_STANDARD 11
    PREFIX "lib" OUTPUT_NAME "dns_services" SUFFIX ".dylib" NO_SONAME true)

target_include_directories(dns_services PRIVATE
    ${ROOT_SOURCE_DIR}/BSD/usr.sbin/mDNSResponder/mDNSMacOSX/xpc_services
    ${ROOT_SOURCE_DIR}/BSD/usr.sbin/mDNSResponder/mDNSMacOSX
    ${ROOT_SOURCE_DIR}/Libraries/openbsm
    ${ROOT_SOURCE_DIR}/Developer/InternalSDK
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libkern/)

target_link_options(dns_services PRIVATE
    -Wl,-install_name,/usr/lib/libdns_services.dylib
    -Wl,-current_version,1 -Wl,-compatibility_version,1)




add_library(liblog_mdnsresponder STATIC
    ${ROOT_SOURCE_DIR}/BSD/usr.sbin/mDNSResponder/mDNSMacOSX/LoggingProfiles/liblog_mdnsresponder.m
	${ROOT_SOURCE_DIR}/BSD/usr.sbin/mDNSResponder/Clients/dnssdutil/DNSMessage.c)

set_target_properties(liblog_mdnsresponder PROPERTIES CXX_STANDARD 14 C_STANDARD 11
    PREFIX "liblog_" OUTPUT_NAME "mdnsresponder" SUFFIX ".dylib" NO_SONAME true)

target_include_directories(liblog_mdnsresponder PRIVATE
    ${ROOT_SOURCE_DIR}/Frameworks
    ${ROOT_SOURCE_DIR}/Frameworks/Foundation/Headers
    ${ROOT_SOURCE_DIR}/Developer/InternalSDK
    ${ROOT_SOURCE_DIR}/Libraries/Libsystem/libdispatch)

target_link_options(liblog_mdnsresponder PRIVATE
    -Wl,-install_name,/usr/lib/log/liblog_mdnsresponder.dylib
    -Wl,-current_version,1 -Wl,-compatibility_version,1)
