/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/Foundation.h>
#import <AppKit/NSUserInterfaceValidation.h>

@class NSWindow, NSWindowController, NSSavePanel, NSMenuItem, NSFileWrapper, NSPrintOperation, NSPrintInfo, NSPageLayout, NSView;

typedef enum {
    NSChangeDone,
    NSChangeUndone,
    NSChangeCleared,
    NSChangeReadOtherContents,
    NSChangeAutosaved,
} NSDocumentChangeType;

typedef enum {
    NSSaveOperation,
    NSSaveAsOperation,
    NSSaveToOperation,
    NSAutosaveOperation,
} NSSaveOperationType;

@interface NSDocument : NSObject {
    NSMutableArray *_windowControllers;
    NSURL *_fileURL;
    NSString *_fileType;
    NSDate *_fileModificationDate;
    NSString *_lastComponentOfFileName;
    NSURL *_autosavedContentsFileURL;
    NSPrintInfo *_printInfo;
    int _changeCount;
    unsigned _untitledNumber;
    NSUndoManager *_undoManager;
    BOOL _hasUndoManager;
    NSMutableArray *_activeEditors; // registered via NSEditorRegistration
}

+ (NSArray *)readableTypes;
+ (NSArray *)writableTypes;

+ (BOOL)isNativeType:(NSString *)type;

- init;
- initWithType:(NSString *)type error:(NSError **)error;
- initWithContentsOfURL:(NSURL *)url ofType:(NSString *)type error:(NSError **)error;
- initForURL:(NSURL *)url withContentsOfURL:(NSURL *)contentsURL ofType:(NSString *)type error:(NSError **)error;

- (NSURL *)autosavedContentsFileURL;
- (NSDate *)fileModificationDate;
- (NSURL *)fileURL;
- (NSPrintInfo *)printInfo;
- (NSString *)fileType;
- (BOOL)hasUndoManager;
- (NSUndoManager *)undoManager;

- (void)setAutosavedContentsFileURL:(NSURL *)url;
- (void)setFileModificationDate:(NSDate *)value;
- (void)setFileURL:(NSURL *)url;
- (void)setPrintInfo:(NSPrintInfo *)value;
- (void)setFileType:(NSString *)value;
- (void)setHasUndoManager:(BOOL)value;
- (void)setUndoManager:(NSUndoManager *)value;

- (BOOL)hasUnautosavedChanges;
- (NSString *)autosavingFileType;

- (void)setLastComponentOfFileName:(NSString *)name;

- (NSString *)windowNibName;
- (void)setWindow:(NSWindow *)window;
- (void)windowControllerDidLoadNib:(NSWindowController *)controller;
- (void)windowControllerWillLoadNib:(NSWindowController *)controller;

- (void)showWindows;
- (void)makeWindowControllers;
- (NSArray *)windowControllers;
- (void)addWindowController:(NSWindowController *)controller;
- (void)removeWindowController:(NSWindowController *)controller;

- (NSString *)displayName;
- (NSWindow *)windowForSheet;

- (BOOL)isDocumentEdited;
- (void)updateChangeCount:(NSDocumentChangeType)changeType;

- (BOOL)readFromData:(NSData *)data ofType:(NSString *)type error:(NSError **)error;
- (BOOL)readFromFileWrapper:(NSFileWrapper *)fileWrapper ofType:(NSString *)type error:(NSError **)error;
- (BOOL)readFromURL:(NSURL *)url ofType:(NSString *)type error:(NSError **)error;
- (BOOL)revertToContentsOfURL:(NSURL *)ul ofType:(NSString *)type error:(NSError **)error;

- (NSData *)dataOfType:(NSString *)type error:(NSError **)error;
- (NSFileWrapper *)fileWrapperOfType:(NSString *)type error:(NSError **)error;

- (BOOL)writeToURL:(NSURL *)url ofType:(NSString *)type error:(NSError **)error;
- (BOOL)writeToURL:(NSURL *)url ofType:(NSString *)type forSaveOperation:(NSSaveOperationType)operation originalContentsURL:(NSURL *)contentsURL error:(NSError **)error;
- (BOOL)writeSafelyToURL:(NSURL *)url ofType:(NSString *)type forSaveOperation:(NSSaveOperationType)operation error:(NSError **)error;
- (NSDictionary *)fileAttributesToWriteToURL:(NSURL *)url ofType:(NSString *)type forSaveOperation:(NSSaveOperationType)operation originalContentsURL:(NSURL *)contentsURL error:(NSError **)error;

- (BOOL)keepBackupFile;

- (void)autosaveDocumentWithDelegate:delegate didAutosaveSelector:(SEL)selector contextInfo:(void *)info;

- (NSError *)willPresentError:(NSError *)error;
- (BOOL)presentError:(NSError *)error;
- (void)presentError:(NSError *)error modalForWindow:(NSWindow *)window delegate:delegate didPresentSelector:(SEL)selector contextInfo:(void *)info;

- (NSArray *)writableTypesForSaveOperation:(NSSaveOperationType)operation;
- (BOOL)shouldRunSavePanelWithAccessoryView;
- (BOOL)prepareSavePanel:(NSSavePanel *)panel;
- (BOOL)fileNameExtensionWasHiddenInLastRunSavePanel;
- (NSString *)fileTypeFromLastRunSavePanel;
- (void)runModalSavePanelForSaveOperation:(NSSaveOperationType)operation delegate:delegate didSaveSelector:(SEL)selector contextInfo:(void *)info;
- (void)saveDocumentWithDelegate:delegate didSaveSelector:(SEL)selector contextInfo:(void *)info;
- (BOOL)saveToURL:(NSURL *)url ofType:(NSString *)type forSaveOperation:(NSSaveOperationType)operation error:(NSError **)error;
- (void)saveToURL:(NSURL *)url ofType:(NSString *)type forSaveOperation:(NSSaveOperationType)operation delegate:delegate didSaveSelector:(SEL)selector contextInfo:(void *)info;

- (BOOL)preparePageLayout:(NSPageLayout *)pageLayout;
- (BOOL)shouldChangePrintInfo:(NSPrintInfo *)printInfo;
- (void)runModalPageLayoutWithPrintInfo:(NSPrintInfo *)printInfo delegate:delegate didRunSelector:(SEL)selector contextInfo:(void *)info;
- (void)runModalPrintOperation:(NSPrintOperation *)printOperation delegate:delegate didRunSelector:(SEL)selector contextInfo:(void *)info;
- (NSPrintOperation *)printOperationWithSettings:(NSDictionary *)settings error:(NSError **)error;
- (void)printDocumentWithSettings:(NSDictionary *)settings showPrintPanel:(BOOL)showPanel delegate:delegate didPrintSelector:(SEL)selector contextInfo:(void *)info;

- (void)close;
- (void)canCloseDocumentWithDelegate:delegate shouldCloseSelector:(SEL)selector contextInfo:(void *)info;
- (void)shouldCloseWindowController:(NSWindowController *)controller delegate:delegate shouldCloseSelector:(SEL)selector contextInfo:(void *)info;

- (void)revertDocumentToSaved:sender;
- (void)saveDocument:sender;
- (void)saveDocumentAs:sender;
- (void)saveDocumentTo:sender;
- (void)printDocument:sender;
- (void)runPageLayout:sender;

- (BOOL)validateUserInterfaceItem:(id<NSValidatedUserInterfaceItem>)item;

- (void)objectDidBeginEditing:editor;
- (void)objectDidEndEditing:editor;

// deprecated

- (BOOL)validateMenuItem:(NSMenuItem *)item;
- (BOOL)canCloseDocument;
- (NSData *)dataRepresentationOfType:(NSString *)type;
- (NSDictionary *)fileAttributesToWriteToFile:(NSString *)path ofType:(NSString *)type saveOperation:(NSSaveOperationType)operation;
- (NSString *)fileName;
- (NSString *)fileNameFromRunningSavePanelForSaveOperation:(NSSaveOperationType)operation;
- (NSFileWrapper *)fileWrapperRepresentationOfType:(NSString *)type;
- initWithContentsOfFile:(NSString *)path ofType:(NSString *)type;
- initWithContentsOfURL:(NSURL *)url ofType:(NSString *)type;
- (BOOL)loadDataRepresentation:(NSData *)data ofType:(NSString *)type;
- (BOOL)loadFileWrapperRepresentation:(NSFileWrapper *)wrapper ofType:(NSString *)type;
- (void)printShowingPrintPanel:(BOOL)flag;
- (BOOL)readFromFile:(NSString *)path ofType:(NSString *)type;
- (BOOL)readFromURL:(NSURL *)url ofType:(NSString *)type;
- (BOOL)revertToSavedFromFile:(NSString *)path ofType:(NSString *)type;
- (BOOL)revertToSavedFromURL:(NSURL *)url ofType:(NSString *)type;
- (int)runModalSavePanel:(NSSavePanel *)savePanel withAccessoryView:(NSView *)accessoryView;
- (int)runModalPageLayoutWithPrintInfo:(NSPrintInfo *)printInfo;
- (void)setFileName:(NSString *)path;
- (void)saveToFile:(NSString *)path saveOperation:(NSSaveOperationType)operation delegate:delegate didSaveSelector:(SEL)selector contextInfo:(void *)info;
- (BOOL)shouldCloseWindowController:(NSWindowController *)controller;
- (BOOL)writeToFile:(NSString *)path ofType:(NSString *)type;
- (BOOL)writeToFile:(NSString *)path ofType:(NSString *)type originalFile:(NSString *)original saveOperation:(NSSaveOperationType)operation;
- (BOOL)writeToURL:(NSURL *)url ofType:(NSString *)type;
- (BOOL)writeWithBackupToFile:(NSString *)path ofType:(NSString *)type saveOperation:(NSSaveOperationType)operation;

@end
