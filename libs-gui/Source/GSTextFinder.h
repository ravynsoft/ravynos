/*                                                    -*-objc-*-
   GSTextFinder.h

   The private text finder class for NSTextView

   Copyright (C) 2010 Free Software Foundation, Inc.

   Author: Wolfgang Lux <wolfgang.lux@gmail.com>
   Date: 2010
   
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

#ifndef _GS_TEXT_FINDER_H
#define _GS_TEXT_FINDER_H

#import <Foundation/NSObject.h>

@class NSString;
@class NSButton;
@class NSMatrix;
@class NSPanel;
@class NSTextField;

@interface GSTextFinder : NSObject
{
  // local attributes
  NSString *findString;
  NSString *replaceString;

  // GUI
  NSPanel *panel;
  NSTextField *findText;
  NSTextField *replaceText;
  NSTextField *messageText;
  NSMatrix *replaceScopeMatrix;
  NSButton *ignoreCaseButton;
}

// return shared panel instance
+ (GSTextFinder *) sharedTextFinder;

// UI actions
- (void) findNext: (id)sender;
- (void) findPrevious: (id)sender;
- (void) replaceAndFind: (id)sender;
- (void) replace: (id)sender;
- (void) replaceAll: (id)sender;
- (void) performFindPanelAction: (id)sender;
- (void) performFindPanelAction: (id)sender
		   withTextView: (NSTextView *)aTextView;
- (BOOL) validateFindPanelAction: (id)sender
		    withTextView: (NSTextView *)aTextView;

// text finder methods
- (void) showFindPanel;
- (void) takeFindStringFromTextView: (NSText *)aTextView;
- (BOOL) findStringInTextView: (NSText *)aTextView forward: (BOOL)forward;
- (void) replaceStringInTextView: (NSTextView *)aTextView;
- (void) replaceAllInTextView: (NSTextView *)aTextView
	      onlyInSelection: (BOOL)flag;
- (NSTextView *) targetView: (NSTextView *)aTextView;

@end

#endif /* _GS_TEXT_FINDER_H */
