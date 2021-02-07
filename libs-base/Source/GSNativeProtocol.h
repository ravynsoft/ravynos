#ifndef	INCLUDED_GSNATIVEPROTOCOL_H
#define	INCLUDED_GSNATIVEPROTOCOL_H

#import "GSEasyHandle.h"
#import "Foundation/NSURLProtocol.h"

@class GSTransferState;

typedef NS_ENUM(NSUInteger, GSCompletionActionType) {
    GSCompletionActionTypeCompleteTask,
    GSCompletionActionTypeFailWithError,
    GSCompletionActionTypeRedirectWithRequest,
};

// Action to be taken after a transfer completes
@interface GSCompletionAction : NSObject
{
  GSCompletionActionType _type;
  int                    _errorCode;
  NSURLRequest           *_redirectRequest;
}

- (GSCompletionActionType) type;
- (void) setType: (GSCompletionActionType) type;

- (int) errorCode;
- (void) setErrorCode: (int)code;

- (NSURLRequest*) redirectRequest;
- (void) setRedirectRequest: (NSURLRequest*)request;

@end

typedef NS_ENUM(NSUInteger, GSNativeProtocolInternalState) {
    // Task has been created, but nothing has been done, yet
    GSNativeProtocolInternalStateInitial,
    // The task is being fulfilled from the cache rather than the network.
    GSNativeProtocolInternalStateFulfillingFromCache,
    // The easy handle has been fully configured. But it is not added to
    // the multi handle.
    GSNativeProtocolInternalStateTransferReady,
    // The easy handle is currently added to the multi handle
    GSNativeProtocolInternalStateTransferInProgress,
    // The transfer completed.
    // The easy handle has been removed from the multi handle. This does
    // not necessarily mean the task completed. A task that gets
    // redirected will do multiple transfers.
    GSNativeProtocolInternalStateTransferCompleted,
    // The transfer failed.
    // Same as `GSNativeProtocolInternalStateTransferCompleted`, 
    // but without response / body data
    GSNativeProtocolInternalStateTransferFailed,
    // Waiting for the completion handler of the HTTP redirect callback.
    // When we tell the delegate that we're about to perform an HTTP
    // redirect, we need to wait for the delegate to let us know what
    // action to take.
    GSNativeProtocolInternalStateWaitingForRedirectCompletionHandler,
    // Waiting for the completion handler of the 'did receive response' callback.
    // When we tell the delegate that we received a response (i.e. when
    // we received a complete header), we need to wait for the delegate to
    // let us know what action to take. In this state the easy handle is
    // paused in order to suspend delegate callbacks.
    GSNativeProtocolInternalStateWaitingForResponseCompletionHandler,
    // The task is completed
    // Contrast this with `GSNativeProtocolInternalStateTransferCompleted`.
    GSNativeProtocolInternalStateTaskCompleted,
};

// This abstract class has the common implementation of Native protocols like 
// HTTP, FTP, etc.
// These are libcurl helpers for the URLSession API code.
@interface GSNativeProtocol : NSURLProtocol <GSEasyHandleDelegate>
{
  GSEasyHandle                   *_easyHandle;
  GSNativeProtocolInternalState  _internalState;
  GSTransferState                *_transferState;
}

- (void) setInternalState: (GSNativeProtocolInternalState)newState;

- (void) failWithError: (NSError*)error request: (NSURLRequest*)request;

- (void) completeTaskWithError: (NSError*)error;

- (void) completeTask;

- (void) startNewTransferWithRequest: (NSURLRequest*)request;

@end

#endif
