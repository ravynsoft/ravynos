/** Interface for NSFileHandle for GNUStep
   Copyright (C) 1997 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: 1997

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

    AutogsdocSource: NSFileHandle.m
    AutogsdocSource: NSPipe.m
   */

#ifndef __NSFileHandle_h_GNUSTEP_BASE_INCLUDE
#define __NSFileHandle_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#import	<Foundation/NSObject.h>
#import	<Foundation/NSRange.h>

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSData;
@class NSError;
@class NSString;
@class NSURL;

GS_EXPORT_CLASS
@interface NSFileHandle : NSObject

// Allocating and Initializing a FileHandle Object

+ (instancetype) fileHandleForReadingAtPath: (NSString*)path;
+ (instancetype) fileHandleForWritingAtPath: (NSString*)path;
+ (instancetype) fileHandleForUpdatingAtPath: (NSString*)path;
+ (instancetype) fileHandleWithStandardError;
+ (instancetype) fileHandleWithStandardInput;
+ (instancetype) fileHandleWithStandardOutput;
+ (instancetype) fileHandleWithNullDevice;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
+ (instancetype) fileHandleForReadingFromURL: (NSURL*)url error:(NSError**)error;
+ (instancetype) fileHandleForWritingToURL: (NSURL*)url error:(NSError**)error;
+ (instancetype) fileHandleForUpdatingURL: (NSURL*)url error:(NSError**)error;
#endif

- (id) initWithFileDescriptor: (int)desc;
- (id) initWithFileDescriptor: (int)desc closeOnDealloc: (BOOL)flag;
- (id) initWithNativeHandle: (void*)hdl;
- (id) initWithNativeHandle: (void*)hdl closeOnDealloc: (BOOL)flag;

// Returning file handles

- (int) fileDescriptor;
- (void*) nativeHandle;

// Synchronous I/O operations

- (NSData*) availableData;
- (NSData*) readDataToEndOfFile;
- (NSData*) readDataOfLength: (unsigned int)len;
- (void) writeData: (NSData*)item;

// Asynchronous I/O operations

- (void) acceptConnectionInBackgroundAndNotify;
- (void) acceptConnectionInBackgroundAndNotifyForModes: (NSArray*)modes;
- (void) readInBackgroundAndNotify;
- (void) readInBackgroundAndNotifyForModes: (NSArray*)modes;
- (void) readToEndOfFileInBackgroundAndNotify;
- (void) readToEndOfFileInBackgroundAndNotifyForModes: (NSArray*)modes;
- (void) waitForDataInBackgroundAndNotify;
- (void) waitForDataInBackgroundAndNotifyForModes: (NSArray*)modes;

// Seeking within a file

- (unsigned long long) offsetInFile;
- (unsigned long long) seekToEndOfFile;
- (void) seekToFileOffset: (unsigned long long)pos;

// Operations on file

- (void) closeFile;
- (void) synchronizeFile;
- (void) truncateFileAtOffset: (unsigned long long)pos;

@end

// Notification names.

/**
 * Posted when one of the [NSFileHandle] methods
 * <code>acceptConnectionInBackground...</code> succeeds and has connected to a
 * stream-type socket in another process.  The notification's
 * <em>userInfo</em> dictionary will contain the [NSFileHandle] for the near
 * end of the connection (associated to the key
 * '<code>NSFileHandleNotificationFileHandleItem</code>').
 */
GS_EXPORT NSString * const NSFileHandleConnectionAcceptedNotification;

/**
 * Posted when one of the [NSFileHandle] methods
 * <code>waitForDataInBackground...</code> has been informed that data is
 * available.  The receiving [NSFileHandle] is passed in the notification.
 */
GS_EXPORT NSString * const NSFileHandleDataAvailableNotification;

/**
 * Posted when one of the [NSFileHandle] methods readDataInBackground... has
 * consumed data.  The receiving [NSFileHandle] is passed in the
 * notification's <em>userInfo</em> dictionary associated to the key
 * '<code>NSFileHandleNotificationDataItem</code>'.
 */
GS_EXPORT NSString * const NSFileHandleReadCompletionNotification;

/**
 * Posted when one of the [NSFileHandle] methods
 * <code>readToEndOfFileInBackground...</code> has finished.  The receiving
 * [NSFileHandle] is passed in the notification's <em>userInfo</em> dictionary
 * associated to the key '<code>NSFileHandleNotificationDataItem</code>'.
 */
GS_EXPORT NSString * const NSFileHandleReadToEndOfFileCompletionNotification;

// Keys for accessing userInfo dictionary in notification handlers.

/**
 * Dictionary key for [NSFileHandle] notifications used to access an
 * [NSData] object containing received data.
 */
GS_EXPORT NSString * const NSFileHandleNotificationDataItem;

/**
  * Dictionary key for [NSFileHandle] notifications used to mark the
  * [NSFileHandle] that has established a stream-socket connection.
 */
GS_EXPORT NSString * const NSFileHandleNotificationFileHandleItem;

/**
 * Dictionary key for [NSFileHandle] notifications postable to certain run
 * loop modes, associated to an NSArray containing the modes allowed.
 */
GS_EXPORT NSString * const NSFileHandleNotificationMonitorModes;

// Exceptions

/**
 * Exception raised when attempts to read from an [NSFileHandle] channel fail.
 */
GS_EXPORT NSString * const NSFileHandleOperationException;

GS_EXPORT_CLASS
@interface NSPipe : NSObject
{
#if	GS_EXPOSE(NSPipe)
@private
  NSFileHandle	*_readHandle;
  NSFileHandle	*_writeHandle;
#endif
#if     GS_NONFRAGILE
#else
  /* Pointer to private additional data used to avoid breaking ABI
   * when we don't have the non-fragile ABI available.
   * Use this mechanism rather than changing the instance variable
   * layout (see Source/GSInternal.h for details).
   */
  @private id _internal GS_UNUSED_IVAR;
#endif
}
+ (id) pipe;
- (NSFileHandle*) fileHandleForReading;
- (NSFileHandle*) fileHandleForWriting;
@end



#if OS_API_VERSION(GS_API_NONE, GS_API_NONE)

// GNUstep class extensions

@interface NSFileHandle (GNUstepExtensions)
+ (id) fileHandleAsServerAtAddress: (NSString*)address
			   service: (NSString*)service
			  protocol: (NSString*)protocol;
+ (id) fileHandleAsClientAtAddress: (NSString*)address
			   service: (NSString*)service
			  protocol: (NSString*)protocol;
+ (id) fileHandleAsClientInBackgroundAtAddress: (NSString*)address
				       service: (NSString*)service
				      protocol: (NSString*)protocol;
+ (id) fileHandleAsClientInBackgroundAtAddress: (NSString*)address
				       service: (NSString*)service
				      protocol: (NSString*)protocol
				      forModes: (NSArray*)modes;
- (void) readDataInBackgroundAndNotifyLength: (unsigned)len;
- (void) readDataInBackgroundAndNotifyLength: (unsigned)len
				    forModes: (NSArray*)modes;
- (BOOL) readInProgress;
- (NSString*) socketAddress;
- (NSString*) socketLocalAddress;
- (NSString*) socketLocalService;
- (NSString*) socketService;
- (NSString*) socketProtocol;
- (BOOL) useCompression;
- (void) writeInBackgroundAndNotify: (NSData*)item forModes: (NSArray*)modes;
- (void) writeInBackgroundAndNotify: (NSData*)item;
- (BOOL) writeInProgress;
@end

/**
 * Where OpenSSL is available, you can use the subclass returned by +sslClass
 * to handle SSL connections.<br />
 * The -sslAccept method is used to do SSL handshake and start an
 * encrypted session on a channel where the connection was initiated
 * from the far end.<br />
 * The -sslConnect method is used to do SSL handshake and start an
 * encrypted session on a channel where the connection was initiated
 * from the near end.<br />
 * The -sslDisconnect method is used to end the encrypted session.
 * The -sslSetCertificate:privateKey:PEMpasswd: method is used to
 * establish a client certificate before starting an encrypted session.<br />
 * NB. Some of these methods may block while performing I/O on the network
 * connection, (though they should run the current runloop while doing so)
 * so you should structure your code to handle that.  In particular, if you
 * are writing a server application, you should initiate a background accept
 * to allow another incoming connection <em>before</em> you perform an
 * -sslAccept on a connection you have just accepted.
 */
@interface NSFileHandle (GNUstepTLS)

/** Returns the class to handle ssl enabled connections.
 */
+ (Class) sslClass;

/** Repeatedly attempt an incoming handshake for up to 30 seconds or until
 * the handshake completes.
 */
- (BOOL) sslAccept;

/** Repeatedly attempt an outgoing handshake for up to 30 seconds or until
 * the handshake completes.
 */
- (BOOL) sslConnect;

/** <override-dummy />
 * Shuts down the SSL connection to the system that the handle is talking to.
 */
- (void) sslDisconnect;

/** <override-dummy />
 * Make a non-blocking handshake attempt.  Calls to this method should be
 * repeated until the method returns YES indicating that the handshake
 * completed.  If the method returns YES indicating completion of the
 * handshake, the result indicates whether the handshake succeeded in
 * establishing a connection or not.<br />
 * The default implementation simply returns YES and sets result to NO.<br />
 * This is implemented by an SSL handling subclass to perform real work.
 */
- (BOOL) sslHandshakeEstablished: (BOOL*)result outgoing: (BOOL)isOutgoing;

/** If the session verified a certificate from the remote end, returns the
 * name of the certificate issuer in the form "C=xxxx,O=yyyy,CN=zzzz" as
 * described in RFC2253.  Otherwise returns nil.
 */
- (NSString*) sslIssuer;

/** If the session verified a certificate from the remote end, returns the
 * name of the certificate owner in the form "C=xxxx,O=yyyy,CN=zzzz" as
 * described in RFC2253.  Otherwise returns nil.
 */
- (NSString*) sslOwner;

/** Deprecated ... use -sslSetOptions: instead
 */
- (void) sslSetCertificate: (NSString*)certFile
                privateKey: (NSString*)privateKey
                 PEMpasswd: (NSString*)PEMpasswd;

/** <override-dummy />
 * Sets options to be used to configure this channel before the handshake.<br />
 * Returns nil on success, or an error message if some options could not
 * be set.<br />
 * You may use the same options as property settings with the GNUstep
 * implementation of NSStream.<br />
 * Expects key value pairs with the following names/meanings:
 * <deflist>
 *   <term>GSTLSCAFile</term>
 *   <desc>A string identifying the full path to the file containing any
 *   trusted certificate authorities to be used when verifying a certificate
 *   presented by the remote end of a connection.
 *   </desc>
 *   <term>GSTLSCertificateFile</term>
 *   <desc>The path to a PEM encoded certificate used to identify this end
 *   of the connection.  This option <em>must</em> be set for handing an
 *   incoming connection, but is optional for outgoing connections.<br />
 *   This must be used in conjunction with GSTLSCertificateKeyFile.
 *   </desc>
 *   <term>GSTLSCertificateKeyFile</term>
 *   <desc>The path to a PEM encoded key used to unlock the certificate
 *   file for the connection.  The key in the file may or may not be
 *   encrypted, but if it is encrypted you must specify
 *   GSTLSCertificateKeyPassword.
 *   </desc>
 *   <term>GSTLSCertificateKeyPassword</term>
 *   <desc>A string to be used as the password to decrypt a key which was
 *   specified using GSTLSKeyPassword.
 *   </desc>
 *   <term>GSTLSDebug</term>
 *   <desc>A boolean specifying whether diagnostic debug is to be enabled
 *   to log information about a connection where the handshake fails.<br />
 *   </desc>
 *   <term>GSTLSPriority</term>
 *   <desc>A GNUTLS priority string describing the ciphers etc which may be
 *   used for the connection.  In addition the string may be one of
 *   SSLv3, or TLSv1 to use the appropriate general settings
 *   for negotiating a connection of the specified type.
 *   </desc>
 *   <term>GSTLSRemoteHosts</term>
 *   <desc>A comma delimited list of host names to be allowed when verifying
 *   the certificate of the host we are connecting to.<br />
 *   If this is not specified, all the names provided by NSHost are used.
 *   </desc>
 *   <term>GSTLSRevokeFile</term>
 *   <desc>The full path of a file containing certificate revocation
 *   information for certificates issued by our trusted authorites but
 *   no longer valid.
 *   </desc>
 *   <term>GSTLSServerName</term>
 *   <desc>By default the TLS layer when making an HTTPS request sets the
 *   'Server Name Indication' (SNI) to be the name of the host in the URL
 *   that is being fetched.<br />
 *   This option allows the SNI to be set for other connections and permits
 *   overriding of the default behavior for HTTPS requests.  Setting the
 *   value of GSTLSServerName to an empty string will prevent the SNI from
 *   being sent in the TLS handshake (this is sometimes desirable to prevent
 *   information leakage; the SNI information is sent unencrypted).<br />
 *   Some web servers require SNI in order to tell what hostname an HTTPS
 *   request is for and decide which certificate to present to the client.
 *   </desc>
 *   <term>GSTLSVerify</term>
 *   <desc>A boolean specifying whether we should require the remote end to
 *   supply a valid certificate in order to establish an encrypted connection.
 *   </desc>
 * </deflist>
 */
- (NSString*) sslSetOptions: (NSDictionary*)options;

/** Sets the known (cached) data content for the specified file name.<br />
 * Calling this with a nil data object will remove any existing value
 * from the cache.<br />
 * You may use this method to control what data is used for specified
 * file names when those file names are used as a result of SSL/TLS
 * options being set for a file handle or stream.
 */
+ (void) setData: (NSData*)data forTLSFile: (NSString*)fileName;

@end

/** Dictionary key for the path to a PEM encoded certificate authority
 * file.
 */
GS_EXPORT NSString * const GSTLSCAFile;

/** Dictionary key for the path to a PEM encoded certificate used
 * to identify this end of a connection.
 */
GS_EXPORT NSString * const GSTLSCertificateFile;

/** Dictionary key for the path to a PEM encoded private key used
 * to unlock the certificate used by this end of a connection.
 */
GS_EXPORT NSString * const GSTLSCertificateKeyFile;

/** Dictionary key for the password used to decrypt the key file used
 * to unlock the certificate used by this end of a connection.
 */
GS_EXPORT NSString * const GSTLSCertificateKeyPassword;

/** Dictionary key for a boolean to enable TLS debug for a session.
 */
GS_EXPORT NSString * const GSTLSDebug;

/** Dictionary key for a GNUTLS priority setting for a session.
 */
GS_EXPORT NSString * const GSTLSPriority;

/** Dictionary key for a list of hosts to use in certificate verification.
 */
GS_EXPORT NSString * const GSTLSRemoteHosts;

/** Dictionary key for the path to a PEM encoded certificate revocation
 * file.
 */
GS_EXPORT NSString * const GSTLSRevokeFile;

/** Dictionary key for the value controlling the Server Name Indication
 * (SNI) sent as part of the TLS handshake.
 */
GS_EXPORT NSString * const GSTLSServerName;

/** Dictionary key for a boolean to enable certificate verification.
 */
GS_EXPORT NSString * const GSTLSVerify;

// GNUstep Notification names.

/**
 * Notification posted when an asynchronous [NSFileHandle] connection
 * attempt (to an FTP, HTTP, or other internet server) has succeeded.
 */
GS_EXPORT NSString * const GSFileHandleConnectCompletionNotification;

/**
 * Notification posted when an asynchronous [NSFileHandle] write
 * operation (to an FTP, HTTP, or other internet server) has succeeded.
 */
GS_EXPORT NSString * const GSFileHandleWriteCompletionNotification;

/**
 * Message describing error in asynchronous [NSFileHandle] accept,read,write
 * operation.
 */
GS_EXPORT NSString * const GSFileHandleNotificationError;

#endif

#if	defined(__cplusplus)
}
#endif

#if     !NO_GNUSTEP && !defined(GNUSTEP_BASE_INTERNAL)
#import <GNUstepBase/NSFileHandle+GNUstepBase.h>
#endif

#endif /* __NSFileHandle_h_GNUSTEP_BASE_INCLUDE */
