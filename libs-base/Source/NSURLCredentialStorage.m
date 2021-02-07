/* Implementation for NSURLCredentialStorage for GNUstep
   Copyright (C) 2006 Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <rfm@gnu.org>
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

#import "common.h"

#define	EXPOSE_NSURLCredentialStorage_IVARS	1
#import "GSURLPrivate.h"

NSString *const NSURLCredentialStorageChangedNotification
  = @"NSURLCredentialStorageChangedNotification";

// Internal data storage
typedef struct {
  NSMutableDictionary	*credentials;
  NSMutableDictionary	*defaults;
} Internal;
 
#define	this	((Internal*)(self->_NSURLCredentialStorageInternal))

@implementation	NSURLCredentialStorage

static NSURLCredentialStorage	*storage = nil;

+ (id) allocWithZone: (NSZone*)z
{
  return RETAIN([self sharedCredentialStorage]);
}

+ (NSURLCredentialStorage *) sharedCredentialStorage
{
  if (storage == nil)
    {
      [gnustep_global_lock lock];
      if (storage == nil)
        {
	  NSURLCredentialStorage	*o;

	  o = (NSURLCredentialStorage*)
	    NSAllocateObject(self, 0, NSDefaultMallocZone());
	  o->_NSURLCredentialStorageInternal = (Internal*)
	    NSZoneCalloc(NSDefaultMallocZone(), 1, sizeof(Internal));
	  ((Internal*)(o->_NSURLCredentialStorageInternal))->credentials
	    = [NSMutableDictionary new];
	  ((Internal*)(o->_NSURLCredentialStorageInternal))->defaults
	    = [NSMutableDictionary new];
	  storage = o;
	}
      [gnustep_global_lock unlock];
    }
  return storage;
}

- (NSDictionary *) allCredentials
{
  NSMutableDictionary	*all;
  NSEnumerator		*enumerator;
  NSURLProtectionSpace	*space;

  all = [NSMutableDictionary dictionaryWithCapacity: [this->credentials count]];
  enumerator = [this->credentials keyEnumerator];
  while ((space = [enumerator nextObject]) != nil)
    {
      NSDictionary	*info = [[this->credentials objectForKey: space] copy];

      [all setObject: info forKey: space];
      RELEASE(info);
    }
  return all;
}

- (NSDictionary *) credentialsForProtectionSpace: (NSURLProtectionSpace *)space
{
  return AUTORELEASE([[this->credentials objectForKey: space] copy]);
}

- (void) dealloc
{
  [NSException raise: NSInternalInconsistencyException
  	      format: @"Attempt to deallocate credential storage"];
  GSNOSUPERDEALLOC;
}

- (NSURLCredential *) defaultCredentialForProtectionSpace:
  (NSURLProtectionSpace *)space
{
  return [this->defaults objectForKey: space];
}

// Should never be called.
- (id) init
{
  DESTROY(self);
  return nil;
}

- (void) removeCredential: (NSURLCredential *)credential
       forProtectionSpace: (NSURLProtectionSpace *)space
{
  if (credential == nil || ![credential isKindOfClass: [NSURLCredential class]])
    {
      [NSException raise: NSInvalidArgumentException
      		  format: @"[%@-%@] nil or bad  class for credential",
		  NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
    }
  if (space == nil || ![space isKindOfClass: [NSURLProtectionSpace class]])
    {
      [NSException raise: NSInvalidArgumentException
      		  format: @"[%@-%@] nil or bad  class for space",
		  NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
    }
  [[this->credentials objectForKey: space]
    removeObjectForKey: [credential user]];
}

/**
 * Sets credential in the storage for the protection space specified.<br />
 * This replaces any old value with the same username.
 */
- (void) setCredential: (NSURLCredential *)credential
    forProtectionSpace: (NSURLProtectionSpace *)space
{
  NSMutableDictionary	*info;

  if (credential == nil || ![credential isKindOfClass: [NSURLCredential class]])
    {
      [NSException raise: NSInvalidArgumentException
      		  format: @"[%@-%@] nil or bad  class for credential",
		  NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
    }
  if (space == nil || ![space isKindOfClass: [NSURLProtectionSpace class]])
    {
      [NSException raise: NSInvalidArgumentException
      		  format: @"[%@-%@] nil or bad  class for space",
		  NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
    }
  info = [this->credentials objectForKey: space];
  if (info == nil)
    {
      info = [NSMutableDictionary new];
      [this->credentials setObject: info forKey: space];
      RELEASE(info);
    }
  [info setObject: credential forKey: [credential user]];
}

/**
 * Sets the default credential for the protection space.  Also calls
 * -setCredential:forProtectionSpace: if the credential has not already
 * been set in space.
 */
- (void) setDefaultCredential: (NSURLCredential *)credential
	   forProtectionSpace: (NSURLProtectionSpace *)space
{
  if (credential == nil || ![credential isKindOfClass: [NSURLCredential class]])
    {
      [NSException raise: NSInvalidArgumentException
      		  format: @"[%@-%@] nil or bad  class for credential",
		  NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
    }
  if (space == nil || ![space isKindOfClass: [NSURLProtectionSpace class]])
    {
      [NSException raise: NSInvalidArgumentException
      		  format: @"[%@-%@] nil or bad  class for space",
		  NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
    }
  [this->defaults setObject: credential forKey: space];
  if ([[this->credentials objectForKey: space] objectForKey: [credential user]]
    != credential)
    {
      [self setCredential: credential forProtectionSpace: space];
    }
}

@end

