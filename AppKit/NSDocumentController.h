/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/Foundation.h>
#import <AppKit/NSUserInterfaceValidation.h>

@class NSDocument, NSOpenPanel, NSWindow, NSMenuItem;

@interface NSDocumentController : NSObject <NSCoding> {
    NSMutableArray *_documents;
    NSArray *_fileTypes;
    NSTimeInterval _autosavingDelay;
}

+ sharedDocumentController;

- init;

- (NSString *)defaultType;
- (NSArray *)documentClassNames;
- (NSTimeInterval)autosavingDelay;

- (NSString *)displayNameForType:(NSString *)type;
- (Class)documentClassForType:(NSString *)type;
- (NSArray *)fileExtensionsFromType:(NSString *)type;
- (NSString *)typeFromFileExtension:(NSString *)extension;

- (void)setAutosavingDelay:(NSTimeInterval)value;

- (NSArray *)documents;
- (void)addDocument:(NSDocument *)document;
- (void)removeDocument:(NSDocument *)document;
- documentForURL:(NSURL *)url;

- currentDocument;
- (NSString *)currentDirectory;
- (BOOL)hasEditedDocuments;
- documentForWindow:(NSWindow *)window;

- (NSString *)typeForContentsOfURL:(NSURL *)url error:(NSError **)error;

- makeDocumentWithContentsOfFile:(NSString *)path ofType:(NSString *)type;
- makeDocumentWithContentsOfURL:(NSURL *)url ofType:(NSString *)type error:(NSError **)error;
- makeDocumentForURL:(NSURL *)url withContentsOfURL:(NSURL *)contentsURL ofType:(NSString *)type error:(NSError **)error;
- makeUntitledDocumentOfType:(NSString *)type;

- openUntitledDocumentOfType:(NSString *)type display:(BOOL)display;
- openUntitledDocumentAndDisplay:(BOOL)display error:(NSError **)error;
- openDocumentWithContentsOfFile:(NSString *)path display:(BOOL)display;
- openDocumentWithContentsOfURL:(NSURL *)url display:(BOOL)display error:(NSError **)error;

- (BOOL)reopenDocumentForURL:(NSURL *)url withContentsOfURL:(NSURL *)contentsUL error:(NSError **)error;

- (void)closeAllDocumentsWithDelegate:delegate didCloseAllSelector:(SEL)selector contextInfo:(void *)info;

- (void)reviewUnsavedDocumentsWithAlertTitle:(NSString *)title cancellable:(BOOL)cancellable delegate:delegate didReviewAllSelector:(SEL)selector info:(void *)info;

- (NSError *)willPresentError:(NSError *)error;
- (BOOL)presentError:(NSError *)error;
- (void)presentError:(NSError *)error modalForWindow:(NSWindow *)window delegate:delegate didPresentSelector:(SEL)selector contextInfo:(void *)info;

- (int)runModalOpenPanel:(NSOpenPanel *)openPanel forTypes:(NSArray *)extensions;
- (NSArray *)fileNamesFromRunningOpenPanel;
- (NSArray *)URLsFromRunningOpenPanel;

- (NSArray *)recentDocumentURLs;
- (unsigned)maximumRecentDocumentCount;
- (void)noteNewRecentDocumentURL:(NSURL *)url;
- (void)noteNewRecentDocument:(NSDocument *)document;
- (void)clearRecentDocuments:sender;

- (void)newDocument:sender;
- (void)openDocument:sender;
- (void)saveAllDocuments:sender;

- (BOOL)validateMenuItem:(NSMenuItem *)item;
- (BOOL)validateUserInterfaceItem:(id<NSValidatedUserInterfaceItem>)item;

// deprecated
- (BOOL)closeAllDocuments;
- (BOOL)reviewUnsavedDocumentsWithAlertTitle:(NSString *)title cancellable:(BOOL)cancellable;

@end
