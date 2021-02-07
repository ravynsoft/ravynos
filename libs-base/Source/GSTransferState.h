#ifndef	INCLUDED_GSTRANSFERTSTATE_H
#define	INCLUDED_GSTRANSFERTSTATE_H

#import "GSURLSessionTaskBodySource.h"


@class GSURLSessionTaskBodySource;
@class NSArray;
@class NSData;
@class NSFileHandle;
@class NSHTTPURLResponse;
@class NSURL;
@class NSURLResponse;

typedef NS_ENUM(NSUInteger, GSParsedResponseHeaderType) {
    GSParsedResponseHeaderTypePartial,
    GSParsedResponseHeaderTypeComplete
};

/*
 * A native protocol like HTTP header being parsed.
 *
 * It can either be complete (i.e. the final CR LF CR LF has been
 * received), or partial.
 */
@interface GSParsedResponseHeader: NSObject
{
  NSArray                     *_lines;
  GSParsedResponseHeaderType  _type;
}

- (GSParsedResponseHeaderType) type;

/*
 * Parse a header line passed by libcurl.
 *
 * These contain the <CRLF> ending and the final line contains nothing but
 * that ending.
 * - Returns: Returning nil indicates failure. Otherwise returns a new
 *   `GSParsedResponseHeader` with the given line added.
 */
- (instancetype) byAppendingHeaderLine: (NSData*)data;

- (NSHTTPURLResponse*) createHTTPURLResponseForURL: (NSURL*)URL;

@end

typedef NS_ENUM(NSUInteger, GSDataDrainType) {
    // Concatenate in-memory
    GSDataDrainInMemory,
    // Write to file
    GSDataDrainTypeToFile,
    // Do nothing. Might be forwarded to delegate
    GSDataDrainTypeIgnore,
};

@interface GSDataDrain: NSObject
{
  GSDataDrainType _type;
  NSData          *_data;
  NSURL           *_fileURL;
  NSFileHandle    *_fileHandle;
}

- (GSDataDrainType) type;
- (void) setType: (GSDataDrainType)type;

- (NSData*) data;
- (void) setData: (NSData*)data;

- (NSURL*) fileURL;
- (void) setFileURL: (NSURL*)url;

- (NSFileHandle*) fileHandle;
- (void) setFileHandle: (NSFileHandle*)handle;

@end

/*
 * State related to an ongoing transfer.
 *
 * This contains headers received so far, body data received so far, etc.
 *
 * There's a strict 1-to-1 relationship between a `GSEasyHandle` and a
 * `GSTransferState`.
 */
@interface GSTransferState: NSObject
{
  NSURL                           *_url; // The URL that's being requested
  GSParsedResponseHeader          *_parsedResponseHeader; // Raw headers received.
  NSURLResponse                   *_response; // Once the headers is complete, this will contain the response
  id<GSURLSessionTaskBodySource>  _requestBodySource; // The body data to be sent in the request
  GSDataDrain                     *_bodyDataDrain; // Body data received
  BOOL                            _isHeaderComplete;
}

// Transfer state that can receive body data, but will not send body data.
- (instancetype) initWithURL: (NSURL*)url
               bodyDataDrain: (GSDataDrain*)bodyDataDrain;

// Transfer state that sends body data and can receive body data.
- (instancetype) initWithURL: (NSURL*)url
               bodyDataDrain: (GSDataDrain*)bodyDataDrain
                  bodySource: (id<GSURLSessionTaskBodySource>)bodySource;

- (instancetype) initWithURL: (NSURL*)url
        parsedResponseHeader: (GSParsedResponseHeader*)parsedResponseHeader
                    response: (NSURLResponse*)response
                  bodySource: (id<GSURLSessionTaskBodySource>)bodySource
               bodyDataDrain: (GSDataDrain*)bodyDataDrain;

/*
 * Append body data
 */
- (instancetype) byAppendingBodyData: (NSData*)bodyData;

/*
 * Appends a header line
 *
 * Will set the complete response once the header is complete, i.e. the
 * return value's `isHeaderComplete` will then by `YES`.
 *
 * When a parsing error occurs `error` will be set.
 */
- (instancetype) byAppendingHTTPHeaderLineData: (NSData*)data 
                                         error: (NSError**)error;

- (NSURLResponse*) response;

- (void) setResponse: (NSURLResponse*)response;

- (BOOL) isHeaderComplete; 

- (id<GSURLSessionTaskBodySource>) requestBodySource;

- (GSDataDrain*) bodyDataDrain;

- (NSURL*) URL;

@end

#endif
