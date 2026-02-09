/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 *  CFServer.c
 *  CFNetwork
 *
 *  Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
 *
 */

#pragma mark Includes
#include <CFNetwork/CFServerPriv.h>

#include <CoreFoundation/CFRuntime.h>
#include <CFNetwork/CFNetwork.h>

#include <assert.h>

#if defined(__WIN32__)
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif


#if 0
#pragma mark -
#pragma mark Constant Strings
#endif

#ifdef __CONSTANT_CFSTRINGS__
#define _kCFServerNULL				CFSTR("0x0")
#define _kCFServerPtrFormat			CFSTR("<0x%x>")
#define _kCFServerDescribeFormat	CFSTR("<Server 0x%x>{sockets=[%@, %@], service=%@, info=%@}")
#define _kCFServerEmptyString		CFSTR("")
#else
static CONST_STRING_DECL(_kCFServerNULL, "0x0")
static CONST_STRING_DECL(_kCFServerPtrFormat, "<0x%x>")
static CONST_STRING_DECL(_kCFServerDescribeFormat, "<Server 0x%x>{sockets=[%@, %@], service=%@, info=%@}")
static CONST_STRING_DECL(_kCFServerEmptyString, "")
#endif	/* __CONSTANT_CFSTRINGS__ */


#pragma mark -
#pragma mark Type Declarations

typedef struct {
    CFRuntimeBase		_base;			// CFRuntimeBase for CF types
	
	CFSocketRef			_sockets[2];	// Server sockets listening for connections
	
	CFStringRef			_name;			// Name that is being registered
	CFStringRef			_type;			// Service type that is being registered
    UInt32				_port;			// Port being serviced
	CFNetServiceRef		_service;		// Registered service on the network
	
	_CFServerCallBack	_callback;		// User's callback function
	_CFServerContext	_ctxt;			// User's context info
} Server;


#pragma mark -
#pragma mark Constant Definitions


#pragma mark -
#pragma mark Static Function Declarations

static void _ServerRelease(_CFServerRef server);
CFStringRef _ServerCopyDescription(_CFServerRef server);

static void _ServerReleaseSocket(Server* server);
static void _ServerHandleAccept(Server* server, CFSocketNativeHandle nativeSocket);
static void _SocketCallBack(CFSocketRef sock, CFSocketCallBackType type, CFDataRef address, const void *data, Server* server);

#if defined(__MACH__)
static void _ServerReleaseNetService(Server* server);
static Boolean _ServerCreateAndRegisterNetService(Server* server);
static void _ServerHandleNetServiceError(Server* server, CFStreamError* error);
static void _NetServiceCallBack(CFNetServiceRef service, CFStreamError* error, Server* server);
#endif


#pragma mark -
#pragma mark Static Variable Definitions

static CFTypeID _ServerTypeId = _kCFRuntimeNotATypeID;


#pragma mark -
#pragma mark Extern Function Definitions (API)


/* CF_EXPORT */ CFTypeID
_CFServerGetTypeID(void) {
    
    if (_ServerTypeId == _kCFRuntimeNotATypeID) {

        static const CFRuntimeClass ServerClass = {
            0,														// version
            "_CFServer",											// class name
            NULL,													// init
            NULL,													// copy
            (void(*)(CFTypeRef))_ServerRelease,						// finalize
            NULL,													// equal
            NULL,													// hash
            NULL,													// copy formatting description
            
            (CFStringRef(*)(CFTypeRef))_ServerCopyDescription		// copy debug description
        };
        
        _ServerTypeId = _CFRuntimeRegisterClass(&ServerClass);
    }
        
    return _ServerTypeId;
}


/* extern */ _CFServerRef
_CFServerCreate(CFAllocatorRef alloc, _CFServerCallBack callback, _CFServerContext* context) {
    
    Server* server = NULL;
	    
	do {
		int yes = 1;
		CFSocketContext socketCtxt = {0,
									  NULL,
									  (const void*(*)(const void*))&CFRetain,
									  (void(*)(const void*))&CFRelease,
									  (CFStringRef(*)(const void *))&CFCopyDescription};
        
        CFTypeID id = _CFServerGetTypeID();
    
        // Ask CF to allocate the instance and then return it.
        if (id != _kCFRuntimeNotATypeID) {
            server = (Server*)_CFRuntimeCreateInstance(alloc,
                                                       id,
                                                       sizeof(Server) - sizeof(CFRuntimeBase),
                                                       NULL);
        }
		
		// Fail if unable to create the server
		if (server == NULL)
			break;

        server->_name = NULL;
        server->_type = NULL;
        server->_port = 0;
        server->_service = NULL;
        memset(&server->_callback, 0, sizeof(server->_callback));
        memset(&server->_ctxt, 0, sizeof(server->_ctxt));
        
		// Make sure the server is saved for the callback.
		socketCtxt.info = server;
		
		// Create the IPv4 server socket.
		server->_sockets[0] = CFSocketCreate(alloc,
											 PF_INET,
											 SOCK_STREAM,
											 IPPROTO_TCP,
											 kCFSocketAcceptCallBack,
											 (CFSocketCallBack)&_SocketCallBack,
											 &socketCtxt);
		
		// If the socket couldn't create, bail.
		if (server->_sockets[0] == NULL)
			break;
		
		// Create the IPv6 server socket.
		server->_sockets[1] = CFSocketCreate(alloc,
											 PF_INET6,
											 SOCK_STREAM,
											 IPPROTO_TCP,
											 kCFSocketAcceptCallBack,
											 (CFSocketCallBack)&_SocketCallBack,
											 &socketCtxt);
		
		// If the socket couldn't create, bail.
		if (server->_sockets[1] == NULL)
			break;
		
		// In order to accomadate stopping and starting the process without closing the socket,
		// set the addr for resuse on the native socket.  This is not required if the port is
		// being supplied by the OS opposed to being specified by the user.
		setsockopt(CFSocketGetNative(server->_sockets[0]), SOL_SOCKET, SO_REUSEADDR, (void*)&yes, sizeof(yes));
		setsockopt(CFSocketGetNative(server->_sockets[1]), SOL_SOCKET, SO_REUSEADDR, (void*)&yes, sizeof(yes));
        
		// Save the user's callback in context.
		server->_callback = callback;
		memcpy(&(server->_ctxt), context, sizeof(server->_ctxt));
		
		// If there is info and a retain function, retain the info.
		if (server->_ctxt.info && server->_ctxt.retain)
			server->_ctxt.info = (void*)server->_ctxt.retain(server->_ctxt.info);
		
		return (_CFServerRef)server;
			
	} while (0);
	
	// Something failed, so clean up.
	if (server) {
		_CFServerInvalidate((_CFServerRef)server);
		CFRelease((_CFServerRef)server);
	}

    return NULL;
}


/* extern */ UInt32
_CFServerGetPort(_CFServerRef server) {

    return ((Server*)server)->_port & 0x0000FFFF;
}


/* static */ void
_ServerRelease(_CFServerRef server) {

    // Invalidate the server which will release the socket and service.
    _CFServerInvalidate(server);
}


/* static */ CFStringRef
_ServerCopyDescription(_CFServerRef server) {

	Server* s = (Server*)server;
    CFAllocatorRef alloc = CFGetAllocator(server);
	
	CFTypeRef socket4, socket6, service;
	CFStringRef info, result;
	
	// Start with everything being "NULL"
	socket4 = socket6 = service = _kCFServerNULL;
	
	// Set socket to it's value
	if (s->_sockets[0] != NULL)
		socket4 = (CFTypeRef)(s->_sockets[0]);
	
	// Set socket to it's value
	if (s->_sockets[1] != NULL)
		socket6 = (CFTypeRef)(s->_sockets[1]);
		
	// Set service to it's value
	if (s->_service != NULL)
		service = (CFTypeRef)(s->_service);
	
	// Set the user's context based upon supplied "copyDescription"
	if (s->_ctxt.copyDescription)
		info = s->_ctxt.copyDescription(s->_ctxt.info);
	else
		info = CFStringCreateWithFormat(alloc, NULL, _kCFServerPtrFormat, (UInt32)(s->_ctxt.info));
	
	// Create the debug string
    result = CFStringCreateWithFormat(alloc,
									  NULL,
									  _kCFServerDescribeFormat,
									  (UInt32)server,
									  socket4,
									  socket6,
									  service,
									  info);
	
	// Release the user's string
	CFRelease(info);
	
	return result;
}


/* extern */ Boolean
_CFServerStart(_CFServerRef server, CFStringRef name, CFStringRef type, UInt32 port) {
	
	Server* s = (Server*)server;

	CFDataRef address = NULL;
	
	do {
        unsigned i;
		CFRunLoopRef rl = CFRunLoopGetCurrent();
        CFAllocatorRef alloc = CFGetAllocator(server);
		
        struct sockaddr_in addr4;
        struct sockaddr_in6 addr6;
				
		// Make sure the port is valid (0 - 65535).
		if ((port & 0xFFFF0000U) != 0)
			break;
		
		// NULL means to use the machine name.
		if (name == NULL)
			name = _kCFServerEmptyString;
		
		for (i = 0; i < (sizeof(s->_sockets) / sizeof(s->_sockets[0])); i++) {
		
			// Create the run loop source for putting on the run loop.
			CFRunLoopSourceRef src = CFSocketCreateRunLoopSource(alloc, s->_sockets[i], 0);
			if (src == NULL)
				break;
				
			// Add the run loop source to the current run loop and default mode.
			CFRunLoopAddSource(rl, src, kCFRunLoopCommonModes);
			CFRelease(src);
		}

		memset(&addr4, 0, sizeof(addr4));
		
		// Put the local port and address into the native address.
#if !defined(__WIN32__)
        addr4.sin_len = sizeof(addr4);
#endif
		addr4.sin_family = AF_INET;
		addr4.sin_port = htons((UInt16)port);
		addr4.sin_addr.s_addr = htonl(INADDR_ANY);
		
		// Wrap the native address structure for CFSocketCreate.
		address = CFDataCreateWithBytesNoCopy(alloc, (const UInt8*)&addr4, sizeof(addr4), kCFAllocatorNull);
		
		// If it failed to create the address data, bail.
		if (address == NULL)
			break;
			
		// Set the local binding which causes the socket to start listening.
		if (CFSocketSetAddress(s->_sockets[0], address) != kCFSocketSuccess)
			break;
		
		CFRelease(address);
		
		address = CFSocketCopyAddress(s->_sockets[0]);
		memcpy(&addr4, CFDataGetBytePtr(address), CFDataGetLength(address));
            
		port = ntohs(addr4.sin_port);

		CFRelease(address);

		memset(&addr6, 0, sizeof(addr6));

        // Put the local port and address into the native address.
        addr6.sin6_family = AF_INET6;
#ifndef __WIN32__
        addr6.sin6_port = htons((UInt16)port);
        addr6.sin6_len = sizeof(addr6);
        memcpy(&(addr6.sin6_addr), &in6addr_any, sizeof(addr6.sin6_addr));
#else
#ifndef __MINGW32__
        // real MS headers have this
        IN6ADDR_SETANY(addr6);
        addr6.sin6_port = htons((UInt16)port);
#else
        addr6.sin6_port = htons((UInt16)port);
        // mingw's w32 headers have this INIT macro instead, for some odd reason
        struct sockaddr_in6 in6addr_any = IN6ADDR_ANY_INIT;
        memcpy(&(addr6.sin6_addr), &in6addr_any, sizeof(addr6.sin6_addr));
#endif
#endif
        
		// Wrap the native address structure for CFSocketCreate.
		address = CFDataCreateWithBytesNoCopy(alloc, (const UInt8*)&addr6, sizeof(addr6), kCFAllocatorNull);
			
		// Set the local binding which causes the socket to start listening.
		if (CFSocketSetAddress(s->_sockets[1], address) != kCFSocketSuccess)
			break;
		
		// Save the name, service type and port.
        s->_name = CFRetain(name);
		s->_type = type ? CFRetain(type) : NULL;
		s->_port = port;

#if defined(__MACH__)
        // Attempt to register the service on the network. 
		if (type && !_ServerCreateAndRegisterNetService(s))
            break;
#endif

        // Release this since it's not needed any longer. 
		CFRelease(address);
	
		return TRUE;
        
	} while (0);
	
	// Handle the error cleanup.
	
	// Release the address data if it was created.
	if (address)
		CFRelease(address);

	// Kill the socket if it was created.
	_ServerReleaseSocket(s);

	return FALSE;
}


/* extern */ void
_CFServerInvalidate(_CFServerRef server) {
	
	Server* s = (Server*)server;
	
	// Release the user's context info pointer.
	if (s->_ctxt.info && s->_ctxt.release)
		s->_ctxt.release(s->_ctxt.info);
		
	// Clear out the context, so nothing can be called.
	memset(&(s->_ctxt), 0, sizeof(s->_ctxt));
	
	// Guarantee that there will be no user callback.
	s->_callback = NULL;

#if defined(__MACH__)
    // Release the net service.
    _ServerReleaseNetService(s);
#endif
    
    if (s->_name) {
        CFRelease(s->_name);
        s->_name = NULL;
    }
    
    if (s->_type) {
        CFRelease(s->_type);
        s->_type = NULL;
    }
    
    // Release the socket.
    _ServerReleaseSocket(s);
}


#pragma mark -
#pragma mark Static Function Definitions


#if defined(__MACH__)
/* static */ void
_ServerReleaseNetService(Server* server) {
	
	// Unschedule, cancel, and release the net service if there is one.
	if (server->_service != NULL) {
		CFNetServiceUnscheduleFromRunLoop(server->_service, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
		CFNetServiceSetClient(server->_service, NULL, NULL);
		CFNetServiceCancel(server->_service);
		CFRelease(server->_service);
		server->_service = NULL;
	}
}
#endif


/* static */ void
_ServerReleaseSocket(Server* server) {
	
    unsigned i;
    
	for (i = 0; i < (sizeof(server->_sockets) / sizeof(server->_sockets[0])); i++) {
		
		// Invalidate and release the socket if there is one.
		if (server->_sockets[i] != NULL) {
			CFSocketInvalidate(server->_sockets[i]);
			CFRelease(server->_sockets[i]);
			server->_sockets[i] = NULL;
		}
	}
}


#if defined(__MACH__)
/* static */ Boolean
_ServerCreateAndRegisterNetService(Server* server) {

	do {
        UInt32 port = server->_port;
		Boolean didSet, didRegister;
		CFNetServiceClientContext netSvcCtxt = {0,
												server,
												(CFAllocatorRetainCallBack)&CFRetain,
												(CFAllocatorReleaseCallBack)&CFRelease,
												(CFAllocatorCopyDescriptionCallBack)&CFCopyDescription};
        
        // If the port was unspecified, get the port from the socket.
        if (port == 0) {
            
            // Get the local address
            CFDataRef addr = CFSocketCopyAddress(server->_sockets[0]);
            struct sockaddr_in* nativeAddr = (struct sockaddr_in*)CFDataGetBytePtr(addr);
            
            CFRelease(addr);
            
            port = ntohs(nativeAddr->sin_port);
        }
        
        // Create the service for registration.
        server->_service = CFNetServiceCreate(CFGetAllocator((_CFServerRef)server),
                                              _kCFServerEmptyString,
                                              server->_type,
                                              server->_name,
                                              port);
        
		// Require the service for the socket.
		if (server->_service == NULL)
			break;
					
		// Try setting the client on the service.
		didSet = CFNetServiceSetClient(server->_service,
									   (CFNetServiceClientCallBack)&_NetServiceCallBack,
									   &netSvcCtxt);
	
		// Check to make sure it set before registering.
		if (!didSet)
			break;
	
		// Schedule the service on the run loop.
		CFNetServiceScheduleWithRunLoop(server->_service, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
			
		// Start the registration.
		didRegister = CFNetServiceRegisterWithOptions(server->_service, 0, NULL);
		
		// If registration failed, die.
		if (!didRegister)
			break;
			
		return TRUE;
	
    } while (0);
    
	// Failed to set up the service, so clean up anything that succeeded.
	_ServerReleaseNetService(server);
	
    return FALSE;
}
#endif


/* static */ void
_ServerHandleAccept(Server* server, CFSocketNativeHandle nativeSocket) {
	
	// Inform the user of an incoming connection.
	if (server->_callback != NULL) {
		CFStreamError error = {0, 0};
		server->_callback((_CFServerRef)server, nativeSocket, &error, server->_ctxt.info);
	}
}


#if defined(__MACH__)
/* static */ void
_ServerHandleNetServiceError(Server* server, CFStreamError* error) {

	// No matter what happened, tear down the service.
	_ServerReleaseNetService(server);

	// Handle the error.
    if (error->error != 0) {
    
		// Kill the underlying socket to prevent callbacks.
		_ServerReleaseSocket(server);
	
		// Inform the user of the error.
		if (server->_callback != NULL)
			server->_callback((_CFServerRef)server, (CFSocketNativeHandle)(-1), error, server->_ctxt.info);
	}
}
#endif


/* static */ void
_SocketCallBack(CFSocketRef sock, CFSocketCallBackType type, CFDataRef address, const void *data, Server* server) {

	assert((sock == server->_sockets[0]) || (sock == server->_sockets[1]));

	// Only care about accept callbacks.
    if (type == kCFSocketAcceptCallBack) {
    
		assert((data != NULL) && (*((CFSocketNativeHandle*)data) != -1));
		
		// Dispatch the accept event.
		_ServerHandleAccept(server, *((CFSocketNativeHandle*)data));
	}
}


#if defined(__MACH__)
/* static */ void
_NetServiceCallBack(CFNetServiceRef service, CFStreamError* error, Server* server) {
    
    assert(service == server->_service);
    
	// Dispatch the registration error.
	if (error->error)
		_ServerHandleNetServiceError(server, error);
}
#endif
