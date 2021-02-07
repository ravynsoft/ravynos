/* Interface for NSURLConnection for GNUstep
   Copyright (C) 2006 Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <frm@gnu.org>
   Date: 2006
   
   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
   */ 

#ifndef __NSURLConnection_h_GNUSTEP_BASE_INCLUDE
#define __NSURLConnection_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_2,GS_API_LATEST) && GS_API_VERSION( 11300,GS_API_LATEST)

#import	<Foundation/NSObject.h>
#import <Foundation/NSRunLoop.h>

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSCachedURLResponse;
@class NSData;
@class NSError;
@class NSURLAuthenticationChallenge;
@class NSURLRequest;
@class NSURLResponse;

/**
 */
GS_EXPORT_CLASS
@interface NSURLConnection : NSObject
{
#if	GS_EXPOSE(NSURLConnection)
  void *_NSURLConnectionInternal;
#endif
}

/**
 * Performs a preliminary check to see if a load of the specified
 * request can be handled by an instance of this class.<br />
 * The results of this method may be invalidated by subsequent
 * changes to the request or changes to the registered protocols
 * etc.
 */
+ (BOOL) canHandleRequest: (NSURLRequest *)request;

/**
 * Allocates and returns the autoreleased instance which it initialises
 * using the -initWithRequest:delegate: method.
 */
+ (NSURLConnection *) connectionWithRequest: (NSURLRequest *)request
				   delegate: (id)delegate;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
/**
 * Start the asynchronous load.  This method is only needed if NO is passed 
 * into startImmediately when calling initWithRequest: delegate: startImmediately.
 */ 
- (void) start;
#endif
  
/**
 * Cancel the asynchronous load in progress (if any) for this connection.
 */
- (void) cancel;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
- (void) scheduleInRunLoop: (NSRunLoop *)aRunLoop 
                   forMode: (NSRunLoopMode)mode;

- (void) unscheduleFromRunLoop: (NSRunLoop *)aRunLoop 
                       forMode: (NSRunLoopMode)mode;
#endif
  
/** <init />
 * Initialises the receiver with the specified request (performing
 * a deep copy so that the request does not change during loading)
 * and delegate.<br />
 * This automatically initiates an asynchronous load for the request.<br />
 * Processing of the request is done in the thread which calls this
 * method, so the thread must run its current run loop
 * (in NSDefaultRunLoopMode) for processing to continue/complete.<br />
 * The delegate will receive callbacks informing it of the progress
 * of the load.<br />
 * This method breaks with convention and retains the delegate object,
 * releasing it when the connection finished loading, fails, or is cancelled.
 */
- (id) initWithRequest: (NSURLRequest *)request delegate: (id)delegate;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
/** <init />
 * Initialises the receiver with the specified request (performing
 * a deep copy so that the request does not change during loading)
 * and delegate.<br />
 * This automatically initiates an asynchronous load for the request 
 * if and only if startImmediately is set to YES.<br />
 * Processing of the request is done in the thread which calls this
 * method, so the thread must run its current run loop
 * (in NSDefaultRunLoopMode) for processing to continue/complete.<br />
 * The delegate will receive callbacks informing it of the progress
 * of the load.<br />
 * This method breaks with convention and retains the delegate object,
 * releasing it when the connection finished loading, fails, or is cancelled.
 */
- (id) initWithRequest: (NSURLRequest *)request delegate: (id)delegate startImmediately: (BOOL)startImmediately;
#endif
  
@end



/**
 * This category is an informal protocol specifying how an NSURLConnection
 * instance will communicate with its delegate to inform it of (and allow
 * it to manage) the progress of a load request.<br />
 * A load operation is performed by asynchronous I/O using the
 * run loop of the thread in which it was initiated, so all callbacks
 * will occur in that thread.<br />
 * The process of loading a resource occurs as follows -<br />
 * <list>
 *   <item>
 *     Any number of -connection:willSendRequest:redirectResponse:
 *     messages may be sent to the delegate before any other messages
 *     in this list are sent.  This permits a chain of redirects to
 *     be followed before eventual loading of 'real' data.
 *   </item>
 *   <item>
 *     A -connection:didReceiveAuthenticationChallenge: message may be
 *     sent to the delegate (where authentication is required) before
 *     response data can be downloaded.
 *   </item>
 *   <item>
 *     Any number of -connection:didReceiveResponse: messages
 *     may be be sent to the delegate before a
 *     -connection:didReceiveData: message.  Usually there is exactly one
 *     of these, but for multipart/x-mixed-replace there may be multiple
 *     responses for each part, and if an error occurs in the download
 *     the delegate may not receive a response at all.<br />
 *     Delegates should discard previously handled data when they
 *     receive a new response.
 *   </item>
 *   <item>
 *     Any number of -connection:didReceiveData: messages may
 *     be sent before the load completes as described below.
 *   </item>
 *   <item>
 *     A single -connection:willCacheResponse: message may
 *     be sent to the delegate after any -connection:didReceiveData:
 *     messages are sent but before a -connectionDidFinishLoading: message
 *     is sent.
 *   </item>
 *   <item>
 *     Unless the NSURLConnection receives a -cancel message,
 *     the delegate will receive one and only one of
 *     -connectionDidFinishLoading:, or
 *     -connection:didFailWithError: message, but never
 *     both.<br />
 *     Once either of these terminal messages is sent the
 *     delegate will receive no further messages from the 
 *     NSURLConnection.
 *   </item>
 * </list>
 */
#if OS_API_VERSION(MAC_OS_X_VERSION_10_7,GS_API_LATEST) && GS_API_VERSION(11300,GS_API_LATEST)
@protocol NSURLConnectionDelegate <NSObject>

#if GS_PROTOCOLS_HAVE_OPTIONAL
@optional
#else
@end
@interface NSObject (NSURLConnectionDelegate)
#endif

#else
@interface NSObject (NSURLConnectionDelegate)
#endif

/**
 * Instructs the delegate that authentication for challenge has
 * been cancelled for the request loading on connection.
 */
- (void) connection: (NSURLConnection *)connection
  didCancelAuthenticationChallenge: (NSURLAuthenticationChallenge *)challenge;

/*
 * Called when an NSURLConnection has failed to load successfully.
 */
- (void) connection: (NSURLConnection *)connection
   didFailWithError: (NSError *)error;

/**
 * Called when an NSURLConnection has finished loading successfully.
 */
- (void) connectionDidFinishLoading: (NSURLConnection *)connection;

/**
 * Called when an authentication challenge is received ... the delegate
 * should send -useCredential:forAuthenticationChallenge: or
 * -continueWithoutCredentialForAuthenticationChallenge: or
 * -cancelAuthenticationChallenge: to the challenge sender when done.
 */
- (void) connection: (NSURLConnection *)connection
  didReceiveAuthenticationChallenge: (NSURLAuthenticationChallenge *)challenge;

/**
 * Called when content data arrives during a load operations ... this
 * may be incremental or may be the compolete data for the load.
 */
- (void) connection: (NSURLConnection *)connection
     didReceiveData: (NSData *)data;

/**
 * Called when enough information to build a NSURLResponse object has
 * been received.
 */
- (void) connection: (NSURLConnection *)connection
 didReceiveResponse: (NSURLResponse *)response;

/**
 * Called with the cachedResponse to be stored in the cache.
 * The delegate can inspect the cachedResponse and return a modified
 * copy if if wants changed to what whill be stored.<br />
 * If it returns nil, nothing will be stored in the cache.
 */
- (NSCachedURLResponse *) connection: (NSURLConnection *)connection
  willCacheResponse: (NSCachedURLResponse *)cachedResponse;

/**
 * Informs the delegate that the connection must change the URL of
 * the request in order to continue with the load operation.<br />
 * This allows the delegate to ionspect and/or modify a copy of the request
 * before the connection continues loading it.  Normally the delegate
 * can return the request unmodifield.<br />
 * The redirection can be rejectected by the delegate calling -cancel
 * or returning nil.<br />
 * Cancelling the load will simply stop it, but returning nil will
 * cause it to complete with a redirection failure.<br />
 * As a special case, this method may be called with a nil response,
 * indicating a change of URL made internally by the system rather than
 * due to a response from the server.
 */
- (NSURLRequest *) connection: (NSURLConnection *)connection
	      willSendRequest: (NSURLRequest *)request
	     redirectResponse: (NSURLResponse *)response;
@end

/**
 * An interface to perform synchronous loading of URL requests.
 */
@interface NSURLConnection (NSURLConnectionSynchronousLoading)

/**
 * Performs a synchronous load of request and returns the
 * [NSURLResponse] in response.<br />
 * Returns the result of the load or nil if the load failed.
 */
+ (NSData *) sendSynchronousRequest: (NSURLRequest *)request
		  returningResponse: (NSURLResponse **)response
			      error: (NSError **)error;

@end

#if	defined(__cplusplus)
}
#endif

#endif

#endif
