/* Interface for NSURLDownload for GNUstep
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

#ifndef __NSURLDownload_h_GNUSTEP_BASE_INCLUDE
#define __NSURLDownload_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_2,GS_API_LATEST) && GS_API_VERSION( 11300,GS_API_LATEST)

#import	<Foundation/NSObject.h>

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSData;
@class NSError;
@class NSString;
@class NSURLAuthenticationChallenge;
@class NSURLRequest;
@class NSURLResponse;

/**
 * Handles download to file.
 */
GS_EXPORT_CLASS
@interface NSURLDownload : NSObject
{
#if	GS_EXPOSE(NSURLDownload)
  void *_NSURLDownloadInternal;
#endif
}

/**
 * Returns a flag saying whether the class can resume a download
 * which was decoded with MIMEType.<br />
 */
+ (BOOL) canResumeDownloadDecodedWithEncodingMIMEType: (NSString *)MIMEType;

/**
 * Cancels the download and deletes any downloaded file.
 */
- (void) cancel;

/**
 * Returns a flag saying whether a partially downloaded file should be
 * deleted on failure ... YES by default.
 */
- (BOOL) deletesFileUponFailure;

/**
 * Initialises the receiver and start the download process.
 */
- (id) initWithRequest: (NSURLRequest *)request delegate: (id)delegate;

/** <init />
 * Initialises the receiver with data from a previous partial
 * download and resumes (or restarts) the downloading process.
 */
- (id) initWithResumeData: (NSData *)resumeData
		 delegate: (id)delegate
		     path: (NSString *)path;

/**
 * Returns the receiver's request.
 */
- (NSURLRequest *) request;

/**
 * Returns state data of an incomplete download ... this data should be
 * sufficient to resume/restart the download using the
 * -initWithResumeData:delegate:path: method.<br />
 * Returns nil if a resume is probably impossible.<br />
 * NB. You need to call -setDeletesFileUponFailure: to turn off deletion
 * if you wish to be able to resume an incomplete download.
 */
- (NSData *) resumeData;

/**
 * Sets a flag to determine if downloaded file is be deleted upon failure.
 * This is YES by default and needs to be set to NO if you want to be able
 * to attempt to resume a failed download.
 */
- (void) setDeletesFileUponFailure: (BOOL)deletesFileUponFailure;

/**
 * Sets the path to which the file is downloaded.<br />
 * May be called (once only) after initialisation of the receiver or when the
 * delegate receives a -download:decideDestinationWithSuggestedFilename:
 * message.<br />
 * Appends a number to the filename if allowOverwrite is NO and a file
 * already exists at path.<br />
 * See -download:didCreateDestination: also.
 */
- (void) setDestination: (NSString *)path allowOverwrite: (BOOL)allowOverwrite;

@end

/**
 * Protocol for delegate used to report the progress of the download.
 */

#if OS_API_VERSION(MAC_OS_X_VERSION_10_7, GS_API_LATEST)
@protocol NSURLDownloadDelegate <NSObject>
#if GS_PROTOCOLS_HAVE_OPTIONAL
@optional
#else
@end
@interface NSObject (NSURLDownloadDelegate)
#endif
#else
@interface NSObject (NSURLDownloadDelegate)
#endif

/**
 * Called immediately once the download has started.
 */
- (void) downloadDidBegin: (NSURLDownload *)download;

/**
 * Called when the download completes after having received all data.
 */
- (void) downloadDidFinish: (NSURLDownload *)download;

/**
 * Called when it's time to establish a name for the downloaded file ...
 * the delegate may decide a name by inspecting the response.<br />
 * The delegate should call -setDestination:allowOverwrite: to set the
 * filename to be used.
 */
- (void) download: (NSURLDownload *)download
 decideDestinationWithSuggestedFilename: (NSString *)filename;

/**
 * Called when authentication of a request is cancelled.
 */
- (void) download: (NSURLDownload *)download
  didCancelAuthenticationChallenge: (NSURLAuthenticationChallenge *)challenge;

/**
 * Called when the download has created the downloaded file.
 */
- (void) download: (NSURLDownload *)download
  didCreateDestination: (NSString *)path;

/**
 * Called when the download fails.
 */
- (void) download: (NSURLDownload *)download didFailWithError: (NSError *)error;

/**
 * Called when an authentication challenge is received.<br />
 * The delegate should send -useCredential:forAuthenticationChallenge: or
 * -continueWithoutCredentialForAuthenticationChallenge: or -cancel to
 * the connection sender when done.
 */
- (void) download: (NSURLDownload *)download
  didReceiveAuthenticationChallenge: (NSURLAuthenticationChallenge *)challenge;

/**
 * Called when some data has been received.
 */
- (void) download: (NSURLDownload *)download
  didReceiveDataOfLength: (NSUInteger)length;

/**
 * Called when a response is received.<br />
 * Multiple responses may be received on the same download (eg with server push)
 * and the delegate should be prepared to treat each separately.
 */
- (void) download: (NSURLDownload *)download
  didReceiveResponse: (NSURLResponse *)response;

/**
 * Called if the download file is encoded ... the delegate should return
 * YES if the downloaded data is to be decoded, NO otherwise.
 */
- (BOOL) download: (NSURLDownload *)download
  shouldDecodeSourceDataOfMIMEType: (NSString *)encodingType;

/**
 * Called when a download is resuming from previously stored data and
 * a response has been received from the server.<br />
 * The startingBytes is the offset from which the downloaded data
 * will actually commence ... and may be zero if the entire download
 * must be redone.
 */
- (void) download: (NSURLDownload *)download
  willResumeWithResponse: (NSURLResponse *)response
  fromByte: (long long)startingByte;

/**
 * Called if a new request has to be sent due to redirection.<br />
 * Must return the request argument (or a modified copy of it)
 * to have the process continue.
 */
- (NSURLRequest *) download: (NSURLDownload *)download
	    willSendRequest: (NSURLRequest *)request
	   redirectResponse: (NSURLResponse *)redirectResponse;

@end

#if	defined(__cplusplus)
}
#endif

#endif

#endif
