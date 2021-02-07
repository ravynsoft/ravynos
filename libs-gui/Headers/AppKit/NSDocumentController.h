/* 
   NSDocumentController.h

   The document controller class

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

#ifndef _GNUstep_H_NSDocumentController
#define _GNUstep_H_NSDocumentController
#import <GNUstepBase/GSVersionMacros.h>

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST) 

#import <Foundation/NSObject.h>
#import <AppKit/NSNibDeclarations.h>
#import <AppKit/NSUserInterfaceValidation.h>

@class NSArray;
@class NSError;
@class NSMutableArray;
@class NSURL;
@class NSString;

@class NSDocument;
@class NSMenu;
@class NSMenuItem;
@class NSOpenPanel;
@class NSWindow;

@interface NSDocumentController : NSObject <NSCoding>
{
  @private
    NSMutableArray 	*_documents;
    NSMutableArray 	*_recent_documents;
    NSArray		*_types;	// from info.plist with key NSTypes
    NSTimeInterval      _autosavingDelay;
    struct __controller_flags {
        unsigned int should_create_ui:1;
        unsigned int RESERVED:31;
    } _controller_flags;
    NSMenu		*_recent_documents_menu;
}

+ (id) sharedDocumentController;

/*" document creation "*/
// doesn't create the windowControllers
- (id) makeUntitledDocumentOfType: (NSString*)type;
- (id) makeDocumentWithContentsOfFile: (NSString*)fileName
			       ofType: (NSString*)type;
// creates window controllers
- (id) openUntitledDocumentOfType: (NSString*)type
			  display: (BOOL)display;
- (id) openDocumentWithContentsOfFile: (NSString*)fileName
			      display: (BOOL)display;

- (id) makeDocumentWithContentsOfURL: (NSURL*)url
			      ofType: (NSString*)type;
- (id) openDocumentWithContentsOfURL: (NSURL*)url
			     display: (BOOL)display;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST) 
- (id) makeDocumentForURL: (NSURL*)url
        withContentsOfURL: (NSURL*)contents
                   ofType: (NSString*)type
                    error: (NSError**)err;
- (id) makeDocumentWithContentsOfURL: (NSURL*)url 
			      ofType: (NSString*)type 
			       error: (NSError**)err;
- (id) makeUntitledDocumentOfType: (NSString*)type 
			    error: (NSError**)err;
- (id) openDocumentWithContentsOfURL: (NSURL*)url
			     display: (BOOL)flag
			       error: (NSError**)err;
- (id) openUntitledDocumentAndDisplay: (BOOL)flag 
			        error: (NSError**)err;
- (BOOL) reopenDocumentForURL: (NSURL*)url
	    withContentsOfURL: (NSURL*)contents
		        error: (NSError**)err;
- (BOOL) presentError: (NSError*)err;
- (void) presentError: (NSError*)err
       modalForWindow: (NSWindow*)win
	     delegate: (id)delegate 
   didPresentSelector: (SEL)sel
	  contextInfo: (void*)context;
- (NSError*) willPresentError: (NSError*)err;
#endif 
#if OS_API_VERSION(MAC_OS_X_VERSION_10_7, GS_API_LATEST) 
DEFINE_BLOCK_TYPE(GSCompletionBlock1, void, NSDocument*, BOOL, NSError*);
DEFINE_BLOCK_TYPE(GSCompletionBlock2, void, NSArray*);

- (id) duplicateDocumentWithContentsOfURL: (NSURL*)url
                                  copying: (BOOL)duplicateByCopying
                              displayName: (NSString*)displayNameOrNil
                                    error: (NSError**)outError;
- (void) openDocumentWithContentsOfURL: (NSURL*)url
                               display: (BOOL)displayDocument
                     completionHandler: (GSCompletionBlock1)completionHandler;
- (void) reopenDocumentForURL: (NSURL*)urlOrNil
            withContentsOfURL: (NSURL*)contentsURL
                      display: (BOOL)displayDocument
            completionHandler: (GSCompletionBlock1)completionHandler;
- (void) beginOpenPanelWithCompletionHandler: (GSCompletionBlock2)completionHandler;

#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_8, GS_API_LATEST) 
DEFINE_BLOCK_TYPE(GSCompletionBlock3, void, NSInteger);

- (void) beginOpenPanel: (NSOpenPanel*)openPanel
               forTypes: (NSArray*)inTypes
      completionHandler: (GSCompletionBlock3)completionHandler;
#endif

/*" With or without UI "*/
- (BOOL) shouldCreateUI;
- (void) setShouldCreateUI: (BOOL)flag;

/*" Actions "*/
- (IBAction) saveAllDocuments: (id)sender;
- (IBAction) openDocument: (id)sender;
- (IBAction) newDocument: (id)sender;
- (IBAction) clearRecentDocuments: (id)sender;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST) 
- (NSUInteger) maximumRecentDocumentCount;
#endif

/*" Recent Documents "*/
- (void) noteNewRecentDocument: (NSDocument*)aDocument;
- (void) noteNewRecentDocumentURL: (NSURL*)anURL;
- (NSArray*) recentDocumentURLs;

/*" Open panel "*/
- (NSArray*) URLsFromRunningOpenPanel;
- (NSArray*) fileNamesFromRunningOpenPanel;
- (NSInteger) runModalOpenPanel: (NSOpenPanel*)openPanel
		       forTypes: (NSArray*)openableFileExtensions;

/*" Document management "*/
- (void) addDocument: (NSDocument*)document;
- (void) removeDocument: (NSDocument*)document;
- (BOOL) closeAllDocuments;
- (void) closeAllDocumentsWithDelegate: (id)delegate 
		   didCloseAllSelector: (SEL)didAllCloseSelector 
			   contextInfo: (void*)contextInfo;
- (BOOL) reviewUnsavedDocumentsWithAlertTitle: (NSString*)title
				  cancellable: (BOOL)cancellable;
- (void) reviewUnsavedDocumentsWithAlertTitle: (NSString*)title 
				  cancellable: (BOOL)cancellable 
				     delegate: (id)delegate
			 didReviewAllSelector: (SEL)didReviewAllSelector 
				  contextInfo: (void*)contextInfo;
- (NSArray*) documents;
- (BOOL) hasEditedDocuments;
- (id) currentDocument;
- (NSString*) currentDirectory;
- (id) documentForWindow: (NSWindow*)window;
- (id) documentForFileName: (NSString*)fileName;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST) 
- (id) documentForURL: (NSURL*)url;
#endif 


/*" Menu validation "*/
- (BOOL) validateMenuItem: (NSMenuItem*)anItem;
- (BOOL) validateUserInterfaceItem: (id <NSValidatedUserInterfaceItem>)anItem;

/*" Types and extensions "*/
- (NSString*) displayNameForType: (NSString*)type;
- (NSString*) typeFromFileExtension: (NSString*)fileExtension;
- (NSArray*) fileExtensionsFromType: (NSString*)type;
- (Class) documentClassForType: (NSString*)type;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST) 
/** Returns the first type found for which the application has an editor
 * role.
 */
- (NSString*) defaultType;

/** Returns the names of the NSDocument subclasses handling documents 
 * in this application.  This will be nil or empty if this is not a document
 * based application.
 */
- (NSArray*) documentClassNames;

- (NSString*) typeForContentsOfURL: (NSURL*)url
			     error: (NSError**)err;
#endif 

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST) 
/* Autosaving */
- (NSTimeInterval) autosavingDelay;
- (void) setAutosavingDelay: (NSTimeInterval)autosavingDelay;
#endif 

@end

#endif // GS_API_MACOSX

#endif // _GNUstep_H_NSDocumentController
