#ifndef	INCLUDED_GSEASYHANDLE_H
#define	INCLUDED_GSEASYHANDLE_H

#import "common.h"
#import <curl/curl.h>

@class NSData;
@class NSError;
@class NSURL;
@class NSURLSessionConfiguration;
@class NSURLSessionTask;
@class GSTimeoutSource;

typedef NS_ENUM(NSUInteger, GSEasyHandleAction) {
    GSEasyHandleActionAbort,
    GSEasyHandleActionProceed,
    GSEasyHandleActionPause,
};

typedef NS_ENUM(NSUInteger, GSEasyHandleWriteBufferResult) {
    GSEasyHandleWriteBufferResultAbort,
    GSEasyHandleWriteBufferResultPause,
    GSEasyHandleWriteBufferResultBytes,
};

@protocol GSEasyHandleDelegate <NSObject>

/* 
 * Handle data read from the network.
 * - returns: the action to be taken: abort, proceed, or pause.
 */
- (GSEasyHandleAction) didReceiveData: (NSData*)data;

/*
 * Handle header data read from the network.
 * - returns: the action to be taken: abort, proceed, or pause.
 */
- (GSEasyHandleAction) didReceiveHeaderData: (NSData*)data 
                              contentLength: (int64_t)contentLength;

/*
 * Fill a buffer with data to be sent.
 * - parameter data: The buffer to fill
 * - returns: the number of bytes written to the `data` buffer, or `nil` 
 *   to stop the current transfer immediately.
 */
- (void) fillWriteBufferLength: (NSInteger)length
                        result: (void (^)(GSEasyHandleWriteBufferResult result, NSInteger length, NSData *data))result;

/*
 * The transfer for this handle completed.
 * - parameter errorCode: An NSURLError code, or `nil` if no error occurred.
 */
- (void) transferCompletedWithError: (NSError*)error;

/*
 * Seek the input stream to the given position
 */
- (BOOL) seekInputStreamToPosition: (uint64_t)position;

/*
 * Gets called during the transfer to update progress.
 */
- (void) updateProgressMeterWithTotalBytesSent: (int64_t)totalBytesSent 
                      totalBytesExpectedToSend: (int64_t)totalBytesExpectedToSend 
                            totalBytesReceived: (int64_t)totalBytesReceived 
                   totalBytesExpectedToReceive: (int64_t)totalBytesExpectedToReceive;

@end


/* 
 * Minimal wrapper around the curl easy interface 
 * (https://curl.haxx.se/libcurl/c/)
 *
 * An *easy handle* manages the state of a transfer inside libcurl.
 *
 * As such the easy handle's responsibility is implementing the HTTP
 * protocol while the *multi handle* is in charge of managing sockets and
 * reading from / writing to these sockets.
 *
 * An easy handle is added to a multi handle in order to associate it with
 * an actual socket. The multi handle will then feed bytes into the easy
 * handle and read bytes from the easy handle. But this process is opaque
 * to use. It is further worth noting, that with HTTP/1.1 persistent
 * connections and with HTTP/2 there's a 1-to-many relationship between
 * TCP streams and HTTP transfers / easy handles. A single TCP stream and
 * its socket may be shared by multiple easy handles.
 *
 * A single HTTP request-response exchange (refered to here as a
 * *transfer*) corresponds directly to an easy handle. Hence anything that
 * needs to be configured for a specific transfer (e.g. the URL) will be
 * configured on an easy handle.
 *
 * A single `NSURLSessionTask` may do multiple, consecutive transfers, and
 * as a result it will have to reconfigure its easy handle between
 * transfers. An easy handle can be re-used once its transfer has
 * completed.
 *
 * Note: All code assumes that it is being called on a single thread,
 * it is intentionally **not** thread safe.
 */
@interface GSEasyHandle : NSObject
{
  CURL                      *_rawHandle;
  char                      *_errorBuffer;
  id<GSEasyHandleDelegate>  _delegate;
  GSTimeoutSource           *_timeoutTimer;
  NSURL                     *_URL;
}

- (CURL*) rawHandle;

- (char*) errorBuffer;

/*
 * Set error buffer for error messages
 * - SeeAlso: https://curl.haxx.se/libcurl/c/CURLOPT_ERRORBUFFER.html
 */
- (void) setErrorBuffer: (char*)buffer;

- (GSTimeoutSource*) timeoutTimer;

- (void) setTimeoutTimer: (GSTimeoutSource*)timer;

- (NSURL*) URL;

/*
 * URL to use in the request
 * - SeeAlso: https://curl.haxx.se/libcurl/c/CURLOPT_URL.html
 */
- (void) setURL: (NSURL*)URL;

- (void) setPipeWait: (BOOL)flag;

- (instancetype) initWithDelegate: (id<GSEasyHandleDelegate>)delegate;

- (void) transferCompletedWithError: (NSError*)error;

- (int) urlErrorCodeWithEasyCode: (int)easyCode;

- (void) setVerboseMode: (BOOL)flag;

- (void) setDebugOutput: (BOOL)flag 
                   task: (NSURLSessionTask*)task;

- (void) setPassHeadersToDataStream: (BOOL)flag;

/*
 * Follow any Location: header that the server sends as part of a HTTP header 
 * in a 3xx response
 */
- (void) setFollowLocation: (BOOL)flag;

/*
 * Switch off the progress meter. 
 */
- (void) setProgressMeterOff: (BOOL)flag;

/*
 * Skip all signal handling
 * - SeeAlso: https://curl.haxx.se/libcurl/c/CURLOPT_NOSIGNAL.html
 */
- (void) setSkipAllSignalHandling: (BOOL)flag;

/* 
 * Request failure on HTTP response >= 400
 */
- (void) setFailOnHTTPErrorCode: (BOOL)flag;

- (void) setConnectToHost: (NSString*)host 
                     port: (NSInteger)port;

- (void) setSessionConfig: (NSURLSessionConfiguration*)config;

- (void) setAllowedProtocolsToHTTPAndHTTPS;

/*
 * set preferred receive buffer size
 * - SeeAlso: https://curl.haxx.se/libcurl/c/CURLOPT_BUFFERSIZE.html
 */
- (void) setPreferredReceiveBufferSize: (NSInteger)size;

/*
 * Set custom HTTP headers
 * - SeeAlso: https://curl.haxx.se/libcurl/c/CURLOPT_HTTPHEADER.html
 */
- (void) setCustomHeaders: (NSArray*)headers;

/*
 * Enable automatic decompression of HTTP downloads
 * - SeeAlso: https://curl.haxx.se/libcurl/c/CURLOPT_ACCEPT_ENCODING.html
 * - SeeAlso: https://curl.haxx.se/libcurl/c/CURLOPT_HTTP_CONTENT_DECODING.html
 */
- (void) setAutomaticBodyDecompression: (BOOL)flag;

/*
 * Set request method
 * - SeeAlso: https://curl.haxx.se/libcurl/c/CURLOPT_CUSTOMREQUEST.html
 */
- (void) setRequestMethod:(NSString*)method;

/*
 * Download request without body
 * - SeeAlso: https://curl.haxx.se/libcurl/c/CURLOPT_NOBODY.html
 */
- (void) setNoBody: (BOOL)flag;

/*
 * Enable data upload
 * - SeeAlso: https://curl.haxx.se/libcurl/c/CURLOPT_UPLOAD.html
 */
- (void) setUpload: (BOOL)flag;

/*
 * Set size of the request body to send
 * - SeeAlso: https://curl.haxx.se/libcurl/c/CURLOPT_INFILESIZE_LARGE.html
 */
- (void) setRequestBodyLength: (int64_t)length;

- (void) setTimeout: (NSInteger)timeout;

- (void) setProxy;

- (double) getTimeoutIntervalSpent;


- (void) pauseReceive;
- (void) unpauseReceive;

- (void) pauseSend;
- (void) unpauseSend;

@end

#endif
