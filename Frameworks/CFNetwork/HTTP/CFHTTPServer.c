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
 *  CFHTTPServer.c
 *  CFNetwork
 *
 *  Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
 *
 */


#pragma mark Description
/*
    The http server is comprised of two objects: a HttpServer and a HttpConnection.  The
    HttpServer is primarily responsible for listening and accepting new connections.
    As individual connections are established, HttpConnections are created in order to
    maintain that single connection's instance.  The HttpServer then maintains the list
    of these HttpConnection's.
    
    Individual requests and responses are handled on a per-HttpConnection basis.  Although
    handled on individual HttpConnection's, clients adding responses for a given request
    do so through the HttpServer's interface.  The HttpServer will then pair the response
    to the request on the proper HttpConnection instance.
    
    All responses are sent out in the order in which the requests were received.  A set
    of responses can be held up should a response not be available for the head queued
    item.  This can take place since responses are not required to be queued at the
    moment that a request is received.
    
    When the response is added to the queue, two items are used.  First there is the
    queue of ordered requests on the connection.  This ordered array is maintained in
    order to send the responses in the correct order.  The second part is a dictionary
    mapping an individual request to its response.  A response is comprised of a set of
    headers (CFHTTPMessageRef) and a stream (CFReadStreamRef) which acts as the body.
    Every outgoing response has both of these elements.  This means that
    _CFHTTPServerAddResponse creates a stream for the body of the response and then calls
    _CFHTTPServerAddStreamedResponse.
    
    Some cheap object model:
    
    ------------  maintains  ---------------- receives  ---------
    |HttpServer|------------@|HttpConnection|----------@|request|
    ------------             ----------------           ---------
                                     |                     |
                                     @ vends               |
                                 ----------                |
                                 |response|-----------------
                                 ----------
*/

#pragma mark -
#pragma mark Includes
#include <CFNetwork/CFServerPriv.h>
#include <CFNetwork/CFHTTPServerPriv.h>

#include <CoreFoundation/CFRuntime.h>
#if !defined(__WIN32__)
#include <sys/types.h>
#include <sys/socket.h>
#else
#include <winsock2.h>
#define SOCK_MAXADDRLEN 255
#endif


#if 0
#pragma mark -
#pragma mark Constant Strings
#endif

#ifdef __CONSTANT_CFSTRINGS__
#define _kCFHTTPServerDescribeFormat			CFSTR("<HttpServer 0x%x>{server=%@, connections=%@, info=%@}")
#define _kCFHTTPServerPtrFormat					CFSTR("<0x%x>")
#define _kCFHTTPServerContentLengthHeader		CFSTR("Content-length")
#define _kCFHTTPServerContentLengthFormat		CFSTR("%d")
#define _kCFHTTPServerConnectionDescribeFormat	CFSTR("<_HttpConnection 0x%x>{server=0x%x, timer=%@, inStream=%@, outStream=%@, responses=%@, requests=%@, buffered=%@}")
#define _kCFHTTPServerTransferEncodingHeader	CFSTR("Transfer-Encoding")
#define _kCFHTTPServerTransferEncodingChunked	CFSTR("chunked")
#define _kCFHTTPServerConnectionHeader			CFSTR("Connection")
#define _kCFHTTPServerConnectionClose			CFSTR("close")
#else
static CONST_STRING_DECL(_kCFHTTPServerDescribeFormat, "<HttpServer 0x%x>{server=%@, connections=%@, info=%@}")
static CONST_STRING_DECL(_kCFHTTPServerPtrFormat, "<0x%x>")
static CONST_STRING_DECL(_kCFHTTPServerContentLengthHeader, "Content-length")
static CONST_STRING_DECL(_kCFHTTPServerContentLengthFormat, "%d")
static CONST_STRING_DECL(_kCFHTTPServerConnectionDescribeFormat, "<_HttpConnection 0x%x>{server=0x%x, timer=%@, inStream=%@, outStream=%@, responses=%@, requests=%@, buffered=%@}")
static CONST_STRING_DECL(_kCFHTTPServerTransferEncodingHeader, "Transfer-Encoding")
static CONST_STRING_DECL(_kCFHTTPServerTransferEncodingChunked, "chunked")
static CONST_STRING_DECL(_kCFHTTPServerConnectionHeader, "Connection")
static CONST_STRING_DECL(_kCFHTTPServerConnectionClose, "close")
#endif	/* __CONSTANT_CFSTRINGS__ */


#pragma mark -
#pragma mark Type Declarations

typedef struct {
    CFRuntimeBase			_base;			// CFRuntimeBase for CF types
	
	_CFServerRef			_server;		// Underlying server object.
	
	CFMutableArrayRef		_connections;	// All outstanding HttpConnection's
    
    _CFHTTPServerCallBacks	_callbacks;		// Callback functions for user
    _CFHTTPServerContext	_ctxt;			// User's context for callback
} HttpServer;


typedef struct {
    CFAllocatorRef			_alloc;			// Allocator used to allocate this
    UInt32					_rc;			// Number of times retained.
	
	HttpServer*				_server;		// Reference back to the owning server context.
	
    CFDataRef				_peer;			// Peer's address
    
    CFRunLoopTimerRef		_timer;			// Timer for controlling timeouts
    
    CFReadStreamRef			_inStream;		// Incoming data stream
    CFWriteStreamRef		_outStream;		// Outgoing data stream
	
	CFMutableDictionaryRef	_responses;		// Responses keyed by their requests
	CFMutableArrayRef		_requests;		// Ordered incoming requests
	
	CFMutableDataRef		_bufferedBytes;	// Bytes bound for delivery but not yet sent
} HttpConnection;


#pragma mark -
#pragma mark Static Function Declarations

// Functions for HttpServer object
static void _HttpServerRelease(_CFHTTPServerRef server);
static CFStringRef _HttpServerCopyDescription(_CFHTTPServerRef server);

// Functions for HttpConnection object
static HttpConnection* _HttpConnectionCreate(CFAllocatorRef alloc, HttpServer* server, CFSocketNativeHandle s);
static HttpConnection* _HttpConnectionRetain(HttpConnection* connection);
static void _HttpConnectionRelease(HttpConnection* connection);
static CFStringRef _HttpConnectionCopyDescription(HttpConnection* connection);

// Handlers for HttpConnection object
static void _HttpConnectionHandleRequest(HttpConnection* connection);
static void _HttpConnectionHandleHasBytesAvailable(HttpConnection* connection);
static void _HttpConnectionHandleCanAcceptBytes(HttpConnection* connection);
static void _HttpConnectionHandleErrorOccurred(HttpConnection* connection, const CFStreamError* error);
static void _HttpConnectionHandleTimeOut(HttpConnection* connection);

static const void*	_ArrayRetainCallBack(CFAllocatorRef allocator, const HttpConnection* connection);
static void _ArrayReleaseCallBack(CFAllocatorRef allocator, const HttpConnection* connection);


// CFType callbacks -- call into HttpConnection's handlers
static void _ReadStreamCallBack(CFReadStreamRef inStream, CFStreamEventType type, HttpConnection* connection);
static void _WriteStreamCallBack(CFWriteStreamRef outStream, CFStreamEventType type, HttpConnection* connection);
static void _TimerCallBack(CFRunLoopTimerRef timer, HttpConnection* connection);

// Functions for manipulating HttpServer's array of HttpConnection's
static void _HttpServerAddConnection(HttpServer* server, HttpConnection* connection);
static void _HttpServerRemoveConnection(HttpServer* server, HttpConnection* connection);

// Handlers for HttpServer object
static void _HttpServerHandleNewConnection(HttpServer* server, CFSocketNativeHandle sock);
static void _HttpServerHandleError(HttpServer* server, const CFStreamError* error);

// Server callback -- call into HttpServer's handlers
static void _ServerCallBack(_CFServerRef server, CFSocketNativeHandle sock, const CFStreamError* error, HttpServer* httpServer);

// General use function
static CFNumberRef _CFNumberCreateWithString(CFAllocatorRef allocator, CFStringRef string);


#if 0
#pragma mark -
#pragma mark Extern Function Declarations
#endif

extern void _CFSocketStreamCreatePair(CFAllocatorRef alloc, CFStringRef host, UInt32 port, CFSocketNativeHandle s,
									  const CFSocketSignature* sig, CFReadStreamRef* readStream, CFWriteStreamRef* writeStream);




#pragma mark -
#pragma mark Static Variable Definitions

// A shorter timeout should be used for a more heavily used server.
#define kTimeOutInSeconds ((CFTimeInterval)60.0)
#define kBufferSize ((CFIndex)8192)

#define kReadEvents	((CFOptionFlags)(kCFStreamEventHasBytesAvailable | kCFStreamEventErrorOccurred))
#define kWriteEvents	((CFOptionFlags)(kCFStreamEventCanAcceptBytes | kCFStreamEventErrorOccurred))

static CFTypeID _HttpServerTypeId = _kCFRuntimeNotATypeID;


#pragma mark -
#pragma mark Extern Function Definitions (API)


/* CF_EXPORT */ CFTypeID
_CFHTTPServerGetTypeID(void) {
    
    if (_HttpServerTypeId == _kCFRuntimeNotATypeID) {

        static const CFRuntimeClass HttpServerClass = {
            0,														// version
            "_CFHTTPServer",										// class name
            NULL,													// init
            NULL,													// copy
            (void(*)(CFTypeRef))_HttpServerRelease,					// finalize
            NULL,													// equal
            NULL,													// hash
            NULL,													// copy formatting description
            
            (CFStringRef(*)(CFTypeRef))_HttpServerCopyDescription	// copy debug description
        };
        
        _HttpServerTypeId = _CFRuntimeRegisterClass(&HttpServerClass);
    }
        
    return _HttpServerTypeId;
}


/* CF_EXPORT */ _CFHTTPServerRef
_CFHTTPServerCreate(CFAllocatorRef alloc, const _CFHTTPServerCallBacks* callbacks, _CFHTTPServerContext* context) {

    HttpServer* server = NULL;

    do {
		_CFServerContext ctxt = {
            0,
            NULL,
            (CFAllocatorRetainCallBack)CFRetain,
            (CFAllocatorReleaseCallBack)CFRelease,
            (CFAllocatorCopyDescriptionCallBack)CFCopyDescription
        };
        
		CFArrayCallBacks arrayCallBacks = {
            0,
            (CFArrayRetainCallBack)_ArrayRetainCallBack,
            (CFArrayReleaseCallBack)_ArrayReleaseCallBack,
            (CFArrayCopyDescriptionCallBack)_HttpConnectionCopyDescription,
            NULL																// Default pointer comparison
        };
        
        CFTypeID id = _CFHTTPServerGetTypeID();
    
        // Ask CF to allocate the instance and then return it.
        if (id != _kCFRuntimeNotATypeID) {
            server = (HttpServer*)_CFRuntimeCreateInstance(alloc,
                                                           id,
                                                           sizeof(HttpServer) - sizeof(CFRuntimeBase),
                                                           NULL);
        }
        
        // Fail if unable to create the server
        if (server == NULL)
                break;
	
        server->_server = NULL;
	    server->_connections = NULL;
        memset(&server->_callbacks, 0, sizeof(server->_callbacks));
        memset(&server->_ctxt, 0, sizeof(server->_ctxt));
        
		// Set the info on the callback context
		ctxt.info = server;
		
		// Create the server
		server->_server = _CFServerCreate(alloc, (_CFServerCallBack)_ServerCallBack, &ctxt);

        // Require server in order to create.
        if (server->_server == NULL)
            break;
			
		server->_connections = CFArrayCreateMutable(alloc, 0, &arrayCallBacks);
		
		// Require the list of outstanding Http connections
		if (server->_connections == NULL)
			break;
                
		// Save the user's callbacks and context.
        memcpy(&(server->_callbacks), callbacks, sizeof(server->_callbacks));
		memcpy(&(server->_ctxt), context, sizeof(server->_ctxt));
		
		// If there is info and a retain function, retain the info.
		if (server->_ctxt.info && server->_ctxt.retain)
			server->_ctxt.info = (void *)(server->_ctxt.retain(server->_ctxt.info));

        return (_CFHTTPServerRef)server;
            
    } while (0);
	
	// Something failed, so clean up.
	if (server) {
		_CFHTTPServerInvalidate((_CFHTTPServerRef)server);
		CFRelease((_CFHTTPServerRef)server);
	}
    
    return NULL;
}


/* static */ void
_HttpServerRelease(_CFHTTPServerRef server) {
    
    // Invalidate the server which will release server and outstanding connections.
    _CFHTTPServerInvalidate(server);
}


/* static */ CFStringRef
_HttpServerCopyDescription(_CFHTTPServerRef server) {
    
    CFStringRef info, result, serverDescription = NULL;
    HttpServer* s = (HttpServer*)server;
    CFAllocatorRef alloc = CFGetAllocator(server);
	
    if (s->_server)
        serverDescription = CFCopyDescription(s->_server);
    
	// Set the user's context based upon supplied "copyDescription"
	if (s->_ctxt.copyDescription)
		info = s->_ctxt.copyDescription(s->_ctxt.info);
	else
		info = CFStringCreateWithFormat(alloc, NULL, _kCFHTTPServerPtrFormat, (UInt32)(s->_ctxt.info));
    
	// Create the debug string
    result = CFStringCreateWithFormat(alloc,
									  NULL,
									  _kCFHTTPServerDescribeFormat,
									  (UInt32)s,
                                      serverDescription,
									  s->_connections,
									  info);
                                      
    if (serverDescription)
        CFRelease(serverDescription);
        
    CFRelease(info);
    
    return result;
}


/* CF_EXPORT */ Boolean
_CFHTTPServerStart(_CFHTTPServerRef server, CFStringRef name, CFStringRef type, UInt32 port) {

    HttpServer* s = (HttpServer*)server;

    // Nothing special needed for the HTTP server.
    
    return _CFServerStart(s->_server, name, type, port);
}


/* CF_EXPORT */ void
_CFHTTPServerInvalidate(_CFHTTPServerRef server) {
	
	HttpServer* s = (HttpServer*)server;
	
	// Release the user's context info pointer.
	if (s->_ctxt.info && s->_ctxt.release)
		s->_ctxt.release(s->_ctxt.info);
		
	// Clear out the context, so nothing can be called.
	memset(&(s->_ctxt), 0, sizeof(s->_ctxt));
	
	// Guarantee that there will be no user callbacks.
    memset(&s->_callbacks, 0, sizeof(s->_callbacks));
    
    // Close out any outstanding connections.
    if (s->_connections) {
        CFRelease(s->_connections);
        s->_connections = NULL;
    }
    
    // If the server has been created, invalidate it and delete it.
    if (s->_server) {
        _CFServerInvalidate(s->_server);
        CFRelease(s->_server);
        s->_server = NULL;
    }
}


/* CF_EXPORT */ UInt32
_CFHTTPServerGetPort(_CFHTTPServerRef server) {

    return ((HttpServer*)server)->_server ? _CFServerGetPort(((HttpServer*)server)->_server) : 0;
}


/* CF_EXPORT */ CFDataRef
_CFHTTPServerCopyPeerAddressForRequest(_CFHTTPServerRef server, CFHTTPMessageRef request) {
    
    CFIndex i, count;
    HttpServer* s = (HttpServer*)server;
    
    // Prepare to look for the given request in the connections
    count = CFArrayGetCount(s->_connections);
    
    // Start the search
    for (i = 0; i < count; i++) {
        
        // **FIXME** This is somewhat incestuous.  The server should not be reaching
        // into the connections.  There should really be a HttpConnection method for
        // adding a response.
        
        // Pull out the current connection
        HttpConnection* c = (HttpConnection*)CFArrayGetValueAtIndex(s->_connections, i);
        
        // Check to see if the connection knows of the request
        CFIndex j = CFArrayGetFirstIndexOfValue(c->_requests, CFRangeMake(0, CFArrayGetCount(c->_requests)), request);
        
        // Handle the response if it was found
        if (j != kCFNotFound) {
        
            // return the copy that was found
            return (c->_peer == NULL) ? NULL : CFDataCreateCopy(CFGetAllocator(server), c->_peer);
        }
    }

    return NULL;
}


/* CF_EXPORT */ void
_CFHTTPServerAddResponse(_CFHTTPServerRef server, CFHTTPMessageRef request, CFHTTPMessageRef response) {

    CFDataRef body;
    UInt8* bytes;
    CFReadStreamRef stream;
    CFIndex length;
    CFStringRef contentLength;
    
    CFAllocatorRef alloc = CFGetAllocator(server);
    
    // Make a copy of the response
    response = CFHTTPMessageCreateCopy(alloc, response);
    
    // Get the body and its length
    body = CFHTTPMessageCopyBody(response);
    
    if (body == NULL)
        body = CFDataCreate(alloc, NULL, 0);
    
    length = CFDataGetLength(body);
    
    // Pull the body off the response since the stream will be used
    CFHTTPMessageSetBody(response, NULL);
    
    // Allocate the buffer for the body
    bytes = (UInt8*)CFAllocatorAllocate(alloc, length, 0);
    
    // Copy the body into the buffer for streaming
    memmove(bytes, CFDataGetBytePtr(body), length);
    
    // Don't need the body anymore
    CFRelease(body);
    
    // Create the stream for the body
    stream = CFReadStreamCreateWithBytesNoCopy(alloc, bytes, length, alloc);
    
    // Check to see if there is a content length header.
    contentLength = CFHTTPMessageCopyHeaderFieldValue(response, _kCFHTTPServerContentLengthHeader);
    
    // If not, add one.
    if (contentLength == NULL) {

        // Create the header value with the length
        contentLength = CFStringCreateWithFormat(alloc, NULL, _kCFHTTPServerContentLengthFormat, length);
        
        // Add the header
        CFHTTPMessageSetHeaderFieldValue(response, _kCFHTTPServerContentLengthHeader, contentLength);
    }
    CFRelease(contentLength);
    
    // Add the streamed response
    _CFHTTPServerAddStreamedResponse(server, request, response, stream);
    
    // No longer needed now that it's in the queue
    CFRelease(stream);
    CFRelease(response);
}


/* CF_EXPORT */ void
_CFHTTPServerAddStreamedResponse(_CFHTTPServerRef server, CFHTTPMessageRef request, CFHTTPMessageRef response, CFReadStreamRef body) {

    CFArrayRef list;
    CFIndex i, count;
    
    HttpServer* s = (HttpServer*)server;
    CFAllocatorRef alloc = CFGetAllocator(server);
    
    // Things to be put into the response list for a request
    CFTypeRef objs[] = {NULL, body};
    
    // Create a copy 'cause it may need adjustment
    objs[0] = CFHTTPMessageCreateCopy(alloc, response);
    
    // Create the response list for the request
    list = CFArrayCreate(alloc, objs, sizeof(objs) / sizeof(objs[0]), &kCFTypeArrayCallBacks);
    
    // Prepare to look for the given request in the connections
    count = CFArrayGetCount(s->_connections);
    
    // Start the search
    for (i = 0; i < count; i++) {
        
        // **FIXME** This is somewhat incestuous.  The server should not be reaching
        // into the connections.  There should really be a HttpConnection method for
        // adding a response.
        
        // Pull out the current connection
        HttpConnection* c = (HttpConnection*)CFArrayGetValueAtIndex(s->_connections, i);
        
        // Check to see if the connection knows of the request
        CFIndex j = CFArrayGetFirstIndexOfValue(c->_requests, CFRangeMake(0, CFArrayGetCount(c->_requests)), request);
        
        // Handle the response if it was found
        if (j != kCFNotFound) {
            
            // Add the response list to the connection for the given request
            CFDictionaryAddValue(c->_responses, request, list);
        
            // If the request was the head of the request queue and the stream can send, pump it.
            if ((j == 0) && CFWriteStreamCanAcceptBytes(c->_outStream))
                _HttpConnectionHandleCanAcceptBytes(c);
        
            // Everything has been handled
            break;
        }
    }
    
    // List has been handled, so it's not needed anymore.
    CFRelease(list);
    
    CFRelease(objs[0]);
}


#pragma mark -
#pragma mark Static Function Definitions

/* static */ HttpConnection*
_HttpConnectionCreate(CFAllocatorRef alloc, HttpServer* server, CFSocketNativeHandle s) {
    
    HttpConnection* connection = NULL;
	    
	do {
        uint8_t name[SOCK_MAXADDRLEN];
        socklen_t namelen = sizeof(name);

        CFRunLoopRef rl = CFRunLoopGetCurrent();
        
        CFRunLoopTimerContext timerCtxt = {
            0,
            NULL,
            NULL,
            NULL,
            (CFStringRef (*)(const void*))_HttpConnectionCopyDescription
        };
        
        CFStreamClientContext streamCtxt = {
            0,
            NULL,
            NULL,
            NULL,
            (CFStringRef (*)(void*))_HttpConnectionCopyDescription
        };
        
		// Allocate the buffer for the connection.
		connection = CFAllocatorAllocate(alloc, sizeof(connection[0]), 0);
		
		// Fail if unable to create the connection
		if (connection == NULL)
			break;
		
		memset(connection, 0, sizeof(connection[0]));
		
		// Save the allocator for deallocating later.
		connection->_alloc = alloc ? CFRetain(alloc) : NULL;
		
        // Bump the retain count.
        _HttpConnectionRetain(connection);
        
		// Make sure the server is saved for the callback.
		connection->_server = (HttpServer*)CFRetain((_CFHTTPServerRef)server);
		
        if (0 == getpeername(s, (struct sockaddr *)name, &namelen))
            connection->_peer = CFDataCreate(alloc, name, namelen);
        
        // Set the info pointer for the contexts to be the connection.
        timerCtxt.info = connection;
        streamCtxt.info = connection;
        
        // Create the timer for detecting dead connections
        connection->_timer = CFRunLoopTimerCreate(alloc,
                                                  CFAbsoluteTimeGetCurrent() + kTimeOutInSeconds,
                                                  kTimeOutInSeconds,
                                                  0,
                                                  0,
                                                  (CFRunLoopTimerCallBack)_TimerCallBack,
                                                  &timerCtxt);
        
        // Make sure it succeeded
        if (connection->_timer == NULL)
            break;
            
        // Add the timer to the run loop
        CFRunLoopAddTimer(rl, connection->_timer, kCFRunLoopCommonModes);
        
        // Create a pair of streams for performing HTTP.
		_CFSocketStreamCreatePair(alloc, NULL, 0, s, NULL, &(connection->_inStream), &(connection->_outStream));
        
        // Make sure both were created
        if ((connection->_inStream == NULL) || (connection->_outStream == NULL))
            break;
        
        // Relinquish the socket to the streams
        CFReadStreamSetProperty(connection->_inStream, kCFStreamPropertyShouldCloseNativeSocket, kCFBooleanTrue);
        CFWriteStreamSetProperty(connection->_outStream, kCFStreamPropertyShouldCloseNativeSocket, kCFBooleanTrue);
        
        // Set the client to for each of the streams and for the proper events.
        CFReadStreamSetClient(connection->_inStream, kReadEvents, (CFReadStreamClientCallBack)_ReadStreamCallBack, &streamCtxt);
        CFWriteStreamSetClient(connection->_outStream, kWriteEvents, (CFWriteStreamClientCallBack)_WriteStreamCallBack, &streamCtxt);
        
        // Schedule both on the run loop
        CFReadStreamScheduleWithRunLoop(connection->_inStream, rl, kCFRunLoopCommonModes);
        CFWriteStreamScheduleWithRunLoop(connection->_outStream, rl, kCFRunLoopCommonModes);
        
        // Open up the streams
        CFReadStreamOpen(connection->_inStream);
        CFWriteStreamOpen(connection->_outStream);
        
        // Create the dictionary mapping requests to responses
        connection->_responses = CFDictionaryCreateMutable(alloc,
                                                           0,
                                                           &kCFTypeDictionaryKeyCallBacks,
                                                           &kCFTypeDictionaryValueCallBacks);
        
        // Make sure it worked
        if (connection->_responses == NULL)
            break;
        
        // Create the list of all outstanding, incoming requests
        connection->_requests = CFArrayCreateMutable(alloc,
                                                     0,
                                                     &kCFTypeArrayCallBacks);
        
        // Make sure the list was created
        if (connection->_requests == NULL)
            break;
        
        // Create a buffer for any buffered bytes which will be sent out
        connection->_bufferedBytes = CFDataCreateMutable(alloc, 0);
        
        // Make sure there is a buffer
        if (connection->_bufferedBytes == NULL)
            break;
        
        // It's all good
		return connection;
			
	} while (0);
	
	// Something failed, so clean up.
	if (connection)
        _HttpConnectionRelease(connection);

    return NULL;
}


/* static */ HttpConnection*
_HttpConnectionRetain(HttpConnection* connection) {
	
	// Bump the retain count.
	connection->_rc++;
		
	return connection;
}


/* static */ void
_HttpConnectionRelease(HttpConnection* connection) {
	
	// Decrease the retain count.
	connection->_rc--;
	
	// Destroy the object if not being held.
	if (connection->_rc == 0) {
		
		// Hold locally so deallocation can happen and then safely release.
		CFAllocatorRef alloc = connection->_alloc;

        CFRunLoopRef runLoop = CFRunLoopGetCurrent();

        if (connection->_server)
            CFRelease((_CFHTTPServerRef)connection->_server);
        
        if (connection->_peer)
            CFRelease(connection->_peer);
        
        // Check if the read stream exists.
        if (connection->_inStream) {
            
            // Unschedule, close, and release it.
            CFReadStreamSetClient(connection->_inStream, 0, NULL, NULL);
            CFReadStreamUnscheduleFromRunLoop(connection->_inStream, runLoop, kCFRunLoopCommonModes);
            CFReadStreamClose(connection->_inStream);
            CFRelease(connection->_inStream);
        }
    
        // Check if the write stream exists.
        if (connection->_outStream) {
            
            // Unschedule, close, and release it.
            CFWriteStreamSetClient(connection->_outStream, 0, NULL, NULL);
            CFWriteStreamUnscheduleFromRunLoop(connection->_outStream, runLoop, kCFRunLoopCommonModes);
            CFWriteStreamClose(connection->_outStream);
            CFRelease(connection->_outStream);
        }
        
        // If the timer exists, toss it too.
        if (connection->_timer != NULL) {
            CFRunLoopRemoveTimer(runLoop, connection->_timer, kCFRunLoopCommonModes);
            CFRunLoopTimerInvalidate(connection->_timer);
            CFRelease(connection->_timer);
        }
        
        // Toss the dictionary of requests and responses
        if (connection->_responses)
            CFRelease(connection->_responses);
            
        // Toss the list of incoming requests
        if (connection->_requests)
            CFRelease(connection->_requests);
            
        // Toss the buffered bytes
        if (connection->_bufferedBytes)
            CFRelease(connection->_bufferedBytes);
        
		// Free the memory in use by the connection.
		CFAllocatorDeallocate(alloc, connection);
		
		// Release the allocator.
		if (alloc)
			CFRelease(alloc);
	}
}


/* static */ CFStringRef
_HttpConnectionCopyDescription(HttpConnection* connection) {
    
    CFStringRef result;
    
	// Create the debug string
    result = CFStringCreateWithFormat(connection->_alloc,
									  NULL,
									  _kCFHTTPServerConnectionDescribeFormat,
									  (UInt32)connection,
                                      (UInt32)connection->_server,
									  connection->_timer,
									  connection->_inStream,
                                      connection->_outStream,
                                      connection->_responses,
                                      connection->_requests,
                                      connection->_bufferedBytes);
                                      
    return result;
}


/* static */ void
_HttpConnectionHandleRequest(HttpConnection* connection) {
    
    assert(0 != CFArrayGetCount(connection->_requests));
    
    // Get the message with which to work (the last one)
    CFHTTPMessageRef msg = (CFHTTPMessageRef)CFArrayGetValueAtIndex(connection->_requests,
                                                                    CFArrayGetCount(connection->_requests) - 1);

    while (msg) {

        // Use to see if it is a chunked request
        CFStringRef encoding = CFHTTPMessageCopyHeaderFieldValue(msg, _kCFHTTPServerTransferEncodingHeader);

        // Assume not chunked
        Boolean chunked = FALSE;
        
        // If there is encoding, cheaply check for chunked.
        if (encoding) {
            chunked = CFStringFindWithOptions(encoding,
                                              _kCFHTTPServerTransferEncodingChunked,
                                              CFRangeMake(0, CFStringGetLength(encoding)),
                                              kCFCompareCaseInsensitive,
                                              NULL);
            CFRelease(encoding);
        }

        // If it's chunked, bail 'cause the API just isn't ready for
        // these types of requests yet.
        if (chunked) {

            // Establish an error
            CFStreamError error = {kCFStreamErrorDomainHTTP, kCFStreamErrorHTTPParseFailure};

            // Handle it just like an error.
            _HttpConnectionHandleErrorOccurred(connection, &error);

            break;
        }

        // No chunking info so use size
        else {

            // Assume zero length to start.
            SInt32 size = 0;

            // Grab the body for testing
            CFDataRef body = CFHTTPMessageCopyBody(msg);

            // Get the length of the current body on the message
            CFIndex length = body ? CFDataGetLength(body) : 0;
            
            // Get the size to see if everything is there.
            CFStringRef value = CFHTTPMessageCopyHeaderFieldValue(msg, _kCFHTTPServerContentLengthHeader);

            // Need to convert the value if there was a header
            if (value) {

                // Convert the header value to a CFNumber
                CFNumberRef num = _CFNumberCreateWithString(connection->_alloc, value);

                // If that succeeded, turn it into the actual size
                if (num) {
                    
                    // Pull out the true expected count of bytes
                    CFNumberGetValue(num, kCFNumberSInt32Type, &size);

                    CFRelease(num);
                }

                // Received a bad content-length header
                else {

                    if (body) CFRelease(body);
                    CFRelease(value);

                    // Establish an error
                    CFStreamError error = {kCFStreamErrorDomainHTTP, kCFStreamErrorHTTPParseFailure};

                    // Handle it just like an error.
                    _HttpConnectionHandleErrorOccurred(connection, &error);

                    break;
                }
                
                CFRelease(value);
            }

            // If enough bytes haven't arrived, bail out now.
            if (length < size) {
                if (body) CFRelease(body);
                break;
            }

            // If the message is just right, inform the client of the message
            // and then exit since this is no more to process.
            else if (length == size) {

                if (body) CFRelease(body);
                
                // Inform the client of the incoming request
                if (connection->_server->_callbacks.didReceiveRequestCallBack != NULL) {
                    CFRetain(msg);
                    connection->_server->_callbacks.didReceiveRequestCallBack((_CFHTTPServerRef)connection->_server,
                                                                              msg,
                                                                              connection->_server->_ctxt.info);
                    CFRelease(msg);
                }

                break;
            }

            // There are too many bytes in the body
            else {

                // Need to make new and truncate, 'cause the current one is too long.
                CFDataRef newBody = CFDataCreate(connection->_alloc, CFDataGetBytePtr(body), size);

                // Create a new request to capture the leftover bytes.
                CFHTTPMessageRef newMsg = CFHTTPMessageCreateEmpty(connection->_alloc, TRUE);
                
                // Set the new body on the first request
                CFHTTPMessageSetBody(msg, newBody);

                // Toss the new body since it's retained by the request.
                CFRelease(newBody);

                // Inform the client of the incoming request
                if (connection->_server->_callbacks.didReceiveRequestCallBack != NULL) {
                    CFRetain(msg);
                    connection->_server->_callbacks.didReceiveRequestCallBack((_CFHTTPServerRef)connection->_server,
                                                                              msg,
                                                                              connection->_server->_ctxt.info);
                    CFRelease(msg);
                }

                // Move on to the new message to handle it
                msg = newMsg;
                
                // Put the new request in the requests list.
                CFArrayAppendValue(connection->_requests, msg);

                // Drop the retain count now since it's being held by the queue.
                CFRelease(msg);

                // Add the leftover bytes from the first request to the new one
                if (!CFHTTPMessageAppendBytes(msg, CFDataGetBytePtr(body) + size, length - size)) {

                    // Establish an error
                    CFStreamError error = {kCFStreamErrorDomainHTTP, kCFStreamErrorHTTPParseFailure};

                    // Handle it just like an error.
                    _HttpConnectionHandleErrorOccurred(connection, &error);

                    CFRelease(body);
                    
                    break;
                }

                // Don't need the body.
                CFRelease(body);

                // Check to see if the new message is complete too.
                if (!CFHTTPMessageIsHeaderComplete(msg))
                    break;	// Not done so bail.

                // There is enough there so inform the client.
                else {

                    // Assume the client is willing to take on the request.
                    Boolean handle = TRUE;

                    // Check the client to make sure this new message should be processed.
                    if (connection->_server->_callbacks.acceptNewRequestCallBack) {

                        handle = connection->_server->_callbacks.acceptNewRequestCallBack((_CFHTTPServerRef)connection->_server,
                                                                                          newMsg,
                                                                                          connection->_peer,
                                                                                          connection->_server->_ctxt.info);
                    }

                    if (!handle) {
                        
                        // Remove the connection from the pool
                        _HttpServerRemoveConnection(connection->_server, connection);

                        // Bail now because the current connection has been killed
                        break;
                    }
                }
            }
        }
    }
}


/* static */ void
_HttpConnectionHandleHasBytesAvailable(HttpConnection* connection) {

	CFIndex bytes;
	UInt8 buffer[kBufferSize];
	
	CFHTTPMessageRef msg;
	
    // Get the count of requests currently known.
	CFIndex i = CFArrayGetCount(connection->_requests);
	
    // If there is, grab the last one with which to work
	if (i != 0)
		msg = (CFHTTPMessageRef)CFArrayGetValueAtIndex(connection->_requests, --i);
		
	else {
		
        // There was no requests, so create a new one with which to work
		msg = CFHTTPMessageCreateEmpty(connection->_alloc, TRUE);
		CFArrayAppendValue(connection->_requests, msg);
		CFRelease(msg);
	}
	
    // Try to read bytes off the wire
	bytes = CFReadStreamRead(connection->_inStream, buffer, sizeof(buffer));
	
    // Did it succeed?
	if (bytes >= 0) {
		
        Boolean complete = CFHTTPMessageIsHeaderComplete(msg);
        
        // Tickle the timer
        CFRunLoopTimerSetNextFireDate(connection->_timer, CFAbsoluteTimeGetCurrent() + kTimeOutInSeconds);
        
        // Attach read bytes to current request
        if (!CFHTTPMessageAppendBytes(msg, buffer, bytes)) {

            // Establish an error
            CFStreamError error = {kCFStreamErrorDomainHTTP, kCFStreamErrorHTTPParseFailure};

            // Handle it just like an error.
            _HttpConnectionHandleErrorOccurred(connection, &error);

            return;
        }
        
        // If the request is complete, handle it as appropriate.
        if (CFHTTPMessageIsHeaderComplete(msg)) {
            
            // Assume the client is willing to take on the request.
            Boolean handle = TRUE;
            
            // Check the client for sure (each message is checked once when it crosses
            // over from incomplete to complete.
            if (!complete && connection->_server->_callbacks.acceptNewRequestCallBack) {
            
                handle = connection->_server->_callbacks.acceptNewRequestCallBack((_CFHTTPServerRef)connection->_server,
                                                                                  msg,
                                                                                  connection->_peer,
                                                                                  connection->_server->_ctxt.info);
            }
            
            if (handle)
                _HttpConnectionHandleRequest(connection);
            
            else {
                
                // Remove the connection from the pool
                _HttpServerRemoveConnection(connection->_server, connection);
            }
        }
	}
	
    // Let error conditions come in naturally through the event dispatch.
}


/* static */ void
_HttpConnectionHandleCanAcceptBytes(HttpConnection* connection) {
	
    // How are responses handled (read the "Description" at the top)?
    //
    // Responses have two parts: a CFHTTPMessageRef containing only headers and a CFReadStreamRef
    // which is a stream to the body contents.  These responses will be sent in the order in
    // which their respective requests were vended.
    //
    // A local buffer on the connection is used for all writing to the wire.  Buffered bytes are
    // always sent first.  An empty buffer signals the start of a new response.  If the buffer is
    // empty, the first, queued response's headers are serialized and placed in the buffer.
    //
    // Bytes in the buffer are sent to the wire.  If all bytes in the buffer were written, the
    // response's stream is read for bytes.  The read bytes are placed into the connection's
    // buffer.  This buffer will be used for writing when this function is called again.
    //
    // If the response's stream has been exhausted, that request-response pair is removed from
    // the connection's queue, and the buffer is left empty.  Since the buffer is empty, the
    // next response will be handled when this function is called again.
    //
    // At the end of each response, the headers are checked for the proper termination of the
    // open connection.  If a "Connection: close" header exists or if in default mode under
    // HTTP version 1.0, the connection will be terminated and dequeued from the server.
    
    // Check to make sure there are queued items.
    if (CFArrayGetCount(connection->_requests) != 0) {
        
        // Pull off the request and its related response information
        CFHTTPMessageRef request = (CFHTTPMessageRef)CFArrayGetValueAtIndex(connection->_requests, 0);
        CFArrayRef list = request ? (CFArrayRef)CFDictionaryGetValue(connection->_responses, request) : NULL;
        CFHTTPMessageRef response = list ? (CFHTTPMessageRef)CFArrayGetValueAtIndex(list, 0) : NULL;
        CFReadStreamRef stream = list ? (CFReadStreamRef)CFArrayGetValueAtIndex(list, 1) : NULL;
        
        // Only handle if there is a response ready to go
        if (list != NULL) {
        
            CFIndex bytesWritten;
        
            // If there are no buffered bytes, need to start the next request.
            if (CFDataGetLength(connection->_bufferedBytes) == 0) {
                
                // Serialize if for sending
                CFDataRef serialized = CFHTTPMessageCopySerializedMessage(response);
                
                // Get rid of the old one before getting the new
                CFRelease(connection->_bufferedBytes);
                
                // Use a mutable copy, because it gets sized down as bytes are sent.
                connection->_bufferedBytes = CFDataCreateMutableCopy(connection->_alloc, 0, serialized);
                
                // Release the original.
                CFRelease(serialized);
            }
            
            // Try writing the entire buffer
            bytesWritten = CFWriteStreamWrite(connection->_outStream,
                                              CFDataGetBytePtr(connection->_bufferedBytes),
                                              CFDataGetLength(connection->_bufferedBytes));
            
            // If successfully wrote, continue on.
            if (bytesWritten > 0) {
            
                // Compute the new size of the buffer after the write
                CFIndex newSize = CFDataGetLength(connection->_bufferedBytes) - bytesWritten;
                
                // Tickle the timer
                CFRunLoopTimerSetNextFireDate(connection->_timer, CFAbsoluteTimeGetCurrent() + kTimeOutInSeconds);
        
                // Move the remaining bytes down in the buffer
                memmove(CFDataGetMutableBytePtr(connection->_bufferedBytes),
                        CFDataGetBytePtr(connection->_bufferedBytes) + bytesWritten,
                        newSize);
                        
                // Resize the buffer to indicate what is left
                CFDataSetLength(connection->_bufferedBytes, newSize);
                
                // If nothing left in the buffer, fill the buffer
                if (newSize == 0) {
                
                    CFIndex bytesRead;
                    
                    // If the response's stream isn't open yet, open it.
                    if (CFReadStreamGetStatus(stream) == kCFStreamStatusNotOpen)
                        CFReadStreamOpen(stream);
            
                    // Size the buffer for a full read
                    CFDataSetLength(connection->_bufferedBytes, kBufferSize);
                    
                    // Try reading a full buffer into the buffer
                    bytesRead = CFReadStreamRead(stream, CFDataGetMutableBytePtr(connection->_bufferedBytes), kBufferSize);
                
                    // Size the buffer to what ever size was read if successful
                    if (bytesRead >= 0)
                        CFDataSetLength(connection->_bufferedBytes, bytesRead);
                    
                    // Was there an error?
                    if (bytesRead < 0) {
                        
                        // Get the error from the read stream
                        CFStreamError error = CFReadStreamGetError(stream);
                        
                        // Inform the client of the error.
                        _HttpConnectionHandleErrorOccurred(connection, &error);
                    }
                    
                    // Was this the end of the response's stream?
                    else if (bytesRead == 0) {
                        
                        // Get the HTTP version and the connection header from the response.
                        CFStringRef close = CFHTTPMessageCopyHeaderFieldValue(response, _kCFHTTPServerConnectionHeader);
                        CFStringRef version = CFHTTPMessageCopyVersion(response);
                        
                        // If no header, check the original request for one.
                        if (close == NULL)
                            close = CFHTTPMessageCopyHeaderFieldValue(request, _kCFHTTPServerConnectionHeader);
                        
                        // Inform the client of a successful send of the response.
                        if (connection->_server->_callbacks.didSendResponseCallBack != NULL) {
                            connection->_server->_callbacks.didSendResponseCallBack((_CFHTTPServerRef)connection->_server,
                                                                                    request,
                                                                                    response,
                                                                                    connection->_server->_ctxt.info);
                        }
                        
                        // Remove the request-response pair from the conneciton's queue
                        CFDictionaryRemoveValue(connection->_responses, request);
                        CFArrayRemoveValueAtIndex(connection->_requests, 0);
                        
                        // If there was a header and it said, "close," or if there was no header and HTTP version
                        // 1.0 is being used, then close the connection and remove it from the server.
                        if (((close != NULL) &&
                            CFStringCompare(close, _kCFHTTPServerConnectionClose, kCFCompareCaseInsensitive) == kCFCompareEqualTo) ||
                        	((close == NULL) && (version != NULL) &&
                            CFStringCompare(version, kCFHTTPVersion1_1, kCFCompareCaseInsensitive) != kCFCompareEqualTo))
                        {
                            _HttpServerRemoveConnection(connection->_server, connection);
                        }
                        if (close != NULL)
                            CFRelease(close);
                            
                        if (version != NULL)
                            CFRelease(version);
                    }
                }
            }
        }
    }
}


/* static */ void
_HttpConnectionHandleErrorOccurred(HttpConnection* connection, const CFStreamError* error) {
    
    CFArrayRef requests = CFArrayCreateCopy(connection->_alloc, connection->_requests);
    CFIndex i, count = CFArrayGetCount(requests);
    
    // Error-out each request in the queue
    for (i = 0; i < count; i++) {

        // Get the request and the response pair
        CFHTTPMessageRef request = (CFHTTPMessageRef)CFArrayGetValueAtIndex(connection->_requests, i);
        CFArrayRef list = (CFArrayRef)CFDictionaryGetValue(connection->_responses, request);
        
        // If there is a response and there is a client, inform the client of the error.
        if ((list != NULL) && (connection->_server->_callbacks.errorCallBack != NULL)) {
            connection->_server->_callbacks.errorCallBack((_CFHTTPServerRef)connection->_server,
                                                          error,
                                                          request,
                                                          (CFHTTPMessageRef)CFArrayGetValueAtIndex(list, 0),
                                                          connection->_server->_ctxt.info);
        }
    }

    CFRelease(requests);
    
    // Remove the connection from the pool
    _HttpServerRemoveConnection(connection->_server, connection);
}


/* static */ void
_HttpConnectionHandleTimeOut(HttpConnection* connection) {
    
    // Establish an error
    CFStreamError error = {kCFStreamErrorDomainCFHTTPServer, kCFStreamErrorCFHTTPServerTimeout};
    
    // Handle it just like an error.
    _HttpConnectionHandleErrorOccurred(connection, &error);
}



/* static */ const void*
_ArrayRetainCallBack(CFAllocatorRef allocator, const HttpConnection* connection) {
    
    return _HttpConnectionRetain((HttpConnection*)connection);
}


/* static */ void
_ArrayReleaseCallBack(CFAllocatorRef allocator, const HttpConnection* connection) {
    
    return _HttpConnectionRelease((HttpConnection*)connection);
}


/* static */ void
_ReadStreamCallBack(CFReadStreamRef inStream, CFStreamEventType type, HttpConnection* connection) {

    assert(inStream == connection->_inStream);
	
    // Dispatch the event properly.
    switch (type) {
    
        case kCFStreamEventHasBytesAvailable:
            _HttpConnectionHandleHasBytesAvailable(connection);
            break;
       
        case kCFStreamEventErrorOccurred:
            {
                CFStreamError error = CFReadStreamGetError(inStream);
                _HttpConnectionHandleErrorOccurred(connection, &error);
            }
            break;
            
        default:
            break;
    }
}


/* static */ void
_WriteStreamCallBack(CFWriteStreamRef outStream, CFStreamEventType type, HttpConnection* connection) {

	assert(outStream == connection->_outStream);

	// Dispatch the event properly.
    switch (type) {
		case kCFStreamEventCanAcceptBytes:
			_HttpConnectionHandleCanAcceptBytes(connection);
			break;
			
        case kCFStreamEventErrorOccurred:
            {
                CFStreamError error = CFWriteStreamGetError(outStream);
                _HttpConnectionHandleErrorOccurred(connection, &error);
            }
			break;
            
        default:
            break;
    }
}


/* static */ void
_TimerCallBack(CFRunLoopTimerRef timer, HttpConnection* connection) {

	assert(timer == connection->_timer);

	// Dispatch the timer event.
	_HttpConnectionHandleTimeOut(connection);
}


/* static */ void
_HttpServerAddConnection(HttpServer* server, HttpConnection* connection) {

    // Add the given connection to the list
    CFArrayAppendValue(server->_connections, connection);
}


/* static */ void
_HttpServerRemoveConnection(HttpServer* server, HttpConnection* connection) {
    
    // Find the given connection in the list of connections
    CFMutableArrayRef connections = server->_connections;
    CFIndex i = CFArrayGetFirstIndexOfValue(connections,
                                            CFRangeMake(0, CFArrayGetCount(connections)),
                                            connection);
    
    // If it existed, remove it from the list.
    if (i != kCFNotFound)
        CFArrayRemoveValueAtIndex(connections, i);
}


/* static */ void
_HttpServerHandleNewConnection(HttpServer* server, CFSocketNativeHandle sock) {
    
    CFAllocatorRef alloc = CFGetAllocator((_CFHTTPServerRef)server);
    
    // Assume the server will allow the connection.
    Boolean accepted = TRUE;
    
    // Find out if the client cares
    if (server->_callbacks.acceptNewConnectionCallBack) {
        
        uint8_t name[SOCK_MAXADDRLEN];
        socklen_t namelen = sizeof(name);
        CFDataRef peer = NULL;
        
        // Get the address of the peer.  **FIXME** this is less than optimal
        // since the peer name is copied again later when the connection is
        // created.
        if (0 == getpeername(sock, (struct sockaddr *)name, &namelen))
            peer = CFDataCreate(alloc, name, namelen);
        
        // Fail if the peer couldn't be established.
        if (!peer)
            accepted = FALSE;
            
        else {
        
            // See what the client says.
            accepted = server->_callbacks.acceptNewConnectionCallBack((_CFHTTPServerRef)server, peer, server->_ctxt.info);
            CFRelease(peer);
        }
    }
    
    if (accepted) {
    
        // Create a new incoming connection
        HttpConnection* connection = _HttpConnectionCreate(alloc, server, sock);
        
        // Add the connection to the server if it created.
        if (connection != NULL) {
            _HttpServerAddConnection(server, connection);
            _HttpConnectionRelease(connection);
        }
            
        else {
            
            // Create an error for the bad situation
            CFStreamError error = {kCFStreamErrorDomainCFHTTPServer, kCFStreamErrorCFHTTPServerInternal};
            
            // Handle the error
            _HttpServerHandleError(server, &error);
        }
    }
}


/* static */ void
_HttpServerHandleError(HttpServer* server, const CFStreamError* error) {

	// Inform the user of an error.
	if (server->_callbacks.errorCallBack != NULL)
		server->_callbacks.errorCallBack((_CFHTTPServerRef)server, error, NULL, NULL, server->_ctxt.info);
}


/* static */ void
_ServerCallBack(_CFServerRef server, CFSocketNativeHandle sock, const CFStreamError* error, HttpServer* httpServer) {

    if (error->error == 0)
        _HttpServerHandleNewConnection(httpServer, sock);
        
    else
        _HttpServerHandleError(httpServer, error);
}


/* static */ CFNumberRef
_CFNumberCreateWithString(CFAllocatorRef allocator, CFStringRef string) {

	CFIndex i, length = CFStringGetLength(string);
	UniChar* buffer = CFAllocatorAllocate(allocator, length * sizeof(buffer[0]), 0);
	
	SInt32 value = 0;
	
	CFStringGetCharacters(string, CFRangeMake(0, length), buffer);
	
	for (i = 0; i < length; i++) {
	
		UniChar c = buffer[i];
		
		if ((c < '0') || (c > '9') || ((value * 10) < value)) {
			CFAllocatorDeallocate(allocator, buffer);
			return NULL;
		}
		
		value *= 10;
		value += (c - '0');
	}
	
	CFAllocatorDeallocate(allocator, buffer);
	
	return CFNumberCreate(allocator, kCFNumberSInt32Type, &value);
}

