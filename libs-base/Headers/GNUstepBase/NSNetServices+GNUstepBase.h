/* Declarations of GNUstep extensions to NSNetService.
   Copyright (C) 2010 Free Software Foundation, Inc.

   Written by:  Niels Grewe <niels.grewe@halbordnung.de>
   Date: March 2010

   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02111 USA.
   */

#import <Foundation/NSNetServices.h>
#import <GNUstepBase/GSConfig.h>

/*
 * Only enable extensions if the Avahi API is used. The mDNS based NSNetService
 * doesn't support them yet.
 */
#if GS_USE_AVAHI==1
@interface NSNetService (GNUstepBase)
/**
 * Starts monitoring the service represented by the receiver for changes in a
 * resource record of <code>recordType</code>. The delegate must either
 * implement a -netService:didUpateFOORecordData: method, where "FOO" is the
 * record type monitored, or the generic
 * -netService:didUpdateRecordData:forRecordType: method.
 */
- (void) startMonitoringForRecordType: (NSString*)recordType;

/**
 * Stops monitoring for the specified record type.
 */
- (void) stopMonitoringForRecordType: (NSString*)recordType;

/**
 * Adds the service record for a service set up for publishing. The return value
 * of this method indicates whether the record was successfully added. The
 * record will only be published when -publish is called and it is thus
 * considered an error to call this method on a published service.
 */
- (BOOL) addServiceRecord;

/**
 * Sets <code>data</code> as the value of a resource record of the specified
 * <code>type</code>. The return value of this method indicates whether the
 * record was successfully added. The record will only be published when
 * -publish is called and it is thus considered an error to call this method on
 * a published service. (But it is possible to add a new record to a published
 * NSNetService by creating a new NSNetService instance with the same name,
 * type, domain and port parameters and publish only the new record in that
 * service.)
 */
- (BOOL) addRecordData: (NSData*)data
         forRecordType: (NSString*)type;

/**
 * Returns the record data for the specified record type. If there is only one
 * such record, the return value will be an NSData object, otherwise it will be
 * an NSArray.
 */
- (id) recordDataForRecordType: (NSString*)type;

/**
 * Returns the full name (including, name, type, and domain) of the service.
 */
- (NSString*) fullServiceName;
@end


@protocol  GSNetServiceDelegate <NSNetServiceDelegate>
#if GS_PROTOCOLS_HAVE_OPTIONAL
@optional
#else
@end
@interface NSObject (GSNetServiceDelegateMethods)
#endif // GS_PROTOCOLS_HAVE_OPTIONAL

/**
 * Notifies the delegate that the addresses for the <var>service</var> have
 * changed.
 */
- (void)netService: (NSNetService*)service
didUpdateAddresses: (NSArray*)addresses;

/**
 * Notifies the delegate that the record of the <var>type</var> given
 * has changed.
 */
-  (void)netService: (NSNetService*)service
didUpdateRecordData: (id)newData
      forRecordType: (NSString*)type;

/**
 * Notifies the delegate that there was a problem when trying to monitor a
 * resource record of <var>type</var> for updates.
 */
- (void)netService: (NSNetService*)service
     didNotMonitor: (NSDictionary*)errorDict
     forRecordType: (NSString*)type;
@end

#endif // GS_USE_AVAHI
