/* Implementation for NSURLCredential for GNUstep
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

#define	EXPOSE_NSURLCredential_IVARS	1
#import "GSURLPrivate.h"

// Internal data storage
typedef struct {
  NSString			*user;
  NSString			*password;
  NSURLCredentialPersistence	persistence;
  BOOL				hasPassword;
} Internal;
 
#define	this	((Internal*)(self->_NSURLCredentialInternal))

@implementation	NSURLCredential

+ (id) allocWithZone: (NSZone*)z
{
  NSURLCredential	*o = [super allocWithZone: z];

  if (o != nil)
    {
      o->_NSURLCredentialInternal = NSZoneCalloc(z, 1, sizeof(Internal));
    }
  return o;
}

+ (NSURLCredential *) credentialWithUser: (NSString *)user
  password: (NSString *)password
  persistence: (NSURLCredentialPersistence)persistence
{
  NSURLCredential	*o = [self alloc];

  o = [o initWithUser: user password: password persistence: persistence];
  return AUTORELEASE(o);
}

- (id) copyWithZone: (NSZone*)z
{
  NSURLCredential	*o;

  if (NSShouldRetainWithZone(self, z) == YES)
    {
      o = RETAIN(self);
    }
  else
    {
      o = [[self class] allocWithZone: z];
      o = [o initWithUser: this->user
		 password: this->password
	      persistence: this->persistence];
    }
  return o;
}

- (void) dealloc
{
  if (this != 0)
    {
      RELEASE(this->user);
      RELEASE(this->password);
      NSZoneFree([self zone], this);
    }
  [super dealloc];
}

- (BOOL) hasPassword
{
  return this->hasPassword;
}

- (NSUInteger) hash
{
  return [this->user hash];
}

- (id) initWithUser: (NSString *)user
	   password: (NSString *)password
	persistence: (NSURLCredentialPersistence)persistence
{
  if (user == nil)
    {
      DESTROY(self);
      return nil;
    }
  if ((self = [super init]) != nil)
    {
      if (persistence == NSURLCredentialPersistenceSynchronizable)
	{
	  persistence = NSURLCredentialPersistencePermanent;
	}
      
      this->user = [user copy];
      this->password = [password copy];
      this->persistence = persistence;
      this->hasPassword = (this->password == nil) ? NO : YES;
      if (persistence == NSURLCredentialPersistencePermanent)
        {
	// FIXME ... should check to see if we really have a password
	}
    }
  return self;
}

- (BOOL) isEqual: (id)other
{
  if ((id)self == other)
    {
      return YES;
    }
  if ([other isKindOfClass: [NSURLCredential class]] == NO)
    {
      return NO;
    }
  return [[(NSURLCredential*)other user] isEqualToString: this->user]
    && [[(NSURLCredential*)other password] isEqualToString: this->password]
    && [(NSURLCredential*)other persistence] == this->persistence;
}

- (NSString *) password
{
  return this->password;
}

- (NSURLCredentialPersistence) persistence
{
  return this->persistence;
}

- (NSString *) user
{
  return this->user;
}

@end

