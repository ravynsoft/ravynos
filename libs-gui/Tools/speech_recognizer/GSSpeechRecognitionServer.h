/* Interface of class GSSpeechRecognitionServer
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   By: Gregory John Casamento
   Date: Fri Dec  6 04:55:59 EST 2019

   This file is part of the GNUstep Library.
   
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

#import <Foundation/Foundation.h>
#import <AppKit/NSSpeechRecognizer.h>

@class GSSpeechRecognitionEngine;

/**
 * GSSpeechRecognitionServer handles all of the engine-agnostic operations.  Currently,
 * there aren't any, but when the on-screen text interface is added it should
 * go in here.
 */
@interface GSSpeechRecognitionServer : NSObject
{
  GSSpeechRecognitionEngine *_engine;
  NSMutableArray *_blocking;
}

/**
 * Returns a shared instance of the speech server.
 */
+ (id)sharedServer;

// Add or remove from blocking list...
- (void) addToBlockingRecognizers: (NSString *)s;
- (void) removeFromBlockingRecognizers: (NSString *)s;
- (BOOL) isBlocking: (NSString *)s;

// Connection...
- (void) addClient;
@end
