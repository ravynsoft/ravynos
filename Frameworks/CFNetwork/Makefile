# Simple makefile for building the CNetwork. 
#
# Define sets of files to build, other info specific to this project.
#

NAME = CFNetwork

PUBLIC_HEADERS = CFFTPStream.h CFHost.h CFHTTPAuthentication.h CFHTTPMessage.h CFHTTPStream.h CFNetDiagnostics.h CFNetServices.h CFNetwork.h CFNetworkDefs.h CFSocketStream.h
PRIVATE_HEADERS = CFFTPStreamPriv.h CFHostPriv.h CFHTTPConnectionPriv.h CFHTTPMessagePriv.h CFHTTPServerPriv.h CFHTTPStreamPriv.h CFNetDiagnosticsPriv.h CFNetServicesPriv.h CFNetworkPriv.h CFServerPriv.h CFSocketStreamPriv.h

# These are the actual vars that are used by framework.make
PUBLIC_HFILES = $(foreach S, $(PUBLIC_HEADERS), $(SRCROOT)/Headers/$(S))
PRIVATE_HFILES = $(foreach S, $(PRIVATE_HEADERS), $(SRCROOT)/Headers/$(S))

PROJECT_HFILES = CFNetworkInternal.h HTTP/CFHTTPConnectionInternal.h HTTP/CFHTTPInternal.h NetDiagnostics/CFNetDiagnosticsInternal.h NetDiagnostics/CFNetDiagnosticsProtocol.h NetServices/DeprecatedDNSServiceDiscovery.h Proxies/ProxySupport.h SharedCode/CFNetConnection.h SharedCode/CFNetworkSchedule.h SharedCode/CFNetworkThreadSupport.h Stream/CFSocketStreamImpl.h HTTP/SPNEGO/spnegoBlob.h HTTP/SPNEGO/spnegoDER.h HTTP/SPNEGO/spnegoKrb.h HTTP/NTLM/ntlmBlobPriv.h HTTP/NTLM/NtlmGenerator.h

CFILES = CFNetwork.c SharedCode/CFServer.c SharedCode/CFNetConnection.c SharedCode/CFNetworkSchedule.c SharedCode/CFNetworkThreadSupport.c \
	FTP/CFFTPStream.c Host/CFHost.c \
	HTTP/CFHTTPAuthentication.c HTTP/CFHTTPConnection.c HTTP/CFHTTPFilter.c HTTP/CFHTTPMessage.c HTTP/CFHTTPServer.c HTTP/CFHTTPStream.c\
	NetDiagnostics/CFNetDiagnosticPing.c NetDiagnostics/CFNetDiagnostics.c NetDiagnostics/CFNetDiagnosticsProtocolUser.c \
	NetServices/CFNetServices.c NetServices/CFNetServiceBrowser.c NetServices/CFNetServiceMonitor.c NetServices/DeprecatedDNSServiceDiscovery.c \
	Proxies/ProxySupport.c Stream/CFSocketStream.c URL/_CFURLAccess.c JavaScriptGlue.c libresolv.c

CPP_FILES = HTTP/SPNEGO/spnegoBlob.cpp HTTP/SPNEGO/spnegoDER.cpp HTTP/SPNEGO/spnegoKrb.cpp HTTP/NTLM/ntlmBlobPriv.cpp HTTP/NTLM/NtlmGenerator.cpp

OTHER_CFLAGS += -F/System/Library/PrivateFrameworks -F/usr/local/SecurityPieces/Frameworks
OTHER_CPPFLAGS += -F/usr/local/SecurityPieces/Frameworks
OTHER_LFLAGS += -framework CoreFoundation -framework Security -framework SystemConfiguration -F/usr/local/SecurityPieces/Frameworks -framework security_cdsa_utils

# Careful:  This must be included after files are set, since they are used in dependencies which
# evaluate variables on the first parsing pass.  Other variables used in rule bodiess are evaluated
# when the rules run, so they come after to have an additive effect on any defaults.
include framework.make

#
# Misc additional options
#

CURRENT_PROJECT_VERSION = 7

# -DAVAILABLE_MAC_OS_X_VERSION_XMerlot_AND_LATER is a hack - not sure why we need this, perhaps
# we need a newer version of Interfacer to build Merlot-based CFNetwork
#
# base addr is set to come after CoreFoundation - use the rebase MS command to see the sizes
# more info at http://msdn.microsoft.com/library/en-us/tools/tools/rebase.asp
ifeq "$(PLATFORM)" "CYGWIN"
CFLAGS += -DCFNETWORK_BUILDING_DLL -DAVAILABLE_MAC_OS_X_VERSION_XMerlot_AND_LATER=
CPPFLAGS += -DCFNETWORK_BUILDING_DLL
LIBS += -lCoreFoundation$(LIBRARY_SUFFIX) -lole32 -loleaut32 -lws2_32 -lwininet -luuid
LFLAGS += -Wl,--image-base=0x660b0000
endif

ifeq "$(PLATFORM)" "Darwin"
CFLAGS += -F/System/Library/Frameworks/CoreServices.framework/Frameworks
CPPFLAGS += -F/System/Library/Frameworks/CoreServices.framework/Frameworks
LFLAGS += -compatibility_version 1 -current_version $(CURRENT_PROJECT_VERSION)
endif

ifeq "$(PLATFORM)" "FreeBSD"
LFLAGS += -shared
endif

ifeq "$(PLATFORM)" "Linux"
LIBS += -lpthread
endif


# needs to be after including framework.make, to get our options after the -Wall
C_WARNING_FLAGS += -Wmissing-prototypes -Wpointer-arith -Wcast-align -Wno-unknown-pragmas -Wmissing-declarations -Wbad-function-cast
CPP_WARNING_FLAGS += -Wmissing-prototypes -Wpointer-arith -Wcast-align -Wno-unknown-pragmas

#
# Install the PAC support JS routines (build time is done, install time not done)
#

ifeq "$(LIBRARY_STYLE)" "Library"
prebuild_after::
	$(SILENT) $(COPY_RECUR) PACSupport.js $(RESOURCE_DIR)

clean_after::
	$(REMOVE_RECUR) -f $(RESOURCE_DIR)/PACSupport.js
endif


# mimics the Tests target in PB
#Tests:
#	(cd HTTP/HTTPEcho; make)
#	(cd HTTP/HTTPSConnect; make)
#	(cd HTTP/ListLoad; make)
#	(cd Stream/gethostbyname; make)
#	(cd Stream/SocketStreamPerf; make)
