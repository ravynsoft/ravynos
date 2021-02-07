#import "GSURLPrivate.h"
#import "GSEasyHandle.h"
#import "GSTimeoutSource.h"
#import "Foundation/NSCharacterSet.h"
#import "Foundation/NSURLSession.h"

typedef NS_OPTIONS(NSUInteger, GSEasyHandlePauseState) {
  GSEasyHandlePauseStateReceive = 1 << 0,
  GSEasyHandlePauseStateSend = 1 << 1
};

@interface GSEasyHandle ()

- (void) resetTimer;

- (NSInteger) didReceiveData: (char*)data 
                        size: (NSInteger)size 
                       nmemb:(NSInteger)nmemb;

- (NSInteger) fillWriteBuffer: (char *)buffer 
                         size: (NSInteger)size 
                        nmemb: (NSInteger)nmemb;

- (NSInteger) didReceiveHeaderData: (char*)headerData
                              size: (NSInteger)size
                             nmemb: (NSInteger)nmemb
                     contentLength: (double)contentLength;

- (int) seekInputStreamWithOffset: (int64_t)offset 
                           origin: (NSInteger)origin;

@end

static void
handleEasyCode(int code)
{
  if (CURLE_OK != code)
    {
      NSString    *reason;
      NSException *e;

      reason = [NSString stringWithFormat: @"An error occurred, CURLcode is %d", 
        code];
      e = [NSException exceptionWithName: @"libcurl.easy" 
                                  reason: reason 
                                userInfo: nil];
      [e raise];
    }
}

static size_t
curl_write_function(char *data, size_t size, size_t nmemb, void *userdata) 
{
  if (!userdata)
    {
      return 0;
    }

  GSEasyHandle *handle = (GSEasyHandle*)userdata;
  
  [handle resetTimer]; //FIXME should be deffered after the function returns?

  return [handle didReceiveData:data size:size nmemb:nmemb];
}

static size_t
curl_read_function(char *data, size_t size, size_t nmemb, void *userdata) 
{
  if (!userdata)
    {
      return 0;
    }

  GSEasyHandle *handle = (GSEasyHandle*)userdata;
   
  [handle resetTimer]; //FIXME should be deffered after the function returns?

  return [handle fillWriteBuffer: data size: size nmemb: nmemb];
}

size_t
curl_header_function(char *data, size_t size, size_t nmemb, void *userdata) 
{
  if (!userdata)
    {
      return 0;
    }

  GSEasyHandle *handle = (GSEasyHandle*)userdata;
  double length;

  [handle resetTimer]; //FIXME should be deffered after the function returns?

  handleEasyCode(curl_easy_getinfo(handle.rawHandle,
    CURLINFO_CONTENT_LENGTH_DOWNLOAD, &length));
  
  return [handle didReceiveHeaderData: data 
                                 size: size 
                                nmemb: nmemb 
                        contentLength: length];
}

static int
curl_seek_function(void *userdata, curl_off_t offset, int origin) 
{
  if (!userdata)
    {
      return CURL_SEEKFUNC_FAIL;
    }

  GSEasyHandle *handle = (GSEasyHandle*)userdata;
  
  return [handle seekInputStreamWithOffset: offset origin: origin];
}

static int
curl_debug_function(CURL *handle, curl_infotype type, char *data,
  size_t size, void *userptr) 
{
  if (!userptr)
    {
      return 0;
    }

  if (CURLINFO_SSL_DATA_OUT == type || CURLINFO_SSL_DATA_IN == type)
    {
      return 0; // Don't log encrypted data here
    }

  NSURLSessionTask      *task = (NSURLSessionTask*)userptr;
  NSString              *text = @"";
  NSURLRequest          *o = [task originalRequest];
  NSURLRequest          *r = [task currentRequest];
  id<GSLogDelegate>     d = [(nil == r ? o : r) _debugLogDelegate];

  if (d != nil)
    {
      if (CURLINFO_DATA_IN == type || CURLINFO_HEADER_IN == type)
        {
          if ([d getBytes: (const uint8_t *)data ofLength: size byHandle: o])
            {
              return 0; // Handled
            }
        }
      if (CURLINFO_DATA_OUT == type || CURLINFO_HEADER_OUT == type)
        {
          if ([d putBytes: (const uint8_t *)data ofLength: size byHandle: o])
            {
              return 0; // Handled
            }
        }
    }
  if (data) 
    {
      text = [NSString stringWithUTF8String: data];
    }
  
  NSLog(@"%p %lu %d %@", o, [task taskIdentifier], type, text);

  return 0;
}

static int
curl_socket_function(void *userdata, curl_socket_t fd, curlsocktype type) 
{ 
  return 0; 
}

@implementation GSEasyHandle
{
  NSURLSessionConfiguration  *_config;
  GSEasyHandlePauseState     _pauseState;
  struct curl_slist          *_headerList;
}

- (instancetype) initWithDelegate: (id<GSEasyHandleDelegate>)delegate 
{
  if (nil != (self = [super init])) 
    {
      _rawHandle = curl_easy_init();
      _delegate = delegate;

      char *eb = (char *)malloc(sizeof(char) * (CURL_ERROR_SIZE + 1));
      _errorBuffer = memset(eb, 0, sizeof(char) * (CURL_ERROR_SIZE + 1));
      
      [self setupCallbacks];
    }

  return self;
}

- (void) dealloc 
{
  curl_easy_cleanup(_rawHandle);
  curl_slist_free_all(_headerList);
  free(_errorBuffer);
  DESTROY(_config);
  DESTROY(_timeoutTimer);
  DESTROY(_URL);
  [super dealloc];
}

- (CURL*) rawHandle
{
  return _rawHandle;
}

- (char*) errorBuffer
{
  return _errorBuffer;
}

- (GSTimeoutSource*) timeoutTimer
{
  return _timeoutTimer;
}

- (void) setTimeoutTimer: (GSTimeoutSource*)timer
{
  ASSIGN(_timeoutTimer, timer);
}

- (NSURL*) URL
{
  return _URL;
}

- (void) transferCompletedWithError: (NSError*)error 
{
  [_delegate transferCompletedWithError: error];
}

- (void) resetTimer 
{
  // simply create a new timer with the same queue, timeout and handler
  // this must cancel the old handler and reset the timer
  DESTROY(_timeoutTimer);
  _timeoutTimer = [[GSTimeoutSource alloc] initWithQueue: [_timeoutTimer queue]
                                            milliseconds: [_timeoutTimer milliseconds]
                                                 handler: [_timeoutTimer handler]];
}

- (void) setupCallbacks 
{
  // write
  handleEasyCode(curl_easy_setopt(_rawHandle, CURLOPT_WRITEDATA, self));
  handleEasyCode(curl_easy_setopt(_rawHandle, CURLOPT_WRITEFUNCTION,
    curl_write_function));

  // read
  handleEasyCode(curl_easy_setopt(_rawHandle, CURLOPT_READDATA, self));
  handleEasyCode(curl_easy_setopt(_rawHandle, CURLOPT_READFUNCTION,
    curl_read_function));

  // header
  handleEasyCode(curl_easy_setopt(_rawHandle, CURLOPT_HEADERDATA, self));
  handleEasyCode(curl_easy_setopt(_rawHandle, CURLOPT_HEADERFUNCTION,
    curl_header_function));

  // socket options
  handleEasyCode(curl_easy_setopt(_rawHandle, CURLOPT_SOCKOPTDATA, self));
  handleEasyCode(curl_easy_setopt(_rawHandle, CURLOPT_SOCKOPTFUNCTION,
    curl_socket_function));

  // seeking in input stream
  handleEasyCode(curl_easy_setopt(_rawHandle, CURLOPT_SEEKDATA, self));
  handleEasyCode(curl_easy_setopt(_rawHandle, CURLOPT_SEEKFUNCTION,
    curl_seek_function));
}

- (int) urlErrorCodeWithEasyCode: (int)easyCode 
{
    int failureErrno = (int)[self connectFailureErrno];

    if (easyCode == CURLE_OK) 
      {
        return 0;
      } 
    else if (failureErrno == ECONNREFUSED) 
      {
        return NSURLErrorCannotConnectToHost;
      } 
    else if (easyCode == CURLE_UNSUPPORTED_PROTOCOL) 
      {
        return NSURLErrorUnsupportedURL;
      } 
    else if (easyCode == CURLE_URL_MALFORMAT) 
      {
        return NSURLErrorBadURL;
      } 
    else if (easyCode == CURLE_COULDNT_RESOLVE_HOST) 
      {
        return NSURLErrorCannotFindHost;
      } 
    else if (easyCode == CURLE_RECV_ERROR && failureErrno == ECONNRESET) 
      {
        return NSURLErrorNetworkConnectionLost;
      } 
    else if (easyCode == CURLE_SEND_ERROR && failureErrno == ECONNRESET) 
      {
        return NSURLErrorNetworkConnectionLost;
      } 
    else if (easyCode == CURLE_GOT_NOTHING) 
      {
        return NSURLErrorBadServerResponse;
      }
    else if (easyCode == CURLE_ABORTED_BY_CALLBACK) 
      {
        return NSURLErrorUnknown;
      }
    else if (easyCode == CURLE_COULDNT_CONNECT && failureErrno == ETIMEDOUT) 
      {
        return NSURLErrorTimedOut;
      }
    else if (easyCode == CURLE_OPERATION_TIMEDOUT) 
      {
        return NSURLErrorTimedOut;
      } 
    else 
      {
        return NSURLErrorUnknown;
      }
}

- (void) setVerboseMode: (BOOL)flag 
{
  handleEasyCode(curl_easy_setopt(_rawHandle, CURLOPT_VERBOSE, flag ? 1 : 0));
}

- (void) setDebugOutput: (BOOL)flag 
                   task: (NSURLSessionTask*)task 
{
  if (flag) 
    {
      handleEasyCode(curl_easy_setopt(_rawHandle, CURLOPT_DEBUGDATA, self));
      handleEasyCode(curl_easy_setopt(_rawHandle, CURLOPT_DEBUGFUNCTION,
	curl_debug_function));
    } 
  else 
    {
      handleEasyCode(curl_easy_setopt(_rawHandle, CURLOPT_DEBUGDATA, NULL));
      handleEasyCode(curl_easy_setopt(_rawHandle, CURLOPT_DEBUGFUNCTION, NULL));
    }
}

- (void) setPassHeadersToDataStream: (BOOL)flag 
{
  handleEasyCode(curl_easy_setopt(_rawHandle, CURLOPT_HEADER,
    flag ? 1 : 0));
}

- (void) setFollowLocation: (BOOL)flag 
{
  handleEasyCode(curl_easy_setopt(_rawHandle, CURLOPT_FOLLOWLOCATION,
    flag ? 1 : 0));
}

- (void) setProgressMeterOff: (BOOL)flag 
{
  handleEasyCode(curl_easy_setopt(_rawHandle, CURLOPT_NOPROGRESS,
    flag ? 1 : 0));
}

- (void) setSkipAllSignalHandling: (BOOL)flag 
{
  handleEasyCode(curl_easy_setopt(_rawHandle, CURLOPT_NOSIGNAL,
    flag ? 1 : 0));
}

- (void) setErrorBuffer: (char*)buffer 
{
  char *b = buffer ? buffer : _errorBuffer;
  handleEasyCode(curl_easy_setopt(_rawHandle, CURLOPT_ERRORBUFFER, b));
}

- (void) setFailOnHTTPErrorCode: (BOOL)flag 
{
  handleEasyCode(curl_easy_setopt(_rawHandle, CURLOPT_FAILONERROR,
    flag ? 1 : 0));
}

- (void) setURL: (NSURL *)URL 
{
  ASSIGN(_URL, URL);
  if (nil != [URL absoluteString]) 
    {
      handleEasyCode(curl_easy_setopt(_rawHandle, CURLOPT_URL,
	[[URL absoluteString] UTF8String]));
    }
}

- (void) setPipeWait: (BOOL)flag
{
  handleEasyCode(curl_easy_setopt(_rawHandle, CURLOPT_PIPEWAIT, flag ? 1 : 0));
}

- (void) setConnectToHost: (NSString*)host port: (NSInteger)port 
{
  if (nil != host) 
    {
      NSString *originHost = [_URL host];
      NSString *value = nil;

      if (0 == port)
        {
          value = [NSString stringWithFormat: @"%@::%@", originHost, host];
        } 
      else 
        {
          value = [NSString stringWithFormat: @"%@:%lu:%@", 
            originHost, port, host];
        }
      
      struct curl_slist *connect_to = NULL;
      connect_to = curl_slist_append(NULL, [value UTF8String]);
      handleEasyCode(
	curl_easy_setopt(_rawHandle, CURLOPT_CONNECT_TO, connect_to));
    }
}

- (void) setSessionConfig: (NSURLSessionConfiguration*)config 
{
  ASSIGN(_config, config);
#if	defined(CURLOPT_MAXAGE_CONN)
  /* This specifies the maximum age of a connection if it is to be considered
   * a candidate for re-use.  By default curl currently uses 118 seconds, so
   * this is what we will get if the configuration does not contain a positive
   * number of seconds.
   */
  if ([config HTTPMaximumConnectionLifetime] > 0)
    {
      handleEasyCode(curl_easy_setopt(_rawHandle, CURLOPT_MAXAGE_CONN,
	(long)[config HTTPMaximumConnectionLifetime]));
    }
#endif
}

- (void) setAllowedProtocolsToHTTPAndHTTPS 
{
  handleEasyCode(curl_easy_setopt(_rawHandle, CURLOPT_PROTOCOLS,
    CURLPROTO_HTTP | CURLPROTO_HTTPS));
  handleEasyCode(curl_easy_setopt(_rawHandle, CURLOPT_REDIR_PROTOCOLS,
    CURLPROTO_HTTP | CURLPROTO_HTTPS));
}

- (void) setPreferredReceiveBufferSize: (NSInteger)size 
{
  handleEasyCode(curl_easy_setopt(_rawHandle, CURLOPT_BUFFERSIZE,
    MIN(size, CURL_MAX_WRITE_SIZE)));
}

- (void) setCustomHeaders: (NSArray*)headers 
{
  NSEnumerator  *e;
  NSString      *h;

  e = [headers objectEnumerator];
  while (nil != (h = [e nextObject]))
    {
      _headerList = curl_slist_append(_headerList, [h UTF8String]);
    }
  handleEasyCode(curl_easy_setopt(_rawHandle, CURLOPT_HTTPHEADER, _headerList));
}

- (void) setAutomaticBodyDecompression: (BOOL)flag 
{
  if (flag) 
    {
      handleEasyCode(curl_easy_setopt(_rawHandle,
	CURLOPT_ACCEPT_ENCODING, ""));
      handleEasyCode(curl_easy_setopt(_rawHandle,
	CURLOPT_HTTP_CONTENT_DECODING, 1));
    } 
  else 
    {
      handleEasyCode(curl_easy_setopt(_rawHandle,
	CURLOPT_ACCEPT_ENCODING, NULL));
      handleEasyCode(curl_easy_setopt(_rawHandle,
	CURLOPT_HTTP_CONTENT_DECODING, 0));
    }
}

- (void) setRequestMethod: (NSString*)method 
{
  if (nil == method) 
    {
      return;
    }

  handleEasyCode(curl_easy_setopt(_rawHandle, CURLOPT_CUSTOMREQUEST,
    [method UTF8String]));
}

- (void) setNoBody: (BOOL)flag 
{
  handleEasyCode(curl_easy_setopt(_rawHandle, CURLOPT_NOBODY,
    flag ? 1 : 0));
}

- (void) setUpload: (BOOL)flag 
{
  handleEasyCode(curl_easy_setopt(_rawHandle, CURLOPT_UPLOAD, flag ? 1 : 0));
}

- (void) setRequestBodyLength: (int64_t)length 
{
  handleEasyCode(curl_easy_setopt(_rawHandle, CURLOPT_INFILESIZE_LARGE,
    length));
}

- (void) setTimeout: (NSInteger)timeout 
{
  handleEasyCode(curl_easy_setopt(_rawHandle, CURLOPT_TIMEOUT,
    (long)timeout));
}

- (void) setProxy 
{    
  //TODO setup proxy
}

- (void) updatePauseState: (GSEasyHandlePauseState)pauseState 
{
  NSUInteger	send = pauseState & GSEasyHandlePauseStateSend;
  NSUInteger	receive = pauseState & GSEasyHandlePauseStateReceive;
  int		bitmask;

  bitmask = 0
    | (send ? CURLPAUSE_SEND : CURLPAUSE_SEND_CONT)
    | (receive ? CURLPAUSE_RECV : CURLPAUSE_RECV_CONT);
  handleEasyCode(curl_easy_pause(_rawHandle, bitmask));
}

- (double) getTimeoutIntervalSpent 
{
  double timeSpent;
  curl_easy_getinfo(_rawHandle, CURLINFO_TOTAL_TIME, &timeSpent);
  return timeSpent / 1000;
}

- (long) connectFailureErrno 
{
  long _errno;
  handleEasyCode(curl_easy_getinfo(_rawHandle, CURLINFO_OS_ERRNO, &_errno));
  return _errno;
}

- (void) pauseSend 
{
  if (_pauseState & GSEasyHandlePauseStateSend) 
    {
      return;
    }
    
  _pauseState = _pauseState | GSEasyHandlePauseStateSend;
  [self updatePauseState: _pauseState];
}

- (void) unpauseSend
{
  if (!(_pauseState & GSEasyHandlePauseStateSend))
    {
      return;
    }
  
  _pauseState = _pauseState ^ GSEasyHandlePauseStateSend;
  [self updatePauseState: _pauseState];
}

- (void) pauseReceive
{
  if (_pauseState & GSEasyHandlePauseStateReceive) 
    {
      return;
    }
  
  _pauseState = _pauseState | GSEasyHandlePauseStateReceive;
  [self updatePauseState: _pauseState];
}

- (void) unpauseReceive 
{
  if (!(_pauseState & GSEasyHandlePauseStateReceive))
    {
      return;
    }
  
  _pauseState = _pauseState ^ GSEasyHandlePauseStateReceive;
  [self updatePauseState: _pauseState];
}

- (NSInteger) didReceiveData: (char*)data 
                        size: (NSInteger)size 
                       nmemb: (NSInteger)nmemb 
{
  NSData              	*buffer;
  GSEasyHandleAction  	action;
  NSUInteger		bytes;

  if (![_delegate respondsToSelector: @selector(didReceiveData:)])
    {
      return 0;
    }

  bytes = size * nmemb;
  buffer = AUTORELEASE([[NSData alloc] initWithBytes: data length: bytes]);
  action = [_delegate didReceiveData: buffer];
  switch (action) 
    {
      case GSEasyHandleActionProceed:
        return bytes;
      case GSEasyHandleActionAbort:
        return 0;
      case GSEasyHandleActionPause:
        _pauseState = _pauseState | GSEasyHandlePauseStateReceive;
        return CURL_WRITEFUNC_PAUSE;
    }
}

- (NSInteger) didReceiveHeaderData: (char*)headerData
                              size: (NSInteger)size
                             nmemb: (NSInteger)nmemb
                     contentLength: (double)contentLength 
{
  NSData              	*buffer;
  GSEasyHandleAction  	action;
  NSInteger		bytes = size * nmemb;

  buffer = [NSData dataWithBytes: headerData length: bytes];

  [self setCookiesWithHeaderData: buffer];

  if (![_delegate respondsToSelector:
    @selector(didReceiveHeaderData:contentLength:)]) 
    {
      return 0;
    }

  action = [_delegate didReceiveHeaderData: buffer 
                             contentLength: (int64_t)contentLength];
  switch (action) 
    {
      case GSEasyHandleActionProceed:
        return bytes;
      case GSEasyHandleActionAbort:
        return 0;
      case GSEasyHandleActionPause:
        _pauseState = _pauseState | GSEasyHandlePauseStateReceive;
        return CURL_WRITEFUNC_PAUSE;
    }
}

- (NSInteger) fillWriteBuffer: (char*)buffer 
                         size: (NSInteger)size 
                        nmemb: (NSInteger)nmemb 
{
  __block NSInteger d;
  
  if (![_delegate respondsToSelector: @selector(fillWriteBufferLength:result:)]) 
    {
      return CURL_READFUNC_ABORT;
    }

  [_delegate fillWriteBufferLength: size * nmemb
    result: ^(GSEasyHandleWriteBufferResult result, NSInteger length, NSData *data) 
      {
        switch (result) 
          {
            case GSEasyHandleWriteBufferResultPause:
              _pauseState = _pauseState | GSEasyHandlePauseStateSend;
              d = CURL_READFUNC_PAUSE;
              break;
            case GSEasyHandleWriteBufferResultAbort:
              d = CURL_READFUNC_ABORT;
              break;
            case GSEasyHandleWriteBufferResultBytes:
              memcpy(buffer, [data bytes], length);
              d = length;
              break;
          }
      }];

  return d;
}

- (int) seekInputStreamWithOffset: (int64_t)offset 
                           origin: (NSInteger)origin 
{
  NSAssert(SEEK_SET == origin, @"Unexpected 'origin' in seek.");
  
  if (![_delegate respondsToSelector: @selector(seekInputStreamToPosition:)]) 
    {
      return CURL_SEEKFUNC_CANTSEEK;
    }

  if ([_delegate seekInputStreamToPosition: offset]) 
    {
      return CURL_SEEKFUNC_OK;
    } 
  else 
    {
      return CURL_SEEKFUNC_CANTSEEK;
    }
}

- (void) setCookiesWithHeaderData: (NSData*)data 
{
  NSString       *headerLine; 
  NSRange        r;
  NSString       *head;
  NSString       *tail;
  NSCharacterSet *set;
  NSString       *key;
  NSString       *value;
  NSArray        *cookies;
  NSDictionary   *d;

  if (nil != _config 
    && NSHTTPCookieAcceptPolicyNever != [_config HTTPCookieAcceptPolicy] 
    && nil != [_config HTTPCookieStorage]) 
    {
      headerLine = [[NSString alloc] initWithData: data 
                                         encoding: NSUTF8StringEncoding];
      if (0 == [headerLine length]) 
        {
          RELEASE(headerLine);
          return;
        }

      r = [headerLine rangeOfString: @":"];
      if (NSNotFound != r.location) 
        {
          head = [headerLine substringToIndex:r.location];
          tail = [headerLine substringFromIndex:r.location + 1];
          set = [NSCharacterSet whitespaceAndNewlineCharacterSet];
          key = [head stringByTrimmingCharactersInSet:set];
          value = [tail stringByTrimmingCharactersInSet:set];

          if (nil != key && nil != value) 
            {
              d = [NSDictionary dictionaryWithObject: value forKey: key];
              cookies = [NSHTTPCookie cookiesWithResponseHeaderFields: d 
                                                                forURL: _URL];
              if ([cookies count] > 0)
                {
                  [[_config HTTPCookieStorage] setCookies: cookies 
                                                   forURL: _URL 
                                          mainDocumentURL: nil];
                }
            }
        }
      RELEASE(headerLine);
    }
}

@end
