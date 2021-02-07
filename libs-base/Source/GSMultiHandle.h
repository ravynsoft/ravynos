#ifndef	INCLUDED_GSMULTIHANDLE_H
#define	INCLUDED_GSMULTIHANDLE_H

#import "common.h"
#import <curl/curl.h>
#import "GSDispatch.h"

@class NSURLSessionConfiguration;
@class GSEasyHandle;

/*
 * Minimal wrapper around curl multi interface
 * (https://curl.haxx.se/libcurl/c/libcurl-multi.html).
 *
 * The the *multi handle* manages the sockets for easy handles
 * (`GSEasyHandle`), and this implementation uses
 * libdispatch to listen for sockets being read / write ready.
 *
 * Using `dispatch_source_t` allows this implementation to be
 * non-blocking and all code to run on the same thread 
 * thus keeping is simple.
 *
 * - SeeAlso: GSEasyHandle
 */
@interface GSMultiHandle : NSObject
{
  CURLM  *_rawHandle;
}

- (CURLM*) rawHandle;
- (instancetype) initWithConfiguration: (NSURLSessionConfiguration*)configuration 
                             workQueue: (dispatch_queue_t)workQueque;
- (void) addHandle: (GSEasyHandle*)easyHandle;
- (void) removeHandle: (GSEasyHandle*)easyHandle;
- (void) updateTimeoutTimerToValue: (NSInteger)value;

@end

// What read / write ready event to register / unregister.
typedef NS_ENUM(NSUInteger, GSSocketRegisterActionType) {
    GSSocketRegisterActionTypeNone = 0,
    GSSocketRegisterActionTypeRegisterRead,
    GSSocketRegisterActionTypeRegisterWrite,
    GSSocketRegisterActionTypeRegisterReadAndWrite,
    GSSocketRegisterActionTypeUnregister,
};

@interface GSSocketRegisterAction : NSObject
{
  GSSocketRegisterActionType  _type;
}

- (instancetype) initWithRawValue: (int)rawValue;
- (GSSocketRegisterActionType) type;
- (BOOL) needsReadSource;
- (BOOL) needsWriteSource;
- (BOOL) needsSource;

@end

/*
 * Read and write libdispatch sources for a specific socket.
 *
 * A simple helper that combines two sources -- both being optional.
 *
 * This info is stored into the socket using `curl_multi_assign()`.
 *
 * - SeeAlso: GSSocketRegisterAction
 */
@interface GSSocketSources : NSObject
{
  dispatch_source_t _readSource;
  dispatch_source_t _writeSource;
}

- (void) createSourcesWithAction: (GSSocketRegisterAction *)action
                          socket: (curl_socket_t)socket
                           queue: (dispatch_queue_t)queue
                         handler: (dispatch_block_t)handler;
- (void) createReadSourceWithSocket: (curl_socket_t)socket
                              queue: (dispatch_queue_t)queue
                            handler: (dispatch_block_t)handler;
- (void) createWriteSourceWithSocket: (curl_socket_t)socket
                               queue: (dispatch_queue_t)queue
                             handler: (dispatch_block_t)handler;

+ (instancetype) from: (void*)socketSourcePtr;

@end

#endif
