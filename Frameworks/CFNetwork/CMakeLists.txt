add_framework_bundle(CFNetwork
    VERSION A
    INFO_PLIST Info.plist
    RESOURCES_DIRS English.lproj/*
    RESOURCES_FILES PACSupport.js
    HEADERS CFFTPStream.h CFHost.h CFHTTPAuthentication.h CFHTTPMessage.h CFHTTPStream.h
            CFNetDiagnostics.h CFNetServices.h CFNetwork.h CFNetworkDefs.h CFSocketStream.h
    PRIVATE_HEADERS CFFTPStreamPriv.h CFHostPriv.h CFHTTPConnectionPriv.h CFHTTPMessagePriv.h
                    CFHTTPServerPriv.h CFHTTPStreamPriv.h CFNetDiagnosticsPriv.h CFNetServicesPriv.h
                    CFNetworkPriv.h CFServerPriv.h CFSocketStreamPriv.h
    INSTALL_NAME_DIR /System/Library/Frameworks/Foundation.framework/Versions/C
)

target_sources(CFNetwork PRIVATE
    ./CFNetwork.c
    ./FTP/CFFTPStream.c
    ./Host/CFHost.c
    ./HTTP/CFHTTPAuthentication.c
    ./HTTP/CFHTTPConnection.c
    ./HTTP/CFHTTPFilter.c
    ./HTTP/CFHTTPMessage.c
    ./HTTP/CFHTTPServer.c
    ./HTTP/CFHTTPStream.c
    ./HTTP/NTLM/ntlmBlobPriv.cpp
    ./HTTP/NTLM/NtlmGenerator.cpp
    ./HTTP/SPNEGO/spnegoBlob.cpp
    ./HTTP/SPNEGO/spnegoDER.cpp
    ./HTTP/SPNEGO/spnegoKrb.cpp
    ./JavaScriptGlue.c
    ./libresolv.c
    ./NetDiagnostics/CFNetDiagnosticPing.c
    ./NetDiagnostics/CFNetDiagnostics.c
    ./NetDiagnostics/CFNetDiagnosticsProtocolUser.c
    ./NetServices/CFNetServiceBrowser.c
    ./NetServices/CFNetServiceMonitor.c
    ./NetServices/CFNetServices.c
    ./NetServices/DeprecatedDNSServiceDiscovery.c
    ./Proxies/ProxySupport.c
    ./security_cdsa_utils.c
    ./SharedCode/CFNetConnection.c
    ./SharedCode/CFNetworkSchedule.c
    ./SharedCode/CFNetworkThreadSupport.c
    ./SharedCode/CFServer.c
    ./Stream/CFSocketStream.c
    ./URL/_CFURLAccess.c
)
target_compile_definitions(CFNetwork PRIVATE )
target_compile_options(CFNetwork PRIVATE -isysroot${RAVYN_SDKROOT} -Wno-implicit-function-declaration
OTHER_CFLAGS += -F/System/Library/PrivateFrameworks -F/usr/local/SecurityPieces/Frameworks
OTHER_CPPFLAGS += -F/usr/local/SecurityPieces/Frameworks
OTHER_LFLAGS += -framework CoreFoundation -framework Security -framework SystemConfiguration -F/usr/local/SecurityPieces/Frameworks -framework security_cdsa_utils
ifeq "$(PLATFORM)" "Darwin"
CFLAGS += -F/System/Library/Frameworks/CoreServices.framework/Frameworks
CPPFLAGS += -F/System/Library/Frameworks/CoreServices.framework/Frameworks
LFLAGS += -compatibility_version 1 -current_version $(CURRENT_PROJECT_VERSION = 7)

C_WARNING_FLAGS += -Wno-precomp -Wno-four-char-constants -Wall
CPP_WARNING_FLAGS += -Wno-precomp -Wno-four-char-constants -Wall
ifneq "$(ARCHS)" ""
ARCH_FLAGS = $(foreach A, $(ARCHS), $(addprefix -arch , $(A)))
else
ifneq "$(RC_ARCHS)" ""
ARCH_FLAGS = $(foreach A, $(RC_ARCHS), $(addprefix -arch , $(A)))
else
ARCH_FLAGS = -arch ppc
endif

# Install two private headers for internal Apple projects' use
	$(SILENT) $(COPY) Base.subproj/CFPriv.h Base.subproj/CFRuntime.h PlugIn.subproj/CFBundlePriv.h $(DSTROOT)/$(FRAMEWORK_DIR)/Versions/A/PrivateHeaders
	$(SILENT) $(CHOWN) -R root:wheel $(DSTROOT)/$(FRAMEWORK_DIR)
	-$(SILENT) $(CHMOD) -w $(DSTROOT)/$(FRAMEWORK_DIR)/Versions/A/Headers/*.h #*/
	-$(SILENT) $(CHMOD) -w $(DSTROOT)/$(FRAMEWORK_DIR)/Versions/A/PrivateHeaders/*.h #*/

target_include_directories(CFNetwork PRIVATE Headers)

target_link_options(CFNetwork PRIVATE -F${RAVYN_SDKROOT}/System/Library/Frameworks)
#set_source_files_properties(NSProcessInfo.m NSStringsFileFormat/NSStringsFileParser.m NSData/NSData.m NSNumberFormatter.m TARGET_DIRECTORY Foundation PROPERTIES COMPILE_OPTIONS "-I${CMAKE_CURRENT_SOURCE_DIR}/NSString")
