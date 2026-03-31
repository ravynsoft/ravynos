#add_library(dns_services STATIC
#	./Private/dns_services.c
#	./dnssd.c
#	./dnssd_object.m
#	./dnssd_xpc.c
#)

#add_framework(DomainBrowser
#    VERSION "A"     # FIXME: check this against Catalina
#    INFO_PLIST DomainBrowser/macOS/Info.plist
#    INSTALL_FILE_DIR /System/Library/Frameworks/DomainBrowser.framework/Versions/A
#    SOURCES
#        ./DomainBrowser/Shared/CNDomainBrowserPathUtils.m
#        ./DomainBrowser/Shared/_CNDomainBrowser.m
#        ./DomainBrowser/macOS/CNDomainBrowserView.m
#         ClientCommon.c
#)


#add_executable(mDNSResponder
#	./D2D.c
#	./coreBLE.m
#	./dnssd_xpc.c
#	 dso-transport.c
#	 mDNS.c
#	./DNS64.c
#	./dnssd_server.c
#	 uDNS.c
#	 DNSCommon.c
#	 DNSDigest.c
#	 mDNSDebug.c
#	./mdns.c
#	 dso.c
#	./mdns_object.m
#	./xpc_services/xpc_service_dns_proxy.c
#	 uds_daemon.c
#	./BLE.c
#	./xpc_services/xpc_services.c
#	 dnssd_ipc.c
#	 PlatformCommon.c
#	./mDNSMacOSX.c
#	./Metrics.m
#	./LegacyNATTraversal.c
#	./daemon.c
#	./helper-stubs.c
#	 dnssd_clientshim.c
#	 CryptoAlg.c
#	 ClientRequests.c
#	./CryptoSupport.c
#	 nsec.c
#	 dnssec.c
#	 dnsproxy.c
#	 posix_utilities.c
#	./DNSProxySupport.c
#	./uDNSPathEvalulation.c
#	./SymptomReporter.c
#	 nsec3.c
#	./utilities/system_utilities.c
#	./DNSSECSupport.c
#	./xpc_services/xpc_service_log_utility.c
#)
#set_source_files_properties(./mdns_object.m TARGET_DIRECTORY mDNSResponder PROPERTIES COMPILE_OPTIONS "-fno-objc-arc")

#add_executable(dnsextd
#	 dnssd_clientshim.c
#	 dso-transport.c
#	 dso.c
#	 CryptoAlg.c
#	 DNSCommon.c
#	 DNSDigest.c
#	 dnsextd.c
#	 mDNSDebug.c
#	 GenLinkedList.c
#	./mDNSMacOSX.c
#	 dnsextd_parser.y
#	 dnsextd_lexer.l
#	 PlatformCommon.c
#	./helper-stubs.c
#)
#set_source_files_properties( dnsextd_lexer.l TARGET_DIRECTORY dnsextd PROPERTIES COMPILE_OPTIONS "-Wno-error")

#add_library(PreferencePane STATIC
#	./PreferencePane/DNSServiceDiscoveryPref.m
#	./DomainBrowser/Shared/CNDomainBrowserPathUtils.m
#	./DomainBrowser/macOS/CNDomainBrowserView.m
#	./SettingsBundle/BonjourSCStore.m
#	 ClientCommon.c
#	./DomainBrowser/Shared/_CNDomainBrowser.m
#)
