#import "GSNativeProtocol.h"
#import "GSTransferState.h"
#import "GSURLSessionTaskBody.h"
#import "Foundation/NSData.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSError.h"
#import "Foundation/NSException.h"
#import "Foundation/NSOperation.h"
#import "Foundation/NSURL.h"
#import "Foundation/NSURLError.h"
#import "Foundation/NSURLSession.h"


static BOOL isEasyHandlePaused(GSNativeProtocolInternalState state) 
{
  switch (state)
    {
      case GSNativeProtocolInternalStateInitial:
        return NO;
      case GSNativeProtocolInternalStateFulfillingFromCache:
        return NO;
      case GSNativeProtocolInternalStateTransferReady:
        return NO;
      case GSNativeProtocolInternalStateTransferInProgress:
        return NO;
      case GSNativeProtocolInternalStateTransferCompleted:
        return NO;
      case GSNativeProtocolInternalStateTransferFailed:
        return NO;
      case GSNativeProtocolInternalStateWaitingForRedirectCompletionHandler:
        return NO;
      case GSNativeProtocolInternalStateWaitingForResponseCompletionHandler:
        return YES;
      case GSNativeProtocolInternalStateTaskCompleted:
        return NO;
    }
}

static BOOL isEasyHandleAddedToMultiHandle(GSNativeProtocolInternalState state)
{
  switch (state)
    {
      case GSNativeProtocolInternalStateInitial:
        return NO;
      case GSNativeProtocolInternalStateFulfillingFromCache:
        return NO;
      case GSNativeProtocolInternalStateTransferReady:
        return NO;
      case GSNativeProtocolInternalStateTransferInProgress:
        return YES;
      case GSNativeProtocolInternalStateTransferCompleted:
        return NO;
      case GSNativeProtocolInternalStateTransferFailed:
        return NO;
      case GSNativeProtocolInternalStateWaitingForRedirectCompletionHandler:
        return NO;
      case GSNativeProtocolInternalStateWaitingForResponseCompletionHandler:
        return YES;
      case GSNativeProtocolInternalStateTaskCompleted:
        return NO;
    }
}

@interface NSURLSession (Internal)

- (void) removeHandle: (GSEasyHandle*)handle;

- (void) addHandle: (GSEasyHandle*)handle;

@end

@implementation NSURLSession (Internal)

- (void) removeHandle: (GSEasyHandle*)handle
{
  [_multiHandle removeHandle: handle];
}

- (void) addHandle: (GSEasyHandle*)handle
{
  [_multiHandle addHandle: handle];
}

@end

@interface NSURLSessionTask (Internal)

- (void) setCurrentRequest: (NSURLRequest*)request;

- (dispatch_queue_t) workQueue;

- (NSUInteger) suspendCount;

- (void) getBodyWithCompletion: (void (^)(GSURLSessionTaskBody *body))completion;

- (void) setKnownBody: (GSURLSessionTaskBody*)body;

- (void) setError: (NSError*)error;

@end

@implementation NSURLSessionTask (Internal)

- (void) setCurrentRequest: (NSURLRequest*)request
{
  ASSIGN(_currentRequest, request);
}

- (dispatch_queue_t) workQueue
{
  return _workQueue;
}

- (NSUInteger) suspendCount
{
  return _suspendCount;
}

- (void) getBodyWithCompletion: (void (^)(GSURLSessionTaskBody *body))completion
{
  if (nil != _knownBody) 
    {
      completion(_knownBody);
      return;
    };

  GSURLSessionTaskBody *body = AUTORELEASE([[GSURLSessionTaskBody alloc] init]);
  completion(body);
}

- (void) setKnownBody: (GSURLSessionTaskBody*)body
{
  ASSIGN(_knownBody, body);
}

- (void) setError: (NSError*)error
{
  ASSIGN(_error, error);
}

@end

@implementation GSCompletionAction

- (void) dealloc
{
  DESTROY(_redirectRequest);
  [super dealloc];
}

- (GSCompletionActionType) type
{
  return _type;
}

- (void) setType: (GSCompletionActionType) type
{
  _type = type;
}

- (int) errorCode
{
  return _errorCode;
}

- (void) setErrorCode: (int)code
{
  _errorCode = code;
}

- (NSURLRequest*) redirectRequest
{
  return _redirectRequest;
}

- (void) setRedirectRequest: (NSURLRequest*)request
{
  ASSIGN(_redirectRequest, request);
}

@end

@implementation GSNativeProtocol

- (instancetype) initWithTask: (NSURLSessionTask*)_task 
               cachedResponse: (NSCachedURLResponse*)_cachedResponse 
                       client: (id<NSURLProtocolClient>)_client
{
  if (nil != (self = [super initWithTask: _task
                          cachedResponse: _cachedResponse 
                                  client: _client]))
    {
      _internalState = GSNativeProtocolInternalStateInitial;
      _easyHandle = [[GSEasyHandle alloc] initWithDelegate: self];
    }
    
  return self;
}

- (void) dealloc
{
  DESTROY(_easyHandle);
  DESTROY(_transferState);
  [super dealloc];
}

+ (NSURLRequest*) canonicalRequestForRequest: (NSURLRequest*)request 
{
  return request;
}

- (void) startLoading 
{
  [self resume];
}

- (void) stopLoading 
{
  NSURLSessionTask  *task;

  if (nil != (task = [self task])
    &&  NSURLSessionTaskStateSuspended == [task state])
    {
      [self suspend];
    }
  else
    {
      [self setInternalState: GSNativeProtocolInternalStateTransferFailed];
      NSAssert(nil != [task error], @"Missing error for failed task");
      [self completeTaskWithError: [task error]];
    }
}

- (void) setInternalState: (GSNativeProtocolInternalState)newState
{
  NSURLSessionTask               *task;
  GSNativeProtocolInternalState  oldState;

  if (!isEasyHandlePaused(_internalState) && isEasyHandlePaused(newState))
    {
      NSAssert(NO, @"Need to solve pausing receive.");
    }
  
  if (isEasyHandleAddedToMultiHandle(_internalState) 
    && !isEasyHandleAddedToMultiHandle(newState))
    {
      if (nil != (task = [self task]))
        {
          [[task session] removeHandle: _easyHandle];
        }
    }
  
  oldState = _internalState;
  _internalState = newState;

  if (!isEasyHandleAddedToMultiHandle(oldState) 
    && isEasyHandleAddedToMultiHandle(_internalState))
    {
      if (nil != (task = [self task]))
        {
          [[task session] addHandle: _easyHandle];
        }
    }
  if (isEasyHandlePaused(oldState) && !isEasyHandlePaused(_internalState))
    {
      NSAssert(NO, @"Need to solve pausing receive.");
    }
}

- (void) startNewTransferWithRequest: (NSURLRequest*)request 
{
  NSURLSessionTask  *task = [self task];

  [task setCurrentRequest: request];

  NSAssert(nil != [request URL], @"No URL in request.");

  [task getBodyWithCompletion: ^(GSURLSessionTaskBody *body) 
    {
      [task setKnownBody: body];
      [self setInternalState: GSNativeProtocolInternalStateTransferReady];
      ASSIGN(_transferState,
	[self createTransferStateWithURL: [request URL] 
				    body: body 
			       workQueue: [task workQueue]]);
      [self configureEasyHandleForRequest: request body: body];
      if ([task suspendCount] < 1) 
        {
          [self resume];
        }
    }];
}

- (void) configureEasyHandleForRequest: (NSURLRequest*)request 
                                  body: (GSURLSessionTaskBody*)body
{
  NSAssert(NO, @"Requires concrete implementation");
}

- (GSTransferState*) createTransferStateWithURL: (NSURL*)url
                                           body: (GSURLSessionTaskBody*)body
                                      workQueue: (dispatch_queue_t)workQueue 
{
    GSDataDrain *drain = [self createTransferBodyDataDrain];

    switch ([body type]) 
      {
        case GSURLSessionTaskBodyTypeNone:
          return AUTORELEASE([[GSTransferState alloc] initWithURL: url 
                                                    bodyDataDrain: drain]);
        case GSURLSessionTaskBodyTypeData: 
          {
            GSBodyDataSource *source;
            
            source = AUTORELEASE([[GSBodyDataSource alloc] initWithData: [body data]]);
            return AUTORELEASE([[GSTransferState alloc] initWithURL: url 
                                                      bodyDataDrain: drain 
                                                         bodySource: source]);
          }
        case GSURLSessionTaskBodyTypeFile: 
          {
            GSBodyFileSource *source;
            
            source = AUTORELEASE([[GSBodyFileSource alloc] initWithFileURL: [body fileURL]
                                                                 workQueue: workQueue
                                                      dataAvailableHandler: ^{
                                                        [_easyHandle unpauseSend];
                                                      }]);
            return AUTORELEASE([[GSTransferState alloc] initWithURL: url 
                                                      bodyDataDrain: drain 
                                                         bodySource: source]);
          }
        case GSURLSessionTaskBodyTypeStream: 
          {
            GSBodyStreamSource *source;
            
            source = AUTORELEASE([[GSBodyStreamSource alloc] initWithInputStream: [body inputStream]]);
            return AUTORELEASE([[GSTransferState alloc] initWithURL: url 
                                                      bodyDataDrain: drain 
                                                         bodySource: source]);
          }
      }
}

// The data drain.
// This depends on what the delegate need.
- (GSDataDrain*) createTransferBodyDataDrain 
{
  NSURLSession *s = [[self task] session];
  GSDataDrain  *dd = AUTORELEASE([[GSDataDrain alloc] init]);
  
  if (nil != [s delegate])
    {
      // Data will be forwarded to the delegate as we receive it, we don't
      // need to do anything about it.
      [dd setType: GSDataDrainTypeIgnore];
      return dd;
    }
  else
    {
      [dd setType: GSDataDrainTypeIgnore];
      return dd;
    }
}

- (void) resume
{
  NSURLSessionTask  *task;

  task = [self task];

  if (_internalState == GSNativeProtocolInternalStateInitial)
    {
      NSAssert(nil != [task originalRequest], @"Task has no original request.");

      // Check if the cached response is good to use:
      if (nil != [self cachedResponse] 
        && [self canRespondFromCacheUsing: [self cachedResponse]])
        {
          [self setInternalState:
	    GSNativeProtocolInternalStateFulfillingFromCache];
          dispatch_async([task workQueue],
            ^{
              id<NSURLProtocolClient> client;

              client = [self client];
              [client URLProtocol: self 
                cachedResponseIsValid: [self cachedResponse]];
              [client URLProtocol: self 
                didReceiveResponse: [[self cachedResponse] response]
                cacheStoragePolicy: NSURLCacheStorageNotAllowed];
              if ([[[self cachedResponse] data] length] > 0)
                {
                  if ([client respondsToSelector:
		    @selector(URLProtocol:didLoad:)])
                    {
                      [client URLProtocol: self 
                              didLoadData: [[self cachedResponse] data]];
                    }
                }
              if ([client respondsToSelector:
		@selector(URLProtocolDidFinishLoading:)])
                {
                  [client URLProtocolDidFinishLoading: self];
                }
              [self setInternalState:
		GSNativeProtocolInternalStateTaskCompleted];
            });
        }
      else
        {
          [self startNewTransferWithRequest: [task originalRequest]];
        }
    }

  if (_internalState == GSNativeProtocolInternalStateTransferReady
    && nil != _transferState)
    {
      [self setInternalState: GSNativeProtocolInternalStateTransferInProgress];
    }
}

- (BOOL) canRespondFromCacheUsing: (NSCachedURLResponse*)response
{
  // Allows a native protocol to process a cached response. 
  // If `YES` is returned, the protocol will replay the cached response 
  // instead of starting a new transfer. The default implementation invalidates 
  // the response in the cache and returns `NO`.
  NSURLCache        *cache;
  NSURLSessionTask  *task;

  task = [self task];
  cache = [[[task session] configuration] URLCache];
  if (nil != cache && [task isKindOfClass: [NSURLSessionDataTask class]])
    {
      [cache removeCachedResponseForDataTask: (NSURLSessionDataTask*)task];
    }
  return NO;
}

- (void) suspend
{
  if (_internalState == GSNativeProtocolInternalStateTransferInProgress)
    {
      [self setInternalState: GSNativeProtocolInternalStateTransferReady];
    }
}

- (void) completeTaskWithError: (NSError*)error
{
  [[self task] setError: error];
  NSAssert(_internalState == GSNativeProtocolInternalStateTransferFailed, 
   @"Trying to complete the task, but its transfer isn't complete / failed.");

  // We don't want a timeout to be triggered after this. 
  // The timeout timer needs to be cancelled.
  [_easyHandle setTimeoutTimer: nil];

  [self setInternalState: GSNativeProtocolInternalStateTaskCompleted];
}

- (GSEasyHandleAction) didReceiveData: (NSData*)data
{
  NSURLResponse  *response;

  NSAssert(GSNativeProtocolInternalStateTransferInProgress == _internalState,
    @"Received body data, but no transfer in progress.");

  response = [self validateHeaderCompleteTransferState: _transferState];

  if (nil != response)
    {
      [_transferState setResponse: response];
    }

  [self notifyDelegateAboutReceivedData: data];

  _internalState = GSNativeProtocolInternalStateTransferInProgress;
  ASSIGN(_transferState, [_transferState byAppendingBodyData: data]);

  return GSEasyHandleActionProceed;
}

- (NSURLResponse*) validateHeaderCompleteTransferState: (GSTransferState*)ts 
{
  if (![ts isHeaderComplete]) 
    {
      NSAssert(NO, @"Received body data, but the header is not complete, yet.");
    }
  
  return nil;
}

- (void) notifyDelegateAboutReceivedData: (NSData*)data
{
  NSURLSessionTask              *task;
  id<NSURLSessionDelegate>      delegate;

  task = [self task];

  NSAssert(nil != task, @"Cannot notify");

  delegate = [[task session] delegate];
  if (nil != delegate
    && [task isKindOfClass: [NSURLSessionDataTask class]]
    && [delegate respondsToSelector: @selector(URLSession:dataTask:didReceiveData:)])
    {
      id<NSURLSessionDataDelegate> dataDelegate;
      NSURLSessionDataTask         *dataTask;
      NSURLSession                 *session;

      session = [task session];
      NSAssert(nil != session, @"Missing session");
      dataDelegate = (id<NSURLSessionDataDelegate>)delegate;
      dataTask = (NSURLSessionDataTask*)task;
      [[session delegateQueue] addOperationWithBlock:
        ^{
          [dataDelegate URLSession: session 
                          dataTask: dataTask 
                    didReceiveData: data];
        }];
    }
}

- (void) notifyDelegateAboutUploadedDataCount: (int64_t)count 
{
  NSURLSessionTask              *task;
  id<NSURLSessionDelegate>      delegate;

  task = [self task];

  NSAssert(nil != task, @"Cannot notify");

  delegate = [[task session] delegate];
  if (nil != delegate
    && [task isKindOfClass: [NSURLSessionUploadTask class]]
    && [delegate respondsToSelector: @selector(URLSession:task:didSendBodyData:totalBytesSent:totalBytesExpectedToSend:)])
    {
      id<NSURLSessionTaskDelegate> taskDelegate;
      NSURLSession                 *session;

      session = [task session];
      NSAssert(nil != session, @"Missing session");
      taskDelegate = (id<NSURLSessionTaskDelegate>)delegate;
      [[session delegateQueue] addOperationWithBlock:
        ^{
          [taskDelegate URLSession: session
                              task: task
                   didSendBodyData: count
                    totalBytesSent: [task countOfBytesSent]
          totalBytesExpectedToSend: [task countOfBytesExpectedToSend]];
        }];
    }
}

- (GSEasyHandleAction) didReceiveHeaderData: (NSData*)data 
                              contentLength: (int64_t)contentLength
{
  NSAssert(NO, @"Require concrete implementation");
  return GSEasyHandleActionAbort;
}

- (void) fillWriteBufferLength: (NSInteger)length
                        result: (void (^)(GSEasyHandleWriteBufferResult result, NSInteger length, NSData *data))result
{
  id<GSURLSessionTaskBodySource> source;

  NSAssert(GSNativeProtocolInternalStateTransferInProgress == _internalState,
    @"Requested to fill write buffer, but transfer isn't in progress.");
  
  source = [_transferState requestBodySource];

  NSAssert(nil != source, 
    @"Requested to fill write buffer, but transfer state has no body source.");

  if (nil == result) 
    {
      return;
    }

  [source getNextChunkWithLength: length
    completionHandler: ^(GSBodySourceDataChunk chunk, NSData *_Nullable data) 
      {
        switch (chunk) 
          {
            case GSBodySourceDataChunkData: 
              {
                NSUInteger count = [data length];
                [self notifyDelegateAboutUploadedDataCount: (int64_t)count];
                result(GSEasyHandleWriteBufferResultBytes, count, data);
                break;
              }
            case GSBodySourceDataChunkDone:
              result(GSEasyHandleWriteBufferResultBytes, 0, nil);
              break;
            case GSBodySourceDataChunkRetryLater:
              // At this point we'll try to pause the easy handle. The body
              // source is responsible for un-pausing the handle once data 
              // becomes available.
              result(GSEasyHandleWriteBufferResultPause, -1, nil);
              break;
            case GSBodySourceDataChunkError:
              result(GSEasyHandleWriteBufferResultAbort, -1, nil);
              break;
          }
      }];
}

- (void) transferCompletedWithError: (NSError*)error
{
  /* At this point the transfer is complete and we can decide what to do.
   * If everything went well, we will simply forward the resulting data
   * to the delegate. But in case of redirects etc. we might send another
   * request.
   */
  NSURLRequest        	*request;
  NSURLResponse       	*response;
  GSCompletionAction  	*action;

  if (nil != error) 
    {
      [self setInternalState: GSNativeProtocolInternalStateTransferFailed];
      [self failWithError: error request: [self request]];
      return;
    }

  NSAssert(_internalState == GSNativeProtocolInternalStateTransferInProgress, 
    @"Transfer completed, but it wasn't in progress.");

  request = [[self task] currentRequest];
  NSAssert(nil != request,
    @"Transfer completed, but there's no current request.");

  if (nil != [[self task] response]) 
    {
      [_transferState setResponse: [[self task] response]];
    }

  response = [_transferState response];
  NSAssert(nil != response, @"Transfer completed, but there's no response.");

  [self setInternalState: GSNativeProtocolInternalStateTransferCompleted];
  
  action = [self completeActionForCompletedRequest: request response: response];
  switch ([action type])
    {
      case GSCompletionActionTypeCompleteTask:
        [self completeTask];
        break;
      case GSCompletionActionTypeFailWithError:
        [self setInternalState: GSNativeProtocolInternalStateTransferFailed];
        error = [NSError errorWithDomain: NSURLErrorDomain 
                                    code: [action errorCode] 
                                userInfo: nil]; 
        [self failWithError: error request: request];
        break;
      case GSCompletionActionTypeRedirectWithRequest:
        [self redirectForRequest: [action redirectRequest]];
        break;
    }
}

- (GSCompletionAction*) completeActionForCompletedRequest: (NSURLRequest*)request
                                                 response: (NSURLResponse*)response
{
  GSCompletionAction  *action;

  action = AUTORELEASE([[GSCompletionAction alloc] init]);
  [action setType: GSCompletionActionTypeCompleteTask];

  return action;
}

- (void) completeTask
{
  NSURLSessionTask  *task;
  GSDataDrain       *bodyDataDrain;
  id<NSURLProtocolClient> client;

  NSAssert(_internalState == GSNativeProtocolInternalStateTransferCompleted,
    @"Trying to complete the task, but its transfer isn't complete.");
  
  task = [self task];
  [task setResponse: [_transferState response]];
  client = [self client];

  // We don't want a timeout to be triggered after this. The timeout timer 
  // needs to be cancelled.
  [_easyHandle setTimeoutTimer: nil];

  // because we deregister the task with the session on internalState being set 
  // to taskCompleted we need to do the latter after the delegate/handler was 
  // notified/invoked
  bodyDataDrain = [_transferState bodyDataDrain];
  if (GSDataDrainInMemory == [bodyDataDrain type])
    {
      NSData                  *data;
      
      if (nil != [bodyDataDrain data])
        {
          data = [NSData dataWithData: [bodyDataDrain data]];
        }
      else
        {
          data = [NSData data];
        }

      if ([client respondsToSelector: @selector(URLProtocol:didLoadData:)])
        {
          [client URLProtocol: self didLoadData: data];
        }
      [self setInternalState: GSNativeProtocolInternalStateTaskCompleted];
    }
  
  if ([client respondsToSelector: @selector(URLProtocolDidFinishLoading:)])
    {
      [client URLProtocolDidFinishLoading: self];
    }
}

- (void) redirectForRequest: (NSURLRequest*)request
{
  NSAssert(NO, @"Require concrete implementation");
}

- (void) failWithError: (NSError*)error request: (NSURLRequest*)request
{
  NSDictionary                 *info;
  NSError                      *urlError;
  id<NSURLProtocolClient>      client;
  
  info = [NSDictionary dictionaryWithObjectsAndKeys: 
    error, NSUnderlyingErrorKey,
    [request URL], NSURLErrorFailingURLErrorKey,
    [[request URL] absoluteString], NSURLErrorFailingURLStringErrorKey,
    [error localizedDescription], NSLocalizedDescriptionKey, nil];

  urlError = [NSError errorWithDomain: NSURLErrorDomain 
                                 code: [error code] 
                             userInfo: info];
  [self completeTaskWithError: urlError];

  client = [self client];
  if ([client respondsToSelector: @selector(URLProtocol:didFailWithError:)])
    {
      [client URLProtocol: self didFailWithError: urlError];
    }
}

- (BOOL) seekInputStreamToPosition: (uint64_t)position
{
  //TODO implement seek for NSURLSessionUploadTask
  return NO;
}

- (void) updateProgressMeterWithTotalBytesSent: (int64_t)totalBytesSent 
                      totalBytesExpectedToSend: (int64_t)totalBytesExpectedToSend 
                            totalBytesReceived: (int64_t)totalBytesReceived 
                   totalBytesExpectedToReceive: (int64_t)totalBytesExpectedToReceive
{
  // TODO: Update progress. Note that a single NSURLSessionTask might
  // perform multiple transfers. The values in `progress` are only for
  // the current transfer.
}

@end


