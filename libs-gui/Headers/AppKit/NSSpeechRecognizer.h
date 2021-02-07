/* Definition of class NSSpeechRecognizer
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

#ifndef _NSSpeechRecognizer_h_GNUSTEP_GUI_INCLUDE
#define _NSSpeechRecognizer_h_GNUSTEP_GUI_INCLUDE

#import <Foundation/NSObject.h>
#import <AppKit/AppKitDefines.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_0, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

@protocol NSSpeechRecognizerDelegate;

@class NSArray, NSString, NSUUID;
  
@interface NSSpeechRecognizer : NSObject
{
  id<NSSpeechRecognizerDelegate> _delegate;
  NSArray *_commands;
  NSString *_displayCommandsTitle;
  NSUUID *_uuid;
  BOOL _blocksOtherRecognizers;
  BOOL _listensInForegroundOnly;
  BOOL _appInForeground; // private
  BOOL _isListening;
}

// Initialize
- (instancetype) init;

- (id<NSSpeechRecognizerDelegate>) delegate;
- (void) setDelegate: (id<NSSpeechRecognizerDelegate>) delegate;
  
// Configuring...
- (NSArray *) commands;
- (void) setCommands: (NSArray *)commands;

- (NSString *) displayCommandsTitle;
- (void) setDisplayCommandsTitle: (NSString *)displayCommandsTitle;

- (BOOL) listensInForegroundOnly;
- (void) setListensInForegroundOnly: (BOOL)listensInForegroundOnly;

- (BOOL) blocksOtherRecognizers;
- (void) setBlocksOtherRecognizers: (BOOL)blocksOtherRecognizers;

// Listening
- (void) startListening;
- (void) stopListening;
  
@end

// Protocol
@protocol NSSpeechRecognizerDelegate
- (void) speechRecognizer: (NSSpeechRecognizer *)sender
      didRecognizeCommand: (NSString *)command;
@end

APPKIT_EXPORT NSString *GSSpeechRecognizerDidRecognizeWordNotification; 

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSSpeechRecognizer_h_GNUSTEP_GUI_INCLUDE */

