/* 
   NSDocumentFramworkPrivate.h

   The private methods of all the classes of the document framework

   Copyright (C) 1999 Free Software Foundation, Inc.

   Author: Carl Lindberg <Carl.Lindberg@hbo.com>
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

#ifndef _GNUstep_H_NSDocumentFramworkPrivate
#define _GNUstep_H_NSDocumentFramworkPrivate

#import "AppKit/NSDocument.h"
#import "AppKit/NSDocumentController.h"
#import "AppKit/NSWindowController.h"

@class NSTimer;

@interface NSDocumentController (Private)
- (NSArray *)_readableTypesForClass:(Class)documentClass;
- (NSArray *)_writableTypesForClass:(Class)documentClass;
- (NSString *)_autosaveDirectory: (BOOL)create;
- (void)_autosaveDocuments: (NSTimer *)timer;
- (BOOL)_reopenAutosavedDocuments;
- (void)_recordAutosavedDocument: (NSDocument *)document;
@end

@interface NSDocumentController (RecentDocumentsMenu)
- (NSMenu *) _recentDocumentsMenu;
- (void) _setRecentDocumentsMenu: (NSMenu *)menu;
- (void) _updateRecentDocumentsMenu;
- (IBAction) _openRecentDocument: (id)sender;
- (void) _saveRecentDocuments;
- (void) _loadRecentDocuments;
@end

@interface NSDocument (Private)
- (void)_removeWindowController:(NSWindowController *)controller;
- (NSWindow *)_transferWindowOwnership;
- (void)_removeAutosavedContentsFile;
@end

@interface NSWindowController (Private)
- (void)_windowDidLoad;
@end

#endif // _GNUstep_H_NSDocumentFramworkPrivate
