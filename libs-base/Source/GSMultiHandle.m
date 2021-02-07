#import "GSMultiHandle.h"
#import "GSTimeoutSource.h"
#import "GSEasyHandle.h"

#import "Foundation/NSArray.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSError.h"
#import "Foundation/NSException.h"
#import "Foundation/NSURLError.h"
#import "Foundation/NSURLSession.h"

@interface GSMultiHandle ()
- (void) performActionForSocket: (int)socket;
- (void) readAndWriteAvailableDataOnSocket: (int)socket;
- (void) readMessages;
- (void) completedTransferForEasyHandle: (CURL*)rawEasyHandle 
                               easyCode: (int)easyCode;
- (int32_t) registerWithSocket: (curl_socket_t)socket 
                          what: (int)what 
               socketSourcePtr: (void *)socketSourcePtr;
@end

static void handleEasyCode(int code)
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

static void handleMultiCode(int code)
{
  if (CURLM_OK != code)
    {
      NSString    *reason;
      NSException *e;

      reason = [NSString stringWithFormat: @"An error occurred, CURLcode is %d", 
        code];
      e = [NSException exceptionWithName: @"libcurl.multi" 
                                  reason: reason 
                                userInfo: nil];
      [e raise];
    }
}

static int curl_socket_function(CURL *easyHandle, curl_socket_t socket, int what, void *userdata, void *socketptr) 
{
  GSMultiHandle  *handle = (GSMultiHandle*)userdata;
  
  return [handle registerWithSocket: socket 
                               what: what 
                    socketSourcePtr: socketptr];
}

static int curl_timer_function(CURL *easyHandle, int timeout, void *userdata) {
    GSMultiHandle  *handle = (GSMultiHandle*)userdata;
   
    [handle updateTimeoutTimerToValue: timeout];

    return 0;
}

@implementation GSMultiHandle
{
  NSMutableArray    *_easyHandles;
  dispatch_queue_t  _queue;
  GSTimeoutSource   *_timeoutSource;
}

- (CURLM*) rawHandle
{
  return _rawHandle;
}

- (instancetype) initWithConfiguration: (NSURLSessionConfiguration*)conf 
                             workQueue: (dispatch_queue_t)aQueue
{
  if (nil != (self = [super init]))
    {
      _rawHandle = curl_multi_init();
      _easyHandles = [[NSMutableArray alloc] init];
#if HAVE_DISPATCH_QUEUE_CREATE_WITH_TARGET
      _queue = dispatch_queue_create_with_target("GSMultiHandle.isolation",
	DISPATCH_QUEUE_SERIAL, aQueue);
#else
      _queue = dispatch_queue_create("GSMultiHandle.isolation",
	DISPATCH_QUEUE_SERIAL);
      dispatch_set_target_queue(_queue, aQueue);
#endif
      [self setupCallbacks];
      [self configureWithConfiguration: conf];
    }

  return self;
}

- (void) dealloc
{
  NSEnumerator   *e;
  GSEasyHandle   *handle;

  DESTROY(_timeoutSource);

  e = [_easyHandles objectEnumerator];
  while (nil != (handle = [e nextObject]))
    {
      curl_multi_remove_handle([handle rawHandle], _rawHandle);
    }
  DESTROY(_easyHandles);

  curl_multi_cleanup(_rawHandle);

  [super dealloc];
}

- (void) configureWithConfiguration: (NSURLSessionConfiguration*)configuration 
{
  handleEasyCode(curl_multi_setopt(_rawHandle, CURLMOPT_MAX_HOST_CONNECTIONS, [configuration HTTPMaximumConnectionsPerHost])); 
  handleEasyCode(curl_multi_setopt(_rawHandle, CURLMOPT_PIPELINING, [configuration HTTPShouldUsePipelining] ? CURLPIPE_MULTIPLEX : CURLPIPE_NOTHING)); 
}

- (void)setupCallbacks 
{
  handleEasyCode(curl_multi_setopt(_rawHandle, CURLMOPT_SOCKETDATA, (void*)self));
  handleEasyCode(curl_multi_setopt(_rawHandle, CURLMOPT_SOCKETFUNCTION, curl_socket_function));

  handleEasyCode(curl_multi_setopt(_rawHandle, CURLMOPT_TIMERDATA, (__bridge void *)self));
  handleEasyCode(curl_multi_setopt(_rawHandle, CURLMOPT_TIMERFUNCTION, curl_timer_function));
}

- (void) addHandle: (GSEasyHandle*)easyHandle
{
  // If this is the first handle being added, we need to `kick` the
  // underlying multi handle by calling `timeoutTimerFired` as
  // described in
  // <https://curl.haxx.se/libcurl/c/curl_multi_socket_action.html>.
  // That will initiate the registration for timeout timer and socket
  // readiness.
  BOOL needsTimeout = false;
  
  if ([_easyHandles count] == 0) 
    {
      needsTimeout = YES;
    }

  [_easyHandles addObject: easyHandle];

  handleMultiCode(curl_multi_add_handle(_rawHandle, [easyHandle rawHandle]));

  if (needsTimeout)
    {
      [self timeoutTimerFired];
    }
}

- (void) removeHandle: (GSEasyHandle*)easyHandle
{
  NSEnumerator  *e;
  int           idx = 0;
  BOOL          found = NO;
  GSEasyHandle  *h;

  e = [_easyHandles objectEnumerator];
  while (nil != (h = [e nextObject]))
    {
      if ([h rawHandle] == [easyHandle rawHandle])
        {
          found = YES;
          break;
        }
      idx++;
    }

  NSAssert(found, @"Handle not in list.");

  handleMultiCode(curl_multi_remove_handle(_rawHandle, [easyHandle rawHandle]));
  [_easyHandles removeObjectAtIndex: idx];
}

- (void) updateTimeoutTimerToValue: (NSInteger)value
{
  // A timeout_ms value of -1 passed to this callback means you should delete 
  // the timer. All other values are valid expire times in number 
  // of milliseconds.
  if (-1 == value)
    {
      DESTROY(_timeoutSource);
    }   
  else if (0 == value) 
    {
      DESTROY(_timeoutSource);
      dispatch_async(_queue, 
        ^{
          [self timeoutTimerFired];
        });
    } 
  else 
    {
      if (nil == _timeoutSource || value != [_timeoutSource milliseconds]) 
        {
          DESTROY(_timeoutSource);
          _timeoutSource = [[GSTimeoutSource alloc] initWithQueue: _queue
                                                     milliseconds: value
                                                          handler: ^{
                                                            [self timeoutTimerFired];
                                                          }];
      }
  }
}

- (void) performActionForSocket: (int)socket 
{
  [self readAndWriteAvailableDataOnSocket: socket];
}

- (void)timeoutTimerFired 
{
  [self readAndWriteAvailableDataOnSocket: CURL_SOCKET_TIMEOUT];
}

- (void) readAndWriteAvailableDataOnSocket: (int)socket 
{
  int runningHandlesCount = 0;
  
  handleMultiCode(curl_multi_socket_action(_rawHandle, socket, 0, &runningHandlesCount));
  
  [self readMessages];
}

/// Check the status of all individual transfers.
///
/// libcurl refers to this as “read multi stack informationals”.
/// Check for transfers that completed.
- (void) readMessages 
{
  while (true) 
    {
      int      count = 0;
      CURLMsg  *msg;
      CURL     *easyHandle;
      int      code;

      msg = curl_multi_info_read(_rawHandle, &count);

      if (NULL == msg || CURLMSG_DONE != msg->msg || !msg->easy_handle) break;
      
      easyHandle = msg->easy_handle;
      code = msg->data.result;
      [self completedTransferForEasyHandle: easyHandle easyCode: code];
    }
}

- (void) completedTransferForEasyHandle: (CURL*)rawEasyHandle 
                               easyCode: (int)easyCode 
{
  NSEnumerator  *e;
  GSEasyHandle  *h;
  GSEasyHandle  *handle = nil;
  NSError       *err = nil;
  int           errCode;

  e = [_easyHandles objectEnumerator];
  while (nil != (h = [e nextObject]))
    {
      if ([h rawHandle] == rawEasyHandle)
        {
          handle = h;
          break;
        }
    }

  NSAssert(nil != handle, @"Transfer completed for easy handle"
    @", but it is not in the list of added handles.");

  errCode = [handle urlErrorCodeWithEasyCode: easyCode];
  if (0 != errCode) 
    {
      NSString *d = nil;

      if ([handle errorBuffer][0] == 0) 
        {
          const char *description = curl_easy_strerror(errCode);
          d = [[NSString alloc] initWithCString: description 
                                       encoding: NSUTF8StringEncoding];
        } 
      else 
        {
          d = [[NSString alloc] initWithCString: [handle errorBuffer] 
                                       encoding: NSUTF8StringEncoding];
        }
      err = [NSError errorWithDomain: NSURLErrorDomain 
                                code: errCode 
                            userInfo: @{NSLocalizedDescriptionKey : d}];
      RELEASE(d);
    }

  [handle transferCompletedWithError: err];
}

- (int32_t) registerWithSocket: (curl_socket_t)socket 
                          what: (int)what 
               socketSourcePtr: (void *)socketSourcePtr
{
  // We get this callback whenever we need to register or unregister a
  // given socket with libdispatch.
  // The `action` / `what` defines if we should register or unregister
  // that we're interested in read and/or write readiness. We will do so
  // through libdispatch (DispatchSource) and store the source(s) inside
  // a `SocketSources` which we in turn store inside libcurl's multi handle
  // by means of curl_multi_assign() -- we retain the object first.

  GSSocketRegisterAction  *action;
  GSSocketSources         *socketSources;

  action = [[GSSocketRegisterAction alloc] initWithRawValue: what];
  socketSources = [GSSocketSources from: socketSourcePtr];

  if (nil == socketSources && [action needsSource]) 
    {
      GSSocketSources *s;

      s = [[GSSocketSources alloc] init];
      curl_multi_assign(_rawHandle, socket, (void*)s);
      socketSources = s;
    } 
  else if (nil != socketSources
    && GSSocketRegisterActionTypeUnregister == [action type]) 
    {
      DESTROY(socketSources);
    }

  if (nil != socketSources) 
    {
      [socketSources createSourcesWithAction: action
                                      socket: socket
                                       queue: _queue
                                     handler: ^{
	        [self performActionForSocket: socket];
        }];
    }

  RELEASE(action);

  return 0;
}

@end

@implementation GSSocketRegisterAction

- (instancetype) initWithRawValue: (int)rawValue 
{
  if (nil != (self = [super init]))
    {
      switch (rawValue) {
          case CURL_POLL_NONE:
            _type = GSSocketRegisterActionTypeNone;
            break;
          case CURL_POLL_IN:
            _type = GSSocketRegisterActionTypeRegisterRead;
            break;
          case CURL_POLL_OUT:
            _type = GSSocketRegisterActionTypeRegisterWrite;
            break;
          case CURL_POLL_INOUT:
            _type = GSSocketRegisterActionTypeRegisterReadAndWrite;
            break;
          case CURL_POLL_REMOVE:
            _type = GSSocketRegisterActionTypeUnregister;
            break;
          default:
            NSAssert(NO, @"Invalid CURL_POLL value"); 
      }
    }

  return self;
}

- (GSSocketRegisterActionType) type
{
  return _type;
} 

- (BOOL) needsReadSource 
{
  switch (self.type) 
    {
      case GSSocketRegisterActionTypeNone:
        return false;
      case GSSocketRegisterActionTypeRegisterRead:
        return true;
      case GSSocketRegisterActionTypeRegisterWrite:
        return false;
      case GSSocketRegisterActionTypeRegisterReadAndWrite:
        return true;
      case GSSocketRegisterActionTypeUnregister:
        return false;
    }
}

- (BOOL) needsWriteSource 
{
  switch (self.type) 
    {
      case GSSocketRegisterActionTypeNone:
        return false;
      case GSSocketRegisterActionTypeRegisterRead:
        return false;
      case GSSocketRegisterActionTypeRegisterWrite:
        return true;
      case GSSocketRegisterActionTypeRegisterReadAndWrite:
        return true;
      case GSSocketRegisterActionTypeUnregister:
        return false;
    }
}

- (BOOL)needsSource 
{
  return [self needsReadSource] || [self needsWriteSource];
}

@end

@implementation GSSocketSources

- (void) dealloc
{
  if (_readSource) 
    {
      dispatch_source_cancel(_readSource);
    }
  _readSource = NULL;

  if (_writeSource) 
    {
      dispatch_source_cancel(_writeSource);
    }
  _writeSource = NULL;
  [super dealloc];
}

- (void) createSourcesWithAction: (GSSocketRegisterAction*)action
                          socket: (curl_socket_t)socket
                           queue: (dispatch_queue_t)queue
                         handler: (dispatch_block_t)handler 
{
  if ([action needsReadSource]) 
    {
      [self createReadSourceWithSocket: socket queue: queue handler: handler];
    }

  if ([action needsWriteSource]) 
    {
      [self createWriteSourceWithSocket: socket queue: queue handler: handler];
    }
}

- (void) createReadSourceWithSocket: (curl_socket_t)socket
                              queue: (dispatch_queue_t)queue
                            handler: (dispatch_block_t)handler 
{
  dispatch_source_t s;

  if (_readSource) 
    {
      return;
    }

  s = dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, socket, 0, queue);
  dispatch_source_set_event_handler(s, handler);
  _readSource = s;
  dispatch_resume(s);
}

- (void) createWriteSourceWithSocket: (curl_socket_t)socket
                               queue: (dispatch_queue_t)queue
                             handler: (dispatch_block_t)handler 
{
  dispatch_source_t s;

  if (_writeSource) 
    {
      return;
    }

  s = dispatch_source_create(DISPATCH_SOURCE_TYPE_WRITE, socket, 0, queue);
  dispatch_source_set_event_handler(s, handler);
  _writeSource = s;
  dispatch_resume(s);
}

+ (instancetype) from: (void*)socketSourcePtr 
{
  if (!socketSourcePtr)
    {
      return nil;
    }
  else
    {
      return (GSSocketSources*)socketSourcePtr;
    }
}

@end
