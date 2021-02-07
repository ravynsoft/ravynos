/** <title>NSAlert</title>

   <abstract>Encapsulate an alert panel</abstract>

   Copyright <copy>(C) 2004 Free Software Foundation, Inc.</copy>

   Author: Fred Kiefer <FredKiefer@gmx.de>
   Date: July 2004

   This file is part of the GNUstep GUI Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

#ifndef _GNUstep_H_NSAlert
#define _GNUstep_H_NSAlert
#import <GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSObject.h>

@class NSArray;
@class NSError;
@class NSString;
@class NSMutableArray;
@class NSButton;
@class NSImage;
@class NSWindow;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)

enum _NSAlertStyle { 
  NSWarningAlertStyle = 0, 
  NSInformationalAlertStyle = 1, 
  NSCriticalAlertStyle = 2 
};
typedef NSUInteger NSAlertStyle;

enum { 
  NSAlertFirstButtonReturn = 1000,
  NSAlertSecondButtonReturn = 1001,
  NSAlertThirdButtonReturn = 1002
};

@interface NSAlert : NSObject 
{
  @private
  NSString *_informative_text;
  NSString *_message_text;
  NSImage *_icon;
  NSMutableArray *_buttons;
  NSString *_help_anchor;
  NSWindow *_window;
  id _delegate;
  NSAlertStyle _style;
  BOOL _shows_help;
  id _modalDelegate;
  SEL _didEndSelector;
  NSInteger _result;
}

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
+ (NSAlert *) alertWithError: (NSError *)error;
#endif
+ (NSAlert *) alertWithMessageText: (NSString *)messageTitle
		     defaultButton: (NSString *)defaultButtonTitle
		   alternateButton: (NSString *)alternateButtonTitle
		       otherButton: (NSString *)otherButtonTitle
	 informativeTextWithFormat: (NSString *)format, ...;


- (NSButton *) addButtonWithTitle: (NSString *)aTitle;
- (NSAlertStyle) alertStyle;
- (void) beginSheetModalForWindow: (NSWindow *)window
		    modalDelegate: (id)delegate
		   didEndSelector: (SEL)didEndSelector
		      contextInfo: (void *)contextInfo;
- (NSArray *) buttons;
- (id) delegate;
- (NSString *) helpAnchor;
- (NSImage *) icon;
- (NSString *) informativeText;
- (NSString *) messageText;
- (NSInteger) runModal;
- (void) setAlertStyle: (NSAlertStyle)style;
- (void) setDelegate: (id)delegate;
- (void) setHelpAnchor: (NSString *)anchor;
- (void) setIcon: (NSImage *)icon;
- (void) setInformativeText: (NSString *)informativeText;
- (void) setMessageText: (NSString *)messageText;
- (void) setShowsHelp: (BOOL)showsHelp;
- (BOOL) showsHelp;
- (id) window;

@end


/*
 * Implemented by the delegate
 */

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
@protocol NSAlertDelegate <NSObject>
#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST) && GS_PROTOCOLS_HAVE_OPTIONAL
@optional
#else
@end
@interface NSObject (NSAlertDelegate)
#endif
- (BOOL) alertShowHelp: (NSAlert *)alert;
@end
#endif

#endif /* MAC_OS_X_VERSION_10_3 */
#endif /* _GNUstep_H_NSAlert */
