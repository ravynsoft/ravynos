/* 
   NSDocument.h

   The abstract document class

   Copyright (C) 1999 Free Software Foundation, Inc.

   Author: Carl Lindberg <Carl.Lindberg@hbo.com>
   Date: 1999
   Modifications: Fred Kiefer <fredkiefer@gmx.de>
   Date: Dec 2006
   Added MacOS 10.4 methods.
   
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

#ifndef _GNUstep_H_NSDocument
#define _GNUstep_H_NSDocument
#import <GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSObject.h>
#import <AppKit/NSNibDeclarations.h>
#import <AppKit/NSUserInterfaceValidation.h>


/* Foundation classes */
@class NSString;
@class NSArray;
@class NSMutableArray;
@class NSData;
@class NSDate;
@class NSDictionary;
@class NSError;
@class NSFileManager;
@class NSURL;
@class NSUndoManager;

/* AppKit classes */
@class NSWindow;
@class NSView;
@class NSSavePanel;
@class NSMenuItem;
@class NSPageLayout;
@class NSPrintInfo;
@class NSPrintOperation;
@class NSPopUpButton;
@class NSFileWrapper;
@class NSDocumentController;
@class NSWindowController;


typedef enum _NSDocumentChangeType {
    NSChangeDone 	= 0,
    NSChangeUndone 	= 1,
    NSChangeCleared 	= 2,
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
    NSChangeReadOtherContents = 3,
    NSChangeAutosaved   = 4
#endif
} NSDocumentChangeType;

typedef enum _NSSaveOperationType {
    NSSaveOperation		= 0,
    NSSaveAsOperation		= 1,
    NSSaveToOperation		= 2,
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
    NSAutosaveOperation		= 3
#endif
} NSSaveOperationType;

@interface NSDocument : NSObject
{
  @private
    NSWindow		*_window;		// Outlet for the single window case
    NSMutableArray 	*_window_controllers;	// WindowControllers for this document
    NSURL		*_file_url;		// Save location as URL
    NSString		*_file_name;		// Save location
    NSString 		*_file_type;		// file/document type
    NSDate 		*_file_modification_date;// file modification date
    NSString		*_last_component_file_name; // file name last component
    NSURL		*_autosaved_file_url;	// Autosave location as URL
    NSPrintInfo 	*_print_info;		// print info record
    id			_printOp_delegate;	// delegate and selector called
    SEL			_printOp_didRunSelector;//   after modal print operation
    NSView 		*_save_panel_accessory;	// outlet for the accessory save-panel view
    NSPopUpButton	*_spa_button;     	// outlet for "the File Format:" button in the save panel.
    NSString            *_save_type;             // the currently selected extension.
    NSUndoManager 	*_undo_manager;		// Undo manager for this document
    long		_change_count;		// number of time the document has been changed
    long		_autosave_change_count;	// number of time the document has been changed since the last autosave
    int			_document_index;	// Untitled index
    struct __docFlags {
        unsigned int in_close:1;
        unsigned int has_undo_manager:1;
        unsigned int permanently_modified:1;
        unsigned int autosave_permanently_modified:1;
        unsigned int RESERVED:28;
    } _doc_flags;
    void 		*_reserved1;
}

+ (NSArray *)readableTypes;
+ (NSArray *)writableTypes;
+ (BOOL)isNativeType:(NSString *)type;

/*" Initialization "*/
- (id)init;
- (id)initWithContentsOfFile:(NSString *)fileName ofType:(NSString *)fileType;
- (id)initWithContentsOfURL:(NSURL *)url ofType:(NSString *)fileType;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (id)initForURL:(NSURL *)forUrl
withContentsOfURL:(NSURL *)url
          ofType:(NSString *)type
           error:(NSError **)error;
- (id)initWithContentsOfURL:(NSURL *)url
                     ofType:(NSString *)type
                      error:(NSError **)error;
- (id)initWithType:(NSString *)type
             error:(NSError **)error;
#endif

/*" Window management "*/
- (NSArray *)windowControllers;
- (void)addWindowController:(NSWindowController *)windowController;
#if OS_API_VERSION(GS_API_MACOSX, MAC_OS_X_VERSION_10_4)
- (BOOL)shouldCloseWindowController:(NSWindowController *)windowController;
#endif
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (void)shouldCloseWindowController:(NSWindowController *)windowController 
			   delegate:(id)delegate 
		shouldCloseSelector:(SEL)callback
			contextInfo:(void *)contextInfo;
#endif
- (void)showWindows;
- (void)removeWindowController:(NSWindowController *)windowController;
- (void)setWindow:(NSWindow *)aWindow;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
- (NSWindow *)windowForSheet;
#endif

/*" Window controller creation "*/
- (void)makeWindowControllers;  // Manual creation
- (NSString *)windowNibName;    // Automatic creation (Document will be the nib owner)

/*" Window loading notifications "*/
// Only called if the document is the owner of the nib
- (void)windowControllerWillLoadNib:(NSWindowController *)windowController;
- (void)windowControllerDidLoadNib:(NSWindowController *)windowController;

/*" Edited flag "*/
- (BOOL)isDocumentEdited;
- (void)updateChangeCount:(NSDocumentChangeType)change;

/*" Display Name (window title) "*/
- (NSString *)displayName;

/*" Backup file "*/
- (BOOL)keepBackupFile;

/*" Closing "*/
- (void)close;
#if OS_API_VERSION(GS_API_MACOSX, MAC_OS_X_VERSION_10_4)
- (BOOL)canCloseDocument;
#endif
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (void)canCloseDocumentWithDelegate:(id)delegate 
		 shouldCloseSelector:(SEL)shouldCloseSelector 
			 contextInfo:(void *)contextInfo;
#endif

/*" Type and location "*/
- (NSString *)fileName;
- (void)setFileName:(NSString *)fileName;
- (NSString *)fileType;
- (void)setFileType:(NSString *)type;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (NSURL *)fileURL;
- (void)setFileURL:(NSURL *)url;
- (NSDate *)fileModificationDate;
- (void)setFileModificationDate: (NSDate *)date;
- (NSString *)lastComponentOfFileName;
- (void)setLastComponentOfFileName:(NSString *)str;
#endif

/*" Read/Write/Revert "*/

- (NSData *)dataRepresentationOfType:(NSString *)type;
- (BOOL)loadDataRepresentation:(NSData *)data ofType:(NSString *)type;

- (NSFileWrapper *)fileWrapperRepresentationOfType:(NSString *)type;
- (BOOL)loadFileWrapperRepresentation:(NSFileWrapper *)wrapper 
			       ofType:(NSString *)type;

- (BOOL)writeToFile:(NSString *)fileName ofType:(NSString *)type;
- (BOOL)readFromFile:(NSString *)fileName ofType:(NSString *)type;
- (BOOL)revertToSavedFromFile:(NSString *)fileName ofType:(NSString *)type;

- (BOOL)writeToURL:(NSURL *)url ofType:(NSString *)type;
- (BOOL)readFromURL:(NSURL *)url ofType:(NSString *)type;
- (BOOL)revertToSavedFromURL:(NSURL *)url ofType:(NSString *)type;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (NSData *)dataOfType:(NSString *)type
                 error:(NSError **)error;
- (NSFileWrapper *)fileWrapperOfType:(NSString *)type
                               error:(NSError **)error;
- (BOOL)readFromData:(NSData *)data
              ofType:(NSString *)type
               error:(NSError **)error;
- (BOOL)readFromFileWrapper:(NSFileWrapper *)wrapper
                     ofType:(NSString *)type
                      error:(NSError **)error;
- (BOOL)readFromURL:(NSURL *)url
             ofType:(NSString *)type
              error:(NSError **)error;
- (BOOL)revertToContentsOfURL:(NSURL *)url
                       ofType:(NSString *)type
                        error:(NSError **)error;
- (BOOL)writeSafelyToURL:(NSURL *)url
                  ofType:(NSString *)type
        forSaveOperation:(NSSaveOperationType)op
                   error:(NSError **)error;
- (BOOL)writeToURL:(NSURL *)url 
            ofType:(NSString *)type 
             error:(NSError **)error;
- (BOOL)writeToURL:(NSURL *)url
            ofType:(NSString *)type
  forSaveOperation:(NSSaveOperationType)op
originalContentsURL:(NSURL *)orig
             error:(NSError **)error;
#endif

/*" Save panel "*/
- (BOOL)shouldRunSavePanelWithAccessoryView;
#if OS_API_VERSION(GS_API_MACOSX, MAC_OS_X_VERSION_10_4)
- (NSString *)fileNameFromRunningSavePanelForSaveOperation:(NSSaveOperationType)saveOperation;
- (NSInteger)runModalSavePanel:(NSSavePanel *)savePanel withAccessoryView:(NSView *)accessoryView;
#endif 
- (NSString *)fileTypeFromLastRunSavePanel;
- (NSDictionary *)fileAttributesToWriteToFile: (NSString *)fullDocumentPath 
				       ofType: (NSString *)docType 
				saveOperation: (NSSaveOperationType)saveOperationType;
- (BOOL)writeToFile:(NSString *)fileName 
	     ofType:(NSString *)type 
       originalFile:(NSString *)origFileName
      saveOperation:(NSSaveOperationType)saveOp;
- (BOOL)writeWithBackupToFile:(NSString *)fileName 
		       ofType:(NSString *)fileType 
		saveOperation:(NSSaveOperationType)saveOp;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (NSArray *)writableTypesForSaveOperation:(NSSaveOperationType)op;
- (NSDictionary *)fileAttributesToWriteToURL:(NSURL *)url
                                      ofType:(NSString *)type
                            forSaveOperation:(NSSaveOperationType)op
                         originalContentsURL:(NSURL *)original
                                       error:(NSError **)error;
#endif 
#if OS_API_VERSION(MAC_OS_X_VERSION_10_1, GS_API_LATEST)
- (BOOL)fileNameExtensionWasHiddenInLastRunSavePanel;
#endif 
#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
- (NSString *)fileNameExtensionForType:(NSString *)typeName
                         saveOperation:(NSSaveOperationType)saveOperation;
#endif 

/*" Printing "*/
- (NSPrintInfo *)printInfo;
- (void)setPrintInfo:(NSPrintInfo *)printInfo;
- (BOOL)shouldChangePrintInfo:(NSPrintInfo *)newPrintInfo;
- (IBAction)runPageLayout:(id)sender;
- (NSInteger)runModalPageLayoutWithPrintInfo:(NSPrintInfo *)printInfo;
- (IBAction)printDocument:(id)sender;
- (void)printShowingPrintPanel:(BOOL)flag;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (BOOL)preparePageLayout:(NSPageLayout *)pageLayout;
- (void)runModalPageLayoutWithPrintInfo:(NSPrintInfo *)info
                               delegate:(id)delegate
                         didRunSelector:(SEL)sel
                            contextInfo:(void *)context;
- (void)printDocumentWithSettings:(NSDictionary *)settings
                   showPrintPanel:(BOOL)flag
                         delegate:(id)delegate
                 didPrintSelector:(SEL)sel
                      contextInfo:(void *)context;
- (NSPrintOperation *)printOperationWithSettings:(NSDictionary *)settings
                                           error:(NSError **)error;
- (void)runModalPrintOperation:(NSPrintOperation *)op
                      delegate:(id)delegate
                didRunSelector:(SEL)sel
                   contextInfo:(void *)context;
#endif

/*" IB Actions "*/
- (IBAction)saveDocument:(id)sender;
- (IBAction)saveDocumentAs:(id)sender;
- (IBAction)saveDocumentTo:(id)sender;
- (IBAction)revertDocumentToSaved:(id)sender;

/*" Menus "*/
- (BOOL)validateMenuItem:(NSMenuItem *)anItem;
- (BOOL)validateUserInterfaceItem:(id <NSValidatedUserInterfaceItem>)anItem;

/*" Undo "*/
- (NSUndoManager *)undoManager;
- (void)setUndoManager:(NSUndoManager *)undoManager;
- (BOOL)hasUndoManager;
- (void)setHasUndoManager:(BOOL)flag;

/* NEW delegate operations*/
- (void)saveToFile:(NSString *)fileName 
     saveOperation:(NSSaveOperationType)saveOperation 
	  delegate:(id)delegate
   didSaveSelector:(SEL)didSaveSelector 
       contextInfo:(void *)contextInfo;
- (BOOL)prepareSavePanel:(NSSavePanel *)savePanel;
- (void)saveDocumentWithDelegate:(id)delegate 
		 didSaveSelector:(SEL)didSaveSelector 
		     contextInfo:(void *)contextInfo;
- (void)runModalSavePanelForSaveOperation:(NSSaveOperationType)saveOperation 
				 delegate:(id)delegate
			  didSaveSelector:(SEL)didSaveSelector 
			      contextInfo:(void *)contextInfo;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (BOOL)saveToURL:(NSURL *)url
           ofType:(NSString *)type
 forSaveOperation:(NSSaveOperationType)op
            error:(NSError **)error;
- (void)saveToURL:(NSURL *)url
           ofType:(NSString *)type
 forSaveOperation:(NSSaveOperationType)op
         delegate:(id)delegate
  didSaveSelector:(SEL)didSaveSelector
      contextInfo:(void *)contextInfo;
 
/* Autosaving */
- (NSURL *)autosavedContentsFileURL;
- (void)setAutosavedContentsFileURL:(NSURL *)url;
- (void)autosaveDocumentWithDelegate:(id)delegate
                 didAutosaveSelector:(SEL)didAutosaveSelector
                         contextInfo:(void *)context;
- (NSString *)autosavingFileType;
- (BOOL)hasUnautosavedChanges;


- (BOOL)presentError:(NSError *)error;
- (void)presentError:(NSError *)error
      modalForWindow:(NSWindow *)window
            delegate:(id)delegate
  didPresentSelector:(SEL)sel
         contextInfo:(void *)context;
- (NSError *)willPresentError:(NSError *)error;
#endif
@end

#endif // _GNUstep_H_NSDocument
