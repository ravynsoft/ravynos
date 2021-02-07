#import "GSURLPrivate.h"
#import "GSHTTPURLProtocol.h"
#import "GSTransferState.h"
#import "GSURLSessionTaskBody.h"
#import "GSTimeoutSource.h"

#import "Foundation/FoundationErrors.h"
#import "Foundation/NSCharacterSet.h"
#import "Foundation/NSDateFormatter.h"
#import "Foundation/NSError.h"
#import "Foundation/NSOperation.h"
#import "Foundation/NSSet.h"
#import "Foundation/NSStream.h"
#import "Foundation/NSURL.h"
#import "Foundation/NSURLError.h"
#import "Foundation/NSURLSession.h"
#import "Foundation/NSValue.h"


@interface NSURLSessionTask (Internal)

- (void) setCountOfBytesExpectedToReceive: (int64_t)count;

- (void) setCountOfBytesExpectedToSend: (int64_t)count;

- (dispatch_queue_t) workQueue;

@end

@implementation NSURLSessionTask (Internal)

- (void) setCountOfBytesExpectedToReceive: (int64_t)count
{
  _countOfBytesExpectedToReceive = count;
}

- (void) setCountOfBytesExpectedToSend: (int64_t)count
{
  _countOfBytesExpectedToSend = count;
}

- (GSURLSessionTaskBody*) knownBody
{
  return _knownBody;
}

- (dispatch_queue_t) workQueue
{
  return _workQueue;
}

@end

@interface GSURLCacherHelper : NSObject

+ (BOOL) canCacheResponse: (NSCachedURLResponse*)response 
                  request: (NSURLRequest*)request;

@end

static NSDate*
dateFromString(NSString *v) 
{
  // https://tools.ietf.org/html/rfc2616#section-3.3.1
  NSDateFormatter *df;
  NSDate          *d;

  df = AUTORELEASE([[NSDateFormatter alloc] init]);

  // RFC 822
  [df setDateFormat: @"EEE, dd MMM yyyy HH:mm:ss zzz"];
  d = [df dateFromString: v];
  if (nil != d) 
    {
      return d;
    } 

  // RFC 850
  [df setDateFormat: @"EEEE, dd-MMM-yy HH:mm:ss zzz"];
  d = [df dateFromString: v];
  if (nil != d) 
    {
      return d;
    } 

  // ANSI C's asctime() format
  [df setDateFormat: @"EEE MMM dd HH:mm:ss yy"];
  d = [df dateFromString: v];
  if (nil != d) 
    {
      return d;
    } 

  return nil;
}

static NSInteger
parseArgumentPart(NSString *part, NSString *name) 
{
  NSString *prefix;
  
  prefix = [NSString stringWithFormat: @"%@=", name];
  if ([part hasPrefix: prefix]) 
    {
      NSArray *split;
      
      split = [part componentsSeparatedByString: @"="];
      if (split && [split count] == 2) 
        {
          NSString *argument = split[1];

          if ([argument hasPrefix: @"\""] && [argument hasSuffix: @"\""]) 
            {
              if ([argument length] >= 2) 
                {
                  NSRange range = NSMakeRange(1, [argument length] - 2);
                  argument = [argument substringWithRange: range];
                  return [argument integerValue];
                } 
              else
                {
                  return 0;
                }
            } 
          else 
            {
              return [argument integerValue];
            }
        }
    }
  
  return 0;
}


@implementation GSURLCacherHelper

+ (BOOL) canCacheResponse: (NSCachedURLResponse*)response 
                  request: (NSURLRequest*)request
{
  NSURLRequest       *httpRequest = request;
  NSHTTPURLResponse  *httpResponse = nil;
  NSDate             *now;
  NSDate             *expirationStart;
  NSString           *dateString;
  NSDictionary       *headers;
  BOOL               hasCacheControl = NO;
  BOOL               hasMaxAge = NO;
  NSString           *cacheControl;
  NSString           *pragma;
  NSString           *expires;

  if (nil == httpRequest)
    {
      return NO;
    } 

  if ([[response response] isKindOfClass: [NSHTTPURLResponse class]]) 
    {
      httpResponse = (NSHTTPURLResponse*)[response response];
    }

  if (nil == httpResponse)
    {
      return NO;
    } 

  // HTTP status codes: https://tools.ietf.org/html/rfc7231#section-6.1
  switch ([httpResponse statusCode]) 
    {
      case 200:
      case 203:
      case 204:
      case 206:
      case 300:
      case 301:
      case 404:
      case 405:
      case 410:
      case 414:
      case 501:
          break;

      default:
          return NO;
    }

  headers = [httpResponse allHeaderFields];

  // Vary: https://tools.ietf.org/html/rfc7231#section-7.1.4
  if (nil != [headers objectForKey: @"Vary"]) 
    {
      return NO;
    }

  now = [NSDate date];
  dateString = [headers objectForKey: @"Date"];
  if (nil != dateString) 
    {
      expirationStart = dateFromString(dateString);
    } 
  else 
    {
      return NO;
    }

  // We opt not to cache any requests or responses that contain authorization headers.
  if ([headers objectForKey: @"WWW-Authenticate"] 
    || [headers objectForKey: @"Proxy-Authenticate"] 
    || [headers objectForKey: @"Authorization"] 
    || [headers objectForKey: @"Proxy-Authorization"]) 
    {
      return NO;
    }

  // HTTP Methods: https://tools.ietf.org/html/rfc7231#section-4.2.3
  if ([[httpRequest HTTPMethod] isEqualToString: @"GET"]) 
    {
    } 
  else if ([[httpRequest HTTPMethod] isEqualToString: @"HEAD"]) 
    {
      if ([response data] && [[response data] length] > 0) 
        {
          return NO;
        }
    } 
  else 
    {
      return NO;
    }

  // Cache-Control: https://tools.ietf.org/html/rfc7234#section-5.2
  cacheControl = [headers objectForKey: @"Cache-Control"];
  if (nil != cacheControl) 
    {
      NSInteger  maxAge = 0;
      NSInteger  sharedMaxAge = 0;
      BOOL       noCache = NO;
      BOOL       noStore = NO;

      [self getCacheControlDeirectivesFromHeaderValue: cacheControl
                                               maxAge: &maxAge
                                         sharedMaxAge: &sharedMaxAge
                                              noCache: &noCache
                                              noStore: &noStore];
      if (noCache || noStore) 
        {
          return NO;
        }

      if (maxAge > 0) 
        {
          NSDate	*expiration;

          hasMaxAge = YES;

          expiration = [expirationStart dateByAddingTimeInterval: maxAge];
          if ([now timeIntervalSince1970] >= [expiration timeIntervalSince1970])
            {
              return NO;
            }
        }

      if (sharedMaxAge)
        {
          hasMaxAge = YES;
        } 
      
      hasCacheControl = YES;
    }

  // Pragma: https://tools.ietf.org/html/rfc7234#section-5.4
  pragma = [headers objectForKey: @"Pragma"];
  if (!hasCacheControl && nil != pragma) 
    {
      NSArray         *cs = [pragma componentsSeparatedByString: @","];
      NSMutableArray  *components = [NSMutableArray arrayWithCapacity: [cs count]];
      NSString        *c;

      for (int i = 0; i < [cs count]; i++)
        {
          c = [cs objectAtIndex: i];
          c = [c stringByTrimmingCharactersInSet: 
            [NSCharacterSet whitespaceCharacterSet]];
          c = [c lowercaseString];
          [components setObject: c atIndexedSubscript: i];
        }
      
      if ([components containsObject: @"no-cache"]) 
        {
          return NO;
        }
    }

  // Expires: <https://tools.ietf.org/html/rfc7234#section-5.3>
  // We should not cache a response that has already expired.
  // We MUST ignore this if we have Cache-Control: max-age or s-maxage.
  expires = [headers objectForKey: @"Expires"];
  if (!hasMaxAge && nil != expires) 
    {
      NSDate *expiration = dateFromString(expires);
      if (nil == expiration)
        {
          return NO;
        }

      if ([now timeIntervalSince1970] >= [expiration timeIntervalSince1970]) 
        {
          return NO;
        }
    }

  if (!hasCacheControl) 
    {
      return NO;
    }

  return YES;
}

+ (void) getCacheControlDeirectivesFromHeaderValue: (NSString*)headerValue
                                            maxAge: (NSInteger*)maxAge
                                      sharedMaxAge: (NSInteger*)sharedMaxAge
                                           noCache: (BOOL*)noCache
                                           noStore: (BOOL*)noStore 
{
    NSArray       *components;
    NSEnumerator  *e;
    NSString      *part;
    
    components = [headerValue componentsSeparatedByString: @","];
    e = [components objectEnumerator];
    while (nil != (part = [e nextObject]))
      {
        part = [part stringByTrimmingCharactersInSet: 
          [NSCharacterSet whitespaceCharacterSet]];
        part = [part lowercaseString];

        if ([part isEqualToString: @"no-cache"]) 
          {
            *noCache = YES;
          }
        else if ([part isEqualToString: @"no-store"]) 
          {
            *noStore = YES;
          }
        else if ([part containsString: @"max-age"]) 
          {
            *maxAge = parseArgumentPart(part, @"max-age");
          } 
        else if ([part containsString: @"s-maxage"]) 
          {
            *sharedMaxAge = parseArgumentPart(part, @"s-maxage");
          } 
      }
}

@end

@implementation GSHTTPURLProtocol

+ (BOOL) canInitWithRequest: (NSURLRequest*)request
{
  NSURL  *url;

  if (nil != (url = [request URL]) 
    && ([[url scheme] isEqualToString: @"http"]
    || [[url scheme] isEqualToString: @"https"]))
    {
      return YES;
    }
  else
    {
      return NO;
    }
}

+ (id<NSURLProtocolClient>) _ProtocolClient
{
  return AUTORELEASE([[_NSURLProtocolClient alloc] init]);
}

- (GSEasyHandleAction) didReceiveHeaderData: (NSData*)data 
                              contentLength: (int64_t)contentLength
{
  NSURLSessionTask  *task;
  GSTransferState   *newTS;
  NSError           *error = NULL;

  NSAssert(_internalState == GSNativeProtocolInternalStateTransferInProgress,
    @"Received header data, but no transfer in progress.");

  task = [self task];
  NSAssert(nil != task, @"Received header data but no task available.");

  newTS = [_transferState byAppendingHTTPHeaderLineData: data error: &error];
  if (nil != newTS && NULL == error)
    {
      BOOL didCompleteHeader;

      didCompleteHeader = ![_transferState isHeaderComplete] 
        && [newTS isHeaderComplete];
      [self setInternalState: GSNativeProtocolInternalStateTransferInProgress];
      ASSIGN(_transferState, newTS);
      if (didCompleteHeader)
        {
          // The header is now complete, but wasn't before.
          NSHTTPURLResponse  *response;
          NSString           *contentEncoding;

          response = (NSHTTPURLResponse*)[newTS response];
          contentEncoding = [[response allHeaderFields] 
            objectForKey: @"Content-Encoding"];
          if (nil != contentEncoding
            && ![contentEncoding isEqual: @"identity"])
            {
              // compressed responses do not report expected size
              [task setCountOfBytesExpectedToReceive: -1];
            }
          else
            {
              [task setCountOfBytesExpectedToReceive: 
                (contentLength > 0 ? contentLength : -1)];
            }
          [self didReceiveResponse];
        }
      return GSEasyHandleActionProceed;
    }
  else
    {
      return GSEasyHandleActionAbort;
    }
}

- (BOOL) canRespondFromCacheUsing: (NSCachedURLResponse*)response
{
  BOOL              canCache;
  NSURLSessionTask  *task;

  task = [self task];

  canCache = [GSURLCacherHelper canCacheResponse: response 
                                         request: [task currentRequest]];
  if (!canCache)
    {
      // If somehow cached a response that shouldn't have been,
      // we should remove it.
      NSURLCache  *cache;

      cache = [[[task session] configuration] URLCache];
      if (nil != cache)
        {
          [cache removeCachedResponseForRequest: [task currentRequest]];
        }

      return NO;
    }
  
  return YES;
}

/// Set options on the easy handle to match the given request.
/// This performs a series of `curl_easy_setopt()` calls.
- (void) configureEasyHandleForRequest: (NSURLRequest*)request 
                                  body: (GSURLSessionTaskBody*)body
{
  NSURLSessionTask  		*task = [self task];
  NSURLSession			*session = [task session];
  NSURLSessionConfiguration	*config = [session configuration];

  if ([[request HTTPMethod] isEqualToString:@"GET"]) 
    {
      if ([body type] != GSURLSessionTaskBodyTypeNone) 
        {
          NSError       *error;
          NSDictionary  *info;

          info = [NSDictionary dictionaryWithObjectsAndKeys:
            @"resource exceeds maximum size", NSLocalizedDescriptionKey,
            [[request URL] description], NSURLErrorFailingURLStringErrorKey,
            nil];
          error = [NSError errorWithDomain: NSURLErrorDomain
                                      code: NSURLErrorDataLengthExceedsMaximum
                                  userInfo: info];
          [self setInternalState: GSNativeProtocolInternalStateTransferFailed];
          [self transferCompletedWithError: error];
          return;
        }
    }

  BOOL debugLibcurl = [[[NSProcessInfo processInfo] environment] 
    objectForKey: @"URLSessionDebugLibcurl"] ? YES : NO;

  /* Programatically turning debug on in the request supercedes any
   * environment variable setting.
   */
  if ([request _debug])
    {
      debugLibcurl = YES;
    }
  [_easyHandle setVerboseMode: debugLibcurl];

  BOOL debugOutput = [[[NSProcessInfo processInfo] environment] 
    objectForKey: @"URLSessionDebug"] ? YES : NO;
  [_easyHandle setDebugOutput: debugOutput task: task];
  
  [_easyHandle setPassHeadersToDataStream: NO];
  [_easyHandle setProgressMeterOff: YES];
  [_easyHandle setSkipAllSignalHandling: YES];

  // Error Options:
  [_easyHandle setErrorBuffer: NULL];
  [_easyHandle setFailOnHTTPErrorCode: NO];

  NSAssert(nil != [request URL], @"No URL in request.");
  [_easyHandle setURL: [request URL]];

  [_easyHandle setPipeWait: [config HTTPShouldUsePipelining]];

  [_easyHandle setSessionConfig: config];
  [_easyHandle setAllowedProtocolsToHTTPAndHTTPS];
  [_easyHandle setPreferredReceiveBufferSize: NSIntegerMax];

  NSError  *e = nil;
  NSNumber *bodySize = [body getBodyLengthWithError: &e];
  if (nil != e) 
    {
      NSInteger errorCode;
      NSError   *error;

      [self setInternalState: GSNativeProtocolInternalStateTransferFailed];
      errorCode = [self errorCodeFromFileSystemError: e];
      error = [NSError errorWithDomain: NSURLErrorDomain
                                  code: errorCode
                              userInfo: @{NSLocalizedDescriptionKey : @"File system error"}];
      [self failWithError: error request: request];
      return;
    }

  if ([body type] == GSURLSessionTaskBodyTypeNone) 
    {
      if ([[request HTTPMethod] isEqualToString: @"GET"]) 
        {
          [_easyHandle setUpload: NO];
          [_easyHandle setRequestBodyLength: 0];
        } 
      else 
        {
          [_easyHandle setUpload: YES];
          [_easyHandle setRequestBodyLength: 0];
        }
    } 
  else if (bodySize != nil) 
    {
      [task setCountOfBytesExpectedToSend: [bodySize longLongValue]];
      [_easyHandle setUpload: YES];
      [_easyHandle setRequestBodyLength: [bodySize unsignedLongLongValue]];
    } 
  else if (bodySize == nil) 
    {
      [_easyHandle setUpload: YES];
      [_easyHandle setRequestBodyLength:-1];
    }

  [_easyHandle setFollowLocation: NO];

  /* The httpAdditionalHeaders from session configuration has to be added to 
   * the request. The request.allHTTPHeaders can override the 
   * httpAdditionalHeaders elements. Add the httpAdditionalHeaders from session 
   * configuration first and then append/update the request.allHTTPHeaders 
   * so that request.allHTTPHeaders can override httpAdditionalHeaders.
   */
  NSMutableDictionary *hh = [NSMutableDictionary dictionary];
  NSDictionary        *HTTPAdditionalHeaders;
  NSDictionary        *HTTPHeaders;
  
  hh = [NSMutableDictionary dictionary];
  HTTPAdditionalHeaders
    = [[[task session] configuration] HTTPAdditionalHeaders];
  if (nil == HTTPAdditionalHeaders)
    {
      HTTPAdditionalHeaders = [NSDictionary dictionary];
    }
  HTTPHeaders = [request allHTTPHeaderFields];
  if (nil == HTTPHeaders)
    {
      HTTPHeaders = [NSDictionary dictionary];
    }

  [hh addEntriesFromDictionary: 
    [self transformLowercaseKeyForHTTPHeaders: HTTPAdditionalHeaders]];
  [hh addEntriesFromDictionary: 
    [self transformLowercaseKeyForHTTPHeaders: HTTPHeaders]];

  NSArray *curlHeaders = [self curlHeadersForHTTPHeaders: hh];
  if ([[request HTTPMethod] isEqualToString:@"POST"] 
    && [[request HTTPBody] length] > 0
    && [request valueForHTTPHeaderField: @"Content-Type"] == nil) 
    {
      NSMutableArray *temp = [curlHeaders mutableCopy];
      [temp addObject: @"Content-Type:application/x-www-form-urlencoded"];
      curlHeaders = temp;
    }
  [_easyHandle setCustomHeaders: curlHeaders];
  RELEASE(curlHeaders);

  NSInteger        timeoutInterval = [request timeoutInterval] * 1000;
  GSTimeoutSource  *timeoutTimer;

  timeoutTimer = [[GSTimeoutSource alloc] initWithQueue: [task workQueue] 
                                           milliseconds: timeoutInterval 
                                                handler: 
    ^{
      NSError                 *urlError;
      id<NSURLProtocolClient> client;
      
      [self setInternalState: GSNativeProtocolInternalStateTransferFailed];
      urlError = [NSError errorWithDomain: NSURLErrorDomain 
                                     code: NSURLErrorTimedOut 
                                 userInfo: nil];
      [self completeTaskWithError: urlError];
      if (nil != (client = [self client]) 
        && [client respondsToSelector: @selector(URLProtocol:didFailWithError:)])
        {
          [client URLProtocol: self didFailWithError: urlError];
        }
    }];
  [_easyHandle setTimeoutTimer: timeoutTimer];
  RELEASE(timeoutTimer);

  [_easyHandle setAutomaticBodyDecompression: YES];
  [_easyHandle setRequestMethod: 
    [request HTTPMethod] ? [request HTTPMethod] : @"GET"];

  // always set the status as it may change if a HEAD is converted to a GET
  [_easyHandle setNoBody: [[request HTTPMethod] isEqualToString: @"HEAD"]];
  [_easyHandle setProxy];
}

- (GSCompletionAction*) completeActionForCompletedRequest: (NSURLRequest*)request
                                                 response: (NSURLResponse*)response
{
  GSCompletionAction  *action;
  NSHTTPURLResponse   *httpResponse;
  NSURLRequest        *redirectRequest;

  NSAssert([response isKindOfClass: [NSHTTPURLResponse class]], 
    @"Response was not NSHTTPURLResponse");
  httpResponse = (NSHTTPURLResponse*)response;

  redirectRequest = [self redirectRequestForResponse: httpResponse 
                                         fromRequest: request];
  
  action = AUTORELEASE([[GSCompletionAction alloc] init]);

  if (nil != redirectRequest)
    {
      [action setType: GSCompletionActionTypeRedirectWithRequest];
      [action setRedirectRequest: redirectRequest];
    }
  else
    {
      [action setType: GSCompletionActionTypeCompleteTask];
    }

  return action;
}

/* If the response is a redirect, return the new request
 *
 * RFC 7231 section 6.4 defines redirection behavior for HTTP/1.1
 *
 * - SeeAlso: <https://tools.ietf.org/html/rfc7231#section-6.4>
 */
- (NSURLRequest*) redirectRequestForResponse: (NSHTTPURLResponse*)response 
                                 fromRequest: (NSURLRequest*)fromRequest 
{
  NSString  *method = nil;
  NSURL     *targetURL;
  NSString  *location;

  if (nil == [response allHeaderFields]) 
    {
      return nil;
    }

  location = [[response allHeaderFields] objectForKey: @"Location"];
  targetURL = [NSURL URLWithString: location];
  if (nil == location && nil == targetURL)
    {
      return nil;
    }

  switch ([response statusCode]) 
    {
      case 301:
      case 302:
        method = [[fromRequest HTTPMethod] isEqualToString:@"POST"] ? 
          @"GET" : [fromRequest HTTPMethod];
        break;
      case 303:
        method = @"GET";
        break;
      case 305:
      case 306:
      case 307:
      case 308:
        method = nil != [fromRequest HTTPMethod] ?
          [fromRequest HTTPMethod] : @"GET";
        break;
      default:
        return nil;
   }

  NSMutableURLRequest *request = AUTORELEASE([fromRequest mutableCopy]);
  [request setHTTPMethod: method];

  if (nil != [targetURL scheme] && nil != [targetURL host]) 
    {
      [request setURL: targetURL];
      return request;
    }

  NSString *scheme = [[request URL] scheme];
  NSString *host = [[request URL] host];
  NSNumber *port = [[request URL] port];

  NSURLComponents *components = [[NSURLComponents alloc] init];
  [components setScheme: scheme];
  [components setHost: host];
  
  /* Use the original port if the new URL does not contain a host
   * ie Location: /foo => <original host>:<original port>/Foo
   * but Location: newhost/foo  will ignore the original port
   */
  if ([targetURL host] == nil) 
    {
      [components setPort: port];
    }

  /* The path must either begin with "/" or be an empty string.
   */
  if (![[targetURL relativePath] hasPrefix:@"/"]) 
    {
      [components setPath: 
        [NSString stringWithFormat:@"/%@", [targetURL relativePath]]];
    } 
  else 
    {
      [components setPath: [targetURL relativePath]];
    }

  NSString *urlString = [components string];
  RELEASE(components);
  if (nil == urlString) 
    {
      return nil;
    }

  [request setURL: [NSURL URLWithString:urlString]];
  double timeSpent = [_easyHandle getTimeoutIntervalSpent];
  [request setTimeoutInterval: [fromRequest timeoutInterval] - timeSpent];
  return request;
}

- (void) redirectForRequest: (NSURLRequest*)request 
{
  NSURLSessionTask         	*task;
  NSURLSession             	*session;
  id<NSURLSessionDelegate> 	delegate;

  NSAssert(_internalState == GSNativeProtocolInternalStateTransferCompleted,
    @"Trying to redirect, but the transfer is not complete.");

  task = [self task];
  session = [task session];
  delegate = [session delegate];

  if (nil != delegate
    && [delegate respondsToSelector:@selector(selectr)])
    {
      // At this point we need to change the internal state to note
      // that we're waiting for the delegate to call the completion
      // handler. Then we'll call the delegate callback
      // (willPerformHTTPRedirection). The task will then switch out of
      // its internal state once the delegate calls the completion
      // handler.
      [self setInternalState: GSNativeProtocolInternalStateWaitingForRedirectCompletionHandler];
      [[session delegateQueue] addOperationWithBlock:
        ^{
          id<NSURLSessionTaskDelegate> taskDelegate = 
            (id<NSURLSessionTaskDelegate>)delegate;
          [taskDelegate URLSession: session 
                              task: task 
        willPerformHTTPRedirection: (NSHTTPURLResponse*)[_transferState response] 
                        newRequest: request 
                 completionHandler: ^(NSURLRequest *_Nullable request) {
                                      dispatch_async([task workQueue], ^{
                                          NSAssert(_internalState == GSNativeProtocolInternalStateWaitingForRedirectCompletionHandler, 
                                            @"Received callback for HTTP redirection, but we're not waiting "
                                            @"for it. Was it called multiple times?");
                                          
                                          // If the request is `nil`, we're supposed to treat the current response
                                          // as the final response, i.e. not do any redirection.
                                          // Otherwise, we'll start a new transfer with the passed in request.
                                          if (nil != request) 
                                            {
                                              [self startNewTransferWithRequest: request];
                                            } 
                                          else 
                                            {
                                              [self setInternalState: GSNativeProtocolInternalStateTransferCompleted];
                                              [self completeTask];
                                            }
                                      });
                                    }];
        }];
    }
  else
    {
      NSURLRequest *configuredRequest;
      
      configuredRequest = [[session configuration] configureRequest: request];
      [self startNewTransferWithRequest: configuredRequest];
    } 
}

- (NSURLResponse*) validateHeaderCompleteTransferState: (GSTransferState*)ts 
{
  if (![_transferState isHeaderComplete])
    {
      /* we received body data before CURL tells us that the headers are complete, 
        that happens for HTTP/0.9 simple responses, see
        - https://www.w3.org/Protocols/HTTP/1.0/spec.html#Message-Types
        - https://github.com/curl/curl/issues/467
       */
      return AUTORELEASE([[NSHTTPURLResponse alloc] 
        initWithURL: [ts URL]
        statusCode: 200 
        HTTPVersion: @"HTTP/0.9" 
        headerFields: [NSDictionary dictionary]]); 
    }
  
  return nil;
}

- (NSDictionary*) transformLowercaseKeyForHTTPHeaders: (NSDictionary*)HTTPHeaders 
{
  NSMutableDictionary *result;
  NSEnumerator        *e;
  NSString            *k;

  if (nil == HTTPHeaders)
    {
      return nil;
    }

  result = [NSMutableDictionary dictionary];
  e = [HTTPHeaders keyEnumerator];
  while (nil != (k = [e nextObject]))
    {
      [result setObject: [HTTPHeaders objectForKey: k] 
                 forKey: [k lowercaseString]];
    }
  
  return AUTORELEASE([result copy]);
}

// These are a list of headers that should be passed to libcurl.
//
// Headers will be returned as `Accept: text/html` strings for
// setting fields, `Accept:` for disabling the libcurl default header, or
// `Accept;` for a header with no content. This is the format that libcurl
// expects.
//
// - SeeAlso: https://curl.haxx.se/libcurl/c/CURLOPT_HTTPHEADER.html
- (NSArray*) curlHeadersForHTTPHeaders: (NSDictionary*)HTTPHeaders 
{
  NSMutableArray *result = [NSMutableArray array];
  NSMutableSet   *names = [NSMutableSet set];
  NSEnumerator   *e;
  NSString       *k;
  NSDictionary   *curlHeadersToSet;
  NSArray        *curlHeadersToRemove;

  if (nil == HTTPHeaders)
    {
      return nil;
    }

  e = [HTTPHeaders keyEnumerator];
  while (nil != (k = [e nextObject]))
    {
      NSString *name = [k lowercaseString];
      NSString *value = [HTTPHeaders objectForKey: k];
      
      if ([names containsObject: name])
        {
          break;
        }

      [names addObject: name];
      
      if ([value length] == 0)
        {
          [result addObject: [NSString stringWithFormat: @"%@;", k]];
        } 
      else 
        {
          [result addObject: [NSString stringWithFormat: @"%@: %@", k, value]];
        }
    }

  curlHeadersToSet = [self curlHeadersToSet];
  e = [curlHeadersToSet keyEnumerator];
  while (nil != (k = [e nextObject]))
    {
      NSString *name = [k lowercaseString];
      NSString *value = [curlHeadersToSet objectForKey: k];
      
      if ([names containsObject: name])
        {
          break;
        }

      [names addObject: name];
      
      if ([value length] == 0)
        {
          [result addObject: [NSString stringWithFormat: @"%@;", k]];
        } 
      else 
        {
          [result addObject: [NSString stringWithFormat: @"%@: %@", k, value]];
        }
    }

  curlHeadersToRemove = [self curlHeadersToRemove];
  e = [curlHeadersToRemove objectEnumerator];
  while (nil != (k = [e nextObject]))
    {
      NSString *name = [k lowercaseString];

      if ([names containsObject: name])
        {
          break;
        }

      [names addObject:name];

      [result addObject: [NSString stringWithFormat: @"%@:", k]];
    }

  return AUTORELEASE([result copy]);
}

// Any header values that should be passed to libcurl
//
// These will only be set if not already part of the request.
// - SeeAlso: https://curl.haxx.se/libcurl/c/CURLOPT_HTTPHEADER.html
- (NSDictionary*) curlHeadersToSet 
{
  return [NSDictionary dictionaryWithObjectsAndKeys:
    @"keep-alive", @"Connection",
    [self userAgentString], @"User-Agent",
    nil];  
}

// Any header values that should be removed from the ones set by libcurl
// - SeeAlso: https://curl.haxx.se/libcurl/c/CURLOPT_HTTPHEADER.html
- (NSArray*) curlHeadersToRemove 
{
  if ([[self task] knownBody] == nil) 
    {
      return [NSArray array];
    }
  else if ([[[self task] knownBody] type] == GSURLSessionTaskBodyTypeNone) 
    {
      return [NSArray array];
    }
  
  return [NSArray arrayWithObject: @"Expect"];
}

- (NSString*) userAgentString
{
  NSProcessInfo          *processInfo = [NSProcessInfo processInfo];
  NSString               *name = [processInfo processName];
  curl_version_info_data *curlInfo = curl_version_info(CURLVERSION_NOW);

  return [NSString stringWithFormat: @"%@ (unknown version) curl/%d.%d.%d",
    name, 
    curlInfo->version_num >> 16 & 0xff, 
    curlInfo->version_num >> 8 & 0xff, 
    curlInfo->version_num & 0xff];
}

- (NSInteger) errorCodeFromFileSystemError: (NSError*)error 
{
  if ([error domain] == NSCocoaErrorDomain) 
    {
      switch (error.code) 
        {
          case NSFileReadNoSuchFileError:
            return NSURLErrorFileDoesNotExist;
          case NSFileReadNoPermissionError:
            return NSURLErrorNoPermissionsToReadFile;
          default:
            return NSURLErrorUnknown;
        }
    } 
  else 
    {
      return NSURLErrorUnknown;
    }
}

// Whenever we receive a response (i.e. a complete header) from libcurl,
// this method gets called.
- (void) didReceiveResponse
{
  NSURLSessionDataTask  *task;
  NSHTTPURLResponse     *response;

  task = (NSURLSessionDataTask*)[self task];

  if (![task isKindOfClass: [NSURLSessionDataTask class]])
    {
      return;
    }
  
  NSAssert(_internalState == GSNativeProtocolInternalStateTransferInProgress,
    @"Transfer not in progress.");

  NSAssert([[_transferState response] isKindOfClass: [NSHTTPURLResponse class]],
    @"Header complete, but not URL response.");

  response = (NSHTTPURLResponse*)[_transferState response];

  if (nil != [[task session] delegate])
    {
      switch ([response statusCode])
        {
          case 301:
          case 302:
          case 303:
          case 307:
            break;
          default:
            {
              id<NSURLProtocolClient> client = [self client];

              if (nil != client)
                {
                  [client URLProtocol: self 
                   didReceiveResponse: response 
                   cacheStoragePolicy: NSURLCacheStorageNotAllowed];
                }
            }
        }
    }
}

@end
