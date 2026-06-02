# To be converted from cmake
# if(NOTYET)
# 
#     # ----------------------------------------------------------------------------
#     #                           libdns_services.dylib
#     # ----------------------------------------------------------------------------
# 
#     add_library(dns_services SHARED
#         ${ROOT_SOURCE_DIR}/BSD/usr.sbin/mDNSResponder/mDNSMacOSX/Private/dns_services.c
#         ${ROOT_SOURCE_DIR}/BSD/usr.sbin/mDNSResponder/mDNSMacOSX/dnssd.c
#         ${ROOT_SOURCE_DIR}/BSD/usr.sbin/mDNSResponder/mDNSMacOSX/dnssd_object.m
#         ${ROOT_SOURCE_DIR}/BSD/usr.sbin/mDNSResponder/mDNSMacOSX/dnssd_xpc.c)
# 
#     set_target_properties(dns_services PROPERTIES CXX_STANDARD 14 C_STANDARD 11
#         PREFIX "lib" OUTPUT_NAME "dns_services" SUFFIX ".dylib" NO_SONAME true)
# 
#     target_include_directories(dns_services PRIVATE
#         ${ROOT_SOURCE_DIR}/BSD/usr.sbin/mDNSResponder/mDNSMacOSX/xpc_services
#         ${ROOT_SOURCE_DIR}/BSD/usr.sbin/mDNSResponder/mDNSMacOSX
#         ${ROOT_SOURCE_DIR}/Libraries/openbsm
#         ${ROOT_SOURCE_DIR}/Kernel/xnu/libkern
#         ${RAVYN_SDKROOT}/System/Library/PrivateFrameworks/CoreUtils.framework/Versions/A/Headers)
# 
#     target_link_options(dns_services PRIVATE
#         -Wl,-install_name,/usr/lib/libdns_services.dylib
#         -Wl,-current_version,1 -Wl,-compatibility_version,1)
# 
#     # ----------------------------------------------------------------------------
#     #                        liblog_mdnsresponder.dylib
#     # ----------------------------------------------------------------------------
# 
#     add_library(liblog_mdnsresponder STATIC
#         ${ROOT_SOURCE_DIR}/BSD/usr.sbin/mDNSResponder/mDNSMacOSX/LoggingProfiles/liblog_mdnsresponder.m
#         ${ROOT_SOURCE_DIR}/BSD/usr.sbin/mDNSResponder/Clients/dnssdutil/DNSMessage.c)
# 
#     set_target_properties(liblog_mdnsresponder PROPERTIES CXX_STANDARD 14 C_STANDARD 11
#         PREFIX "liblog_" OUTPUT_NAME "mdnsresponder" SUFFIX ".dylib" NO_SONAME true)
# 
#     target_include_directories(liblog_mdnsresponder PRIVATE
#         ${ROOT_SOURCE_DIR}/Frameworks
#         ${ROOT_SOURCE_DIR}/Frameworks/Foundation/Headers
#         ${ROOT_SOURCE_DIR}/Kernel/xnu/libkern
#         ${ROOT_SOURCE_DIR}/Libraries/Libsystem/libdispatch
#         ${ROOT_SOURCE_DIR}/BSD/usr.sbin/mDNSResponder/Clients/dnssdutil
#         ${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders)
# 
#     target_link_options(liblog_mdnsresponder PRIVATE
#         -Wl,-install_name,/usr/lib/log/liblog_mdnsresponder.dylib
#         -Wl,-current_version,1 -Wl,-compatibility_version,1)
# 
# else()
#     add_custom_target(liblog_mdnsresponder)
#     add_custom_target(dns_services)
# endif() # NOTYET
