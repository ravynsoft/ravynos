/* 
   NSOpenPanel.h

   Standard panel for opening files

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author:  Scott Christley <scottc@net-community.com>
   Date: 1996
   Author:  Daniel Böhringer <boehring@biomed.ruhr-uni-bochum.de>
   Date: August 1998
   Source by Daniel Böhringer integrated into Scott Christley's preliminary
   implementation by Felipe A. Rodriguez <far@ix.netcom.com> 

   Author: Nicola Pero <n.pero@mi.flashnet.it>
   Date: 1999

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

#ifndef _GNUstep_H_NSOpenPanel
#define _GNUstep_H_NSOpenPanel
#import <GNUstepBase/GSVersionMacros.h>

#import <AppKit/NSSavePanel.h>

@class NSString;
@class NSArray;

@interface NSOpenPanel : NSSavePanel
{
  BOOL _canChooseDirectories;
  BOOL _canChooseFiles;
}
// Accessing the NSOpenPanel shared instance
+ (NSOpenPanel *) openPanel;

// Running an NSOpenPanel 
- (NSInteger) runModalForTypes: (NSArray *)fileTypes;
- (NSInteger) runModalForDirectory: (NSString *)path
                              file: (NSString *)name
                             types: (NSArray *)fileTypes;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (NSInteger) runModalForDirectory: (NSString *)path
                              file: (NSString *)name
                             types: (NSArray *)fileTypes
                  relativeToWindow: (NSWindow*)window;
- (void) beginSheetForDirectory: (NSString *)path
			   file: (NSString *)name
			  types: (NSArray *)fileTypes
		 modalForWindow: (NSWindow *)docWindow
		  modalDelegate: (id)delegate
		 didEndSelector: (SEL)didEndSelector
		    contextInfo: (void *)contextInfo;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
- (void) beginForDirectory: (NSString *)absoluteDirectoryPath
                      file: (NSString *)filename
                     types: (NSArray *)fileTypes
          modelessDelegate: (id)modelessDelegate
            didEndSelector: (SEL)didEndSelector
               contextInfo: (void *)contextInfo;
#endif

- (NSArray *) filenames;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (NSArray *) URLs; 
#endif

// Filtering Files 
- (BOOL) canChooseDirectories;
- (BOOL) canChooseFiles;
- (void) setCanChooseDirectories: (BOOL)flag;
- (void) setCanChooseFiles: (BOOL)flag;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_1, GS_API_LATEST)
- (void) setResolvesAliases: (BOOL)flag; 
- (BOOL) resolvesAliases; 
#endif

- (BOOL) allowsMultipleSelection;
- (void) setAllowsMultipleSelection: (BOOL)flag;
@end

#endif // _GNUstep_H_NSOpenPanel
