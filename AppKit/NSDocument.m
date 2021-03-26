/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSDocument.h>
#import <AppKit/NSDocumentController.h>
#import <AppKit/NSWindowController.h>
#import <AppKit/NSSavePanel.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSMenuItem.h>
#import <AppKit/NSFileWrapper.h>
#import <AppKit/NSPrintOperation.h>
#import <AppKit/NSPageLayout.h>
#import <AppKit/NSPrintInfo.h>
#import <AppKit/NSRaise.h>
#import "NSKeyValueBinding/NSObject+BindingSupport.h"

@implementation NSDocument

static int untitled_document_number = 0;

+(NSArray *)readableTypes {
   int             i;
   NSArray        *knownDocTypes = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleDocumentTypes"];
   NSMutableArray *readableTypes = [NSMutableArray array];
   NSDictionary   *typeDict;
   NSString       *typeName, *typeRole;
   
   for (i = 0; i < [knownDocTypes count]; i++)
   {
      typeDict = [knownDocTypes objectAtIndex:i];
      typeRole = [typeDict objectForKey:@"CFBundleTypeRole"];
      if (NSClassFromString((NSString *)[typeDict objectForKey:@"NSDocumentClass"]) == self &&
          ([typeRole isEqualToString:@"Viewer"] || [typeRole isEqualToString:@"Editor"]))
      {
         typeName = [typeDict objectForKey:@"CFBundleTypeName"];
         if (typeName)
            [readableTypes addObject:typeName];
      }
   }
   
   return [NSArray arrayWithArray:readableTypes];
}

+(NSArray *)writableTypes {
   int             i;
   NSArray        *knownDocTypes = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleDocumentTypes"];
   NSMutableArray *writableTypes = [NSMutableArray array];
   NSDictionary   *typeDict;
   NSString       *typeName;
   
   for (i = 0; i < [knownDocTypes count]; i++)
   {
      typeDict  = [knownDocTypes objectAtIndex:i];
      if (NSClassFromString((NSString *)[typeDict objectForKey:@"NSDocumentClass"]) == self &&
          [(NSString *)[typeDict objectForKey:@"CFBundleTypeRole"] isEqualToString:@"Editor"])
      {
         typeName = [typeDict objectForKey:@"CFBundleTypeName"];
         if (typeName)
            [writableTypes addObject:typeName];
      }
   }
   
   return [NSArray arrayWithArray:writableTypes];
}

+(BOOL)isNativeType:(NSString *)type {
   BOOL          result = NO;
   int           i;
   NSArray      *knownDocTypes = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleDocumentTypes"];
   NSDictionary *typeDict;
   
   for (i = 0; i < [knownDocTypes count]; i++)
   {
      typeDict = [knownDocTypes objectAtIndex:i];
      result  |= NSClassFromString((NSString *)[typeDict objectForKey:@"NSDocumentClass"]) == self &&
                 [(NSString *)[typeDict objectForKey:@"CFBundleTypeRole"] isEqualToString:@"Editor"] &&
                 [(NSString *)[typeDict objectForKey:@"CFBundleTypeName"] isEqualToString:type];
   }
   
   return result;
}

-(BOOL)_isSelectorOverridden:(SEL)selector {
   IMP mine=[NSDocument instanceMethodForSelector:selector];
   IMP theirs=[self methodForSelector:selector];
   
   return (mine!=theirs)?YES:NO;
}

-init {
  self = [super init];
  if (self)
    {
      _windowControllers=[NSMutableArray new];
      _fileURL=nil;
      _fileType=nil;
      _changeCount=0;
      _untitledNumber=untitled_document_number++;
      _hasUndoManager=YES;
      _activeEditors=[NSMutableArray new];
    }
  return self;
}

-initWithType:(NSString *)type error:(NSError **)error {
   [self init];
   [self setFileType:type];
   return self;
}

-(void)_updateFileModificationDate
{
  NSFileManager * fileManager = [NSFileManager defaultManager];
  NSString * path = [_fileURL path];
  NSDictionary * attributes = [fileManager fileAttributesAtPath:path traverseLink:YES];
  [self setFileModificationDate:[attributes objectForKey:NSFileModificationDate]];
}

-initWithContentsOfURL:(NSURL *)url ofType:(NSString *)type error:(NSError **)error {
   if([self _isSelectorOverridden:@selector(initWithContentsOfFile:ofType:)]){
    if([self initWithContentsOfFile:[url path] ofType:type]==nil)
     return nil;
   }
   else {
    [self init];
	   [self setFileURL:url];
	   [self setFileType:type];
    if(![self readFromURL:url ofType:type error:error]){
     [self dealloc];
     return nil;
    }
   }
   [self _updateFileModificationDate];
   return self;
}

-initForURL:(NSURL *)url withContentsOfURL:(NSURL *)contentsURL ofType:(NSString *)type error:(NSError **)error {
   [self init];
	[self setFileURL:url];
	[self setFileType:type];
   if(contentsURL!=nil){
    if(![self readFromURL:contentsURL ofType:type error:error]){
     [self dealloc];
     return nil;
    }
   }
   [self _updateFileModificationDate];
   return self;
}

-(void)dealloc
{
  [[NSNotificationCenter defaultCenter] removeObserver:self];
    
  [_windowControllers release];
  [_fileURL release];
  [_fileType release];
  [_fileModificationDate release];
  [_lastComponentOfFileName release];
  [_autosavedContentsFileURL release];
  [_printInfo release];
  [_undoManager release];
  [_activeEditors release];
  
  [super dealloc];
}

-(NSURL *)autosavedContentsFileURL {
   return _autosavedContentsFileURL;
}

-(NSDate *)fileModificationDate {
   return _fileModificationDate;
}

-(NSURL *)fileURL {
   return _fileURL;
}

-(NSPrintInfo *)printInfo {
    return _printInfo?_printInfo:[NSPrintInfo sharedPrintInfo];
}

-(NSString *)fileType {
   return _fileType;
}

-(BOOL)hasUndoManager {
    return _hasUndoManager;
}


-(NSUndoManager *)undoManager {
    if (_undoManager == nil && _hasUndoManager == YES) {
        [self setUndoManager:[NSUndoManager new]];
        [_undoManager beginUndoGrouping];
    }

    return _undoManager;
}

-(void)setAutosavedContentsFileURL:(NSURL *)url {
   url=[url copy];
   [_autosavedContentsFileURL release];
   _autosavedContentsFileURL=url;
}

-(void)setFileModificationDate:(NSDate *)value {
   value=[value copy];
   [_fileModificationDate release];
   _fileModificationDate=value;
}

-(void)setFileURL:(NSURL *)url {
   url=[url copy];
   [_fileURL release];
   _fileURL=url;
   [_windowControllers makeObjectsPerformSelector:@selector(synchronizeWindowTitleWithDocumentName)];
}

-(void)setPrintInfo:(NSPrintInfo *)value {
	value=[value copy];
   [_printInfo release];
   _printInfo=value;
}

-(void)setFileType:(NSString *)type {
   type=[type copy];
   [_fileType release];
   _fileType=type;
}

-(void)setHasUndoManager:(BOOL)flag {
    _hasUndoManager = flag;
    if (flag == YES && _undoManager == nil)
        [self undoManager];
    else if (flag == NO && _undoManager != nil)
        [self setUndoManager:nil];
}

-(void)setUndoManager:(NSUndoManager *)undoManager {
    if (_undoManager != nil) {
        [[NSNotificationCenter defaultCenter] removeObserver:self
                                                        name:NSUndoManagerDidUndoChangeNotification
                                                      object:_undoManager];
        [[NSNotificationCenter defaultCenter] removeObserver:self
                                                        name:NSUndoManagerDidRedoChangeNotification
                                                      object:_undoManager];
        [[NSNotificationCenter defaultCenter] removeObserver:self
                                                        name:NSUndoManagerWillCloseUndoGroupNotification
                                                      object:_undoManager];
        [_undoManager release];
    }
    
    _undoManager = [undoManager retain];

    if (undoManager) {
        // Only add observers if we have an undoManager we're concerned about
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(_undoManagerDidUndoChange:)
                                                     name:NSUndoManagerDidUndoChangeNotification
                                                   object:_undoManager];
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(_undoManagerDidRedoChange:)
                                                     name:NSUndoManagerDidRedoChangeNotification
                                                   object:_undoManager];
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(_undoManagerDidCloseGroup:)
                                                     name:NSUndoManagerWillCloseUndoGroupNotification
                                                   object:_undoManager];
    }
}

-(NSUndoManager *)windowWillReturnUndoManager:(NSWindow *)window {
    if (_hasUndoManager)
        return [self undoManager];

    return nil;
}

-(BOOL)hasUnautosavedChanges {
   NSUnimplementedMethod();
   return 0;
}

-(NSString *)autosavingFileType {
   return [self fileType];
}

-(void)setLastComponentOfFileName:(NSString *)name {
   name=[name copy];
   [_lastComponentOfFileName release];
   _lastComponentOfFileName=name;
}

-(NSString *)windowNibName {
   return nil;
}

-(void)setWindow:(NSWindow *)window {
   [[_windowControllers objectAtIndex:0] setWindow:window];
}

-(void)windowControllerDidLoadNib:(NSWindowController *)controller {
   // do nothing
}

-(void)windowControllerWillLoadNib:(NSWindowController *)controller {
   // do nothing
}

-(void)showWindows {
   [_windowControllers makeObjectsPerformSelector:@selector(showWindow:) withObject:self];
}

-(void)makeWindowControllers {
   NSString *nibName=[self windowNibName];

   if(nibName!=nil){
    NSWindowController *controller=[[[NSWindowController alloc] initWithWindowNibName:nibName owner:self] autorelease];

    [self addWindowController:controller];
   }
}

-(NSArray *)windowControllers {
   return _windowControllers;
}

-(void)addWindowController:(NSWindowController *)controller {
   [_windowControllers addObject:controller];
   if([controller document]==nil)
    [controller setDocument:self];
}

-(void)removeWindowController:(NSWindowController *)controller  {
   [controller setDocument:nil];
   [_windowControllers removeObjectIdenticalTo:controller];
}

-(NSString *)displayName 
{
  if(_fileURL==nil) 
    {
		NSString* untitledName = NSLocalizedStringFromTableInBundle(@"Untitled", nil, [NSBundle bundleForClass: [NSDocument class]], @"The name of a untitled document");
      if(_untitledNumber != 0)
        return [NSString stringWithFormat:@"%@ %d", untitledName, _untitledNumber];
      else
        return untitledName;
    }
  else 
    {
      return [[NSFileManager defaultManager] displayNameAtPath:[_fileURL path]];
    }
}

-(NSWindow *)windowForSheet {
   if([_windowControllers count]>0){
    NSWindow *check=[[_windowControllers objectAtIndex:0] window];
   
    if(check!=nil)
     return check;
   }
    
   return [NSApp mainWindow];
}

-(BOOL)isDocumentEdited {
   return _changeCount != 0 || [_activeEditors count] > 0;
}

-(void)updateChangeCount:(NSDocumentChangeType)changeType {
   int count=[_windowControllers count];

   switch(changeType){
    case NSChangeDone:
     _changeCount++;
     break;

    case NSChangeUndone:
     _changeCount--;
     break;

    case NSChangeCleared:
     _changeCount=0;
     [self _updateFileModificationDate]; // Since file was just saved or reverted
     break;
    
    case NSChangeReadOtherContents:
    case NSChangeAutosaved:
     NSUnimplementedMethod();
     break;
   }
  
   BOOL edited = [self isDocumentEdited];
   while(--count>=0)
    [[_windowControllers objectAtIndex:count] setDocumentEdited:edited];
}

-(BOOL)readFromData:(NSData *)data ofType:(NSString *)type error:(NSError **)error {
   if([self _isSelectorOverridden:@selector(loadDataRepresentation:ofType:)])
    return [self loadDataRepresentation:data ofType:type];
   else {
    [NSException raise:NSInternalInconsistencyException format:@"-[%@ %s]",isa,sel_getName(_cmd)];
    return NO;
   }
}

-(BOOL)readFromFileWrapper:(NSFileWrapper *)fileWrapper ofType:(NSString *)type error:(NSError **)error {  
   if([self _isSelectorOverridden:@selector(loadFileWrapperRepresentation:ofType:)])
    return [self loadFileWrapperRepresentation:fileWrapper ofType:type];
   else
    return [self readFromData:[fileWrapper regularFileContents] ofType:type error:error];
}

-(BOOL)readFromURL:(NSURL *)url ofType:(NSString *)type error:(NSError **)error {
   if([url isFileURL]){    
    if([self _isSelectorOverridden:@selector(readFromFile:ofType:)]){
     return [self readFromFile:[url path] ofType:type];
    }
    else {
     NSFileWrapper *fileWrapper=[[[NSFileWrapper alloc] initWithPath:[url path]] autorelease];
   
     return [self readFromFileWrapper:fileWrapper ofType:type error:error];
    }
   }
   
   return NO;
}

-(BOOL)revertToContentsOfURL:(NSURL *)url ofType:(NSString *)type error:(NSError **)error {
   if(![self readFromURL:url ofType:type error:error])
    return NO;

   [self updateChangeCount:NSChangeCleared];
   return YES;
}


-(NSData *)dataOfType:(NSString *)type error:(NSError **)error {
   if([self _isSelectorOverridden:@selector(dataRepresentationOfType:)])
    return [self dataRepresentationOfType:type];
    
   [NSException raise:NSInternalInconsistencyException format:@"-[%@ %s]",isa,sel_getName(_cmd)];
   return nil;
}

-(NSFileWrapper *)fileWrapperOfType:(NSString *)type error:(NSError **)error {
   if([self _isSelectorOverridden:@selector(fileWrapperRepresentationOfType:)])
    return [self fileWrapperRepresentationOfType:type];
   else {
    NSData *data=[self dataOfType:type error:error];
    
    if(data==nil)
     return nil;
 
    return [[[NSFileWrapper alloc] initRegularFileWithContents:data] autorelease];
   }
}

-(BOOL)writeToURL:(NSURL *)url ofType:(NSString *)type error:(NSError **)error {
   if([self _isSelectorOverridden:@selector(writeToFile:ofType:)]){
    return [self writeToFile:[url path] ofType:type];
   }
   else {
    NSFileWrapper *wrapper=[self fileWrapperOfType:type error:error];
   
    if(wrapper==nil)
     return NO;
   
    if(![wrapper writeToFile:[url path] atomically:YES updateFilenames:YES])
     return NO;
     
    return YES;
   }
}

-(BOOL)writeToURL:(NSURL *)url ofType:(NSString *)type forSaveOperation:(NSSaveOperationType)operation originalContentsURL:(NSURL *)contentsURL error:(NSError **)error {
	BOOL status = NO;
   if([self _isSelectorOverridden:@selector(writeToFile:ofType:originalFile:saveOperation:)]){
    status = [self writeToFile:[url path] ofType:type originalFile:[contentsURL path] saveOperation:operation];
   }
   else {
    status = [self writeToURL:url ofType:type error:error];
   }
	if (status == YES && [self fileURL] == nil) {
		// It's a new file that's been successfully saved to a url...
		// so note it for the open recent menu
		[[NSDocumentController sharedDocumentController] noteNewRecentDocumentURL: url];
	}
	return status;
}

-(BOOL)writeSafelyToURL:(NSURL *)url ofType:(NSString *)type forSaveOperation:(NSSaveOperationType)operation error:(NSError **)error {
   if(![self writeToURL:url ofType:type forSaveOperation:operation originalContentsURL:url error:error])
    return NO;
    
   NSDictionary *attributes=[self fileAttributesToWriteToURL:url ofType:type forSaveOperation:operation originalContentsURL:url error:error];

   if([attributes count])
    [[NSFileManager defaultManager] changeFileAttributes:attributes atPath:[url path]];
    
   return YES;
}

-(NSDictionary *)fileAttributesToWriteToURL:(NSURL *)url ofType:(NSString *)type forSaveOperation:(NSSaveOperationType)operation originalContentsURL:(NSURL *)contentsURL error:(NSError **)error {
   NSMutableDictionary *result=[NSMutableDictionary dictionary];
   

   return result;
}

-(BOOL)keepBackupFile {
   return NO;
}

-(void)autosaveDocumentWithDelegate:delegate didAutosaveSelector:(SEL)selector contextInfo:(void *)info {
   NSError *error;
   
   if(![self writeToURL:[self autosavedContentsFileURL] ofType:[self autosavingFileType] forSaveOperation:NSAutosaveOperation originalContentsURL:[self fileURL] error:&error]){
   }
   
   NSUnimplementedMethod();
}


-(NSError *)willPresentError:(NSError *)error {
// do nothing
   return error;
}

-(BOOL)presentError:(NSError *)error {
	return [[NSDocumentController sharedDocumentController] presentError:[self willPresentError:error]];
}

-(void)presentError:(NSError *)error modalForWindow:(NSWindow *)window delegate:delegate didPresentSelector:(SEL)selector contextInfo:(void *)info {
	[[NSDocumentController sharedDocumentController] presentError:[self willPresentError:error] modalForWindow:window delegate:delegate didPresentSelector:selector contextInfo:info];
}


-(NSArray *)writableTypesForSaveOperation:(NSSaveOperationType)operation {
   NSArray *result=[[self class] writableTypes];
   
   if(operation==NSSaveToOperation){
    NSMutableArray *filtered=[NSMutableArray array];
    int             i,count=[result count];
    
    for(i=0;i<count;i++){
     NSString *check=[result objectAtIndex:i];
     
     if([[self class] isNativeType:check])
      [filtered addObject:check];
    }
    result=filtered;
   }
   
   return result;
}

-(BOOL)shouldRunSavePanelWithAccessoryView {
   return YES;
}

-(BOOL)prepareSavePanel:(NSSavePanel *)savePanel {
   return YES;
}

-(BOOL)fileNameExtensionWasHiddenInLastRunSavePanel {
   NSUnimplementedMethod();
   return 0;
}

-(NSString *)fileTypeFromLastRunSavePanel {
   NSUnimplementedMethod();
   return nil;
}

-(void)runModalSavePanelForSaveOperation:(NSSaveOperationType)operation 
                                delegate:delegate 
                         didSaveSelector:(SEL)selector 
                             contextInfo:(void *)context 
{
	NSString * path = [_fileURL path];
	NSString * extension = [path pathExtension];
	if([extension length] == 0) {
		extension=[[[NSDocumentController sharedDocumentController] fileExtensionsFromType:[self fileType]] objectAtIndex:0];
	}
	NSSavePanel * savePanel = [NSSavePanel savePanel];
	[savePanel setRequiredFileType:extension];

#if 0
    // setAllowedFileTypes: is unimplemented - so don't call it.
	NSArray* writableTypes = [self writableTypesForSaveOperation: operation];
	[savePanel setAllowedFileTypes: writableTypes];
#endif
    
	if([self prepareSavePanel:savePanel] == NO) { 
		// subclass was unable to prepare the save panel successfully
		// so bail
		return;
	}
	
	int saveResult;
	if (_fileURL) {
		// Suggest saving alongside the original file
		saveResult = [savePanel runModalForDirectory:[path stringByDeletingLastPathComponent]
												file:[path lastPathComponent]];
	} else {
        NSString *directory = [savePanel directory];
        if (directory == nil) {
            // Suggest saving in some reasonable directory
            directory = [[NSDocumentController sharedDocumentController] currentDirectory];
        }
		saveResult = [savePanel runModalForDirectory: directory
                                                file: [self displayName]];
	}
	if(saveResult) {
		NSString *savePath=[savePanel filename];
		NSString* extension = [savePath pathExtension];
		NSString* fileType = [[NSDocumentController sharedDocumentController] typeFromFileExtension: extension];
		
		[[NSUserDefaults standardUserDefaults] setObject:[savePath stringByDeletingLastPathComponent] 
												  forKey:@"NSNavLastRootDirectory"];

		// Try the various saving methods that can be implemented by the document subclass
		if([self _isSelectorOverridden:@selector(saveToFile:saveOperation:delegate:didSaveSelector:contextInfo:)])
        {
			[self saveToFile:savePath 
			   saveOperation:operation 
					delegate:delegate 
			 didSaveSelector:selector
				 contextInfo:context];
        }
		else 
        {
			[self saveToURL:[savePanel URL] 
					 ofType: fileType 
		   forSaveOperation:operation 
				   delegate:delegate 
			didSaveSelector:selector 
				contextInfo:context];
        }
    } 
	else 
    {
		// User cancelled the save panel...
		if ([delegate respondsToSelector:selector])
        {
			// Tell delegate that file couldn't be saved.
			void (*delegateMethod)(id, SEL, id, BOOL, void *);
			delegateMethod = (void (*)(id, SEL, id, BOOL, void *))[delegate methodForSelector:selector];
			delegateMethod(delegate, selector, self, NO, context);
        }     
    }
}

-(void)saveDocumentWithDelegate:delegate didSaveSelector:(SEL)selector contextInfo:(void *)info 
{
	// First make sure there are no uncommitted changes
	for (id editor in [[_activeEditors copy] autorelease]) {
        [editor commitEditing];
	}
	
	// Do we already have a file on disk for this document?
	if (_fileURL != nil) {
      
      // Check if file has been changed by another process
      NSFileManager * fileManager = [NSFileManager defaultManager];
      NSString * path = [_fileURL path];
      NSDictionary * attributes = [fileManager fileAttributesAtPath:path traverseLink:YES];
      NSDate * dateModified = [attributes objectForKey:NSFileModificationDate];
      if (attributes != nil && ![dateModified isEqualToDate:_fileModificationDate])
        {
          int result = NSRunAlertPanel([self displayName],
                                       NSLocalizedStringFromTableInBundle(@"Another user or process has changed this document's file on disk.\n\nIf you save now, those changes will be lost. Save anyway?", nil, [NSBundle bundleForClass: [NSDocument class]], @""),
                                       NSLocalizedStringFromTableInBundle(@"Don't Save", nil, [NSBundle bundleForClass: [NSDocument class]], @""),
									   NSLocalizedStringFromTableInBundle(@"Save", nil, [NSBundle bundleForClass: [NSDocument class]], @""),
									   nil);
          if (result == NSAlertDefaultReturn)
            {
              // The user canceled the save operation.
              if ([delegate respondsToSelector:selector])
                {
                  void (*delegateMethod)(id, SEL, id, BOOL, void *);
                  delegateMethod = (void (*)(id, SEL, id, BOOL, void *))[delegate methodForSelector:selector];
                  delegateMethod(delegate, selector, self, NO, info);
                }
              return;
            }
        }

      if([self _isSelectorOverridden:@selector(saveToFile:saveOperation:delegate:didSaveSelector:contextInfo:)])
        {
          [self saveToFile:[_fileURL path] 
             saveOperation:NSSaveOperation 
                  delegate:delegate 
           didSaveSelector:selector
               contextInfo:info];
        }
      else 
        {
          [self saveToURL:_fileURL 
                   ofType:[self fileType] 
         forSaveOperation:NSSaveOperation 
                 delegate:delegate 
          didSaveSelector:selector 
              contextInfo:info];
        }
    }
  else 
    {
      [self runModalSavePanelForSaveOperation:NSSaveOperation 
                                     delegate:delegate
                              didSaveSelector:selector
                                  contextInfo:info];
    }
}

-(BOOL)saveToURL:(NSURL *)url ofType:(NSString *)type forSaveOperation:(NSSaveOperationType)operation error:(NSError **)error
{
	if(url==nil) {
		return NO;
	}
	else {
		BOOL success=[self writeSafelyToURL:url ofType:type forSaveOperation:operation error:error];

		if(success){
			if(operation!=NSSaveToOperation) {
				[self setFileURL:url];
				[self setFileType: type];
				[self setFileModificationDate: [NSDate date]];
			}
			[self updateChangeCount:NSChangeCleared];
		}
		return success;
	}
}

-(void)saveToURL:(NSURL *)url 
          ofType:(NSString *)type 
forSaveOperation:(NSSaveOperationType)operation 
        delegate:delegate
 didSaveSelector:(SEL)selector 
     contextInfo:(void *)info 
{
  NSError * error = nil;
  BOOL success = [self saveToURL:url ofType:type forSaveOperation:operation error:&error];
  
  if (!success)
    {
      [self presentError:error];
    }
  
  if ([delegate respondsToSelector:selector])
    {
      void (*delegateMethod)(id, SEL, id, BOOL, void *);
      delegateMethod = (void (*)(id, SEL, id, BOOL, void *))[delegate methodForSelector:selector];
      delegateMethod(delegate, selector, self, success, info);
    }
}


-(BOOL)preparePageLayout:(NSPageLayout *)pageLayout {
// do nothing
   return YES;
}

-(BOOL)shouldChangePrintInfo:(NSPrintInfo *)printInfo {
// do nothing
   return YES;
}

-(void)runModalPageLayoutWithPrintInfo:(NSPrintInfo *)printInfo delegate:delegate didRunSelector:(SEL)selector contextInfo:(void *)info {
	int button = [[NSPageLayout pageLayout] runModalWithPrintInfo:printInfo];
	if ([delegate respondsToSelector:selector]) {
		// Tell delegate if the print info was updated.
		void (*delegateMethod)(id, SEL, id, BOOL, void *);
		delegateMethod = (void (*)(id, SEL, id, BOOL, void *))[delegate methodForSelector:selector];
		delegateMethod(delegate, selector, self, button == NSOKButton, info);
	}
}

-(void)runModalPrintOperation:(NSPrintOperation *)printOperation delegate:delegate didRunSelector:(SEL)selector contextInfo:(void *)info {
   NSUnimplementedMethod();
}

-(NSPrintOperation *)printOperationWithSettings:(NSDictionary *)settings error:(NSError **)error {
   NSLog(@"Implement %s in your subclass %@ of NSDocument to enable printing",sel_getName(_cmd),isa);
   return nil;
}

-(void)printDocumentWithSettings:(NSDictionary *)settings showPrintPanel:(BOOL)showPrintPanel delegate:delegate didPrintSelector:(SEL)selector contextInfo:(void *)contextInfo {
   if([self _isSelectorOverridden:@selector(printShowingPrintPanel:)]){
    [self printShowingPrintPanel:showPrintPanel];
   }
   else {
    NSError          *error=nil;
    NSPrintOperation *operation=[self printOperationWithSettings:settings error:&error];
   
    if(operation==nil){
     return;
    }
    [operation setShowsPrintPanel:showPrintPanel];
    [operation runOperation];
   }
// FIX, message delegate
}

-(void)close {
   int count=[_windowControllers count];
   
   while(--count>=0)
    [[_windowControllers objectAtIndex:count] close];

   [[NSDocumentController sharedDocumentController] removeDocument:self];
}

-(void)canCloseDocumentWithDelegate:delegate shouldCloseSelector:(SEL)selector contextInfo:(void *)info 
{
  BOOL OKToClose;
  if ([self isDocumentEdited])
    {
      NSString * fileName = [self fileName];
      if (fileName == nil)
        fileName = [self displayName];
      int result = NSRunAlertPanel([[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleName"], 
                                   NSLocalizedStringFromTableInBundle(@"Save changes to %@?", nil, [NSBundle bundleForClass: [NSDocument class]], @""), 
                                   NSLocalizedStringFromTableInBundle(@"Yes", nil, [NSBundle bundleForClass: [NSDocument class]], @""),
								   NSLocalizedStringFromTableInBundle(@"No", nil, [NSBundle bundleForClass: [NSDocument class]], @""),
								   NSLocalizedStringFromTableInBundle(@"Cancel", nil, [NSBundle bundleForClass: [NSDocument class]], @""),
                                   fileName);
      if (result == NSAlertDefaultReturn)
        {
          [self saveDocumentWithDelegate:delegate
                         didSaveSelector:selector 
                             contextInfo:info];
          return;
        }
      else if (result == NSAlertAlternateReturn)
        {
          // Don't save
          OKToClose = YES;
        }
      else
        {
          // Canceled
          OKToClose = NO;
        }
    }
  else
    {
      // No unsaved changes
      OKToClose = YES;
    }

  if ([delegate respondsToSelector:selector])
    {
      void (*delegateMethod)(id, SEL, id, BOOL, void *);
      delegateMethod = (void (*)(id, SEL, id, BOOL, void *))[delegate methodForSelector:selector];
      delegateMethod(delegate, selector, self, OKToClose, info);
    }
}

-(void)shouldCloseWindowController:(NSWindowController *)controller 
                          delegate:delegate 
               shouldCloseSelector:(SEL)selector 
                       contextInfo:(void *)info 
{
  if ([controller shouldCloseDocument] || [_windowControllers count] <= 1)
    {
      [self canCloseDocumentWithDelegate:delegate
                     shouldCloseSelector:selector 
                             contextInfo:info];
    }
  else if ([delegate respondsToSelector:selector])
    {
      void (*delegateMethod)(id, SEL, id, BOOL, void *);
      delegateMethod = (void (*)(id, SEL, id, BOOL, void *))[delegate methodForSelector:selector];
      delegateMethod(delegate, selector, self, YES, info);
    }
}


-(void)revertDocumentToSaved:sender {
   int result=NSRunAlertPanel(nil,
							  NSLocalizedStringFromTableInBundle(@"%@ has been edited. Are you sure you want to undo changes?", nil, [NSBundle bundleForClass: [NSDocument class]], @""),
							  NSLocalizedStringFromTableInBundle(@"Revert", nil, [NSBundle bundleForClass: [NSDocument class]], @""),
							  NSLocalizedStringFromTableInBundle(@"Cancel", nil, [NSBundle bundleForClass: [NSDocument class]], @""),
							  nil,
							  [self displayName]);

   if(result==NSAlertDefaultReturn)
     {
       for (id editor in [[_activeEditors copy] autorelease])
         [editor discardEditing];
       [self revertToSavedFromFile:[self fileName] ofType:[self fileType]];
     }
}

-(void)saveDocument:sender 
{
  [self saveDocumentWithDelegate:nil didSaveSelector:NULL contextInfo:NULL];
}

-(void)saveDocumentAs:sender {
   [self runModalSavePanelForSaveOperation:NSSaveAsOperation delegate:nil didSaveSelector:NULL contextInfo:NULL];
}

-(void)saveDocumentTo:sender {
   [self runModalSavePanelForSaveOperation:NSSaveToOperation delegate:nil didSaveSelector:NULL contextInfo:NULL];
}

-(void)printDocument:sender {
   [self printDocumentWithSettings:nil showPrintPanel:YES delegate:nil didPrintSelector:NULL contextInfo:NULL];
}


-(void)runPageLayout:sender {
   [[NSPageLayout pageLayout] runModal];
}

-(BOOL)validateUserInterfaceItem:(id <NSValidatedUserInterfaceItem>)item {
   if(sel_isEqual([item action],@selector(revertDocumentToSaved:)))
    return (_fileURL!=nil)?YES:NO;
   if(sel_isEqual([item action],@selector(saveDocument:)))
    return YES;
   if([self respondsToSelector:[item action]]) 
    return YES; 

   return NO;
}

-(void)objectDidBeginEditing:editor 
{
  [_activeEditors addObject:editor];
  
  BOOL edited = [self isDocumentEdited];
  for (NSWindowController * wc in _windowControllers)
    [wc setDocumentEdited:edited];
}

-(void)objectDidEndEditing:editor 
{
  [_activeEditors removeObject:editor];
  
  BOOL edited = [self isDocumentEdited];
  for (NSWindowController * wc in _windowControllers)
    [wc setDocumentEdited:edited];
}

-(BOOL)validateMenuItem:(NSMenuItem *)item {
   if(sel_isEqual([item action],@selector(revertDocumentToSaved:)))
    return (_fileURL!=nil)?YES:NO;
   if(sel_isEqual([item action],@selector(saveDocument:)))
    return YES;
   if([self respondsToSelector:[item action]]) 
    return YES; 

   return NO;
}

-(BOOL)canCloseDocument {
   return YES;
}

-(NSData *)dataRepresentationOfType:(NSString *)type {
   [NSException raise:NSInternalInconsistencyException format:@"-[%@ %s]",isa,sel_getName(_cmd)];
   return nil;
}

-(NSDictionary *)fileAttributesToWriteToFile:(NSString *)path ofType:(NSString *)type saveOperation:(NSSaveOperationType)operation {
   return [NSDictionary dictionary];
}

-(NSString *)fileName {
   return [_fileURL path];
}

-(NSString *)fileNameFromRunningSavePanelForSaveOperation:(NSSaveOperationType)operation {
   NSUnimplementedMethod();
   return nil;
}

-(NSFileWrapper *)fileWrapperRepresentationOfType:(NSString *)type {
   NSData *data=[self dataRepresentationOfType:type];
   
   if(data==nil)
    return nil;
    
   return [[[NSFileWrapper alloc] initRegularFileWithContents:data] autorelease];
}

-initWithContentsOfFile:(NSString *)path ofType:(NSString *)type {
   NSURL   *url=[NSURL fileURLWithPath:path];
   NSError *error;
   
   [self init];

	[self setFileName:path];
	[self setFileType:type];

	error=nil;
   if(![self readFromURL:url ofType:type error:&error]){
	   NSRunAlertPanel(nil,
					   NSLocalizedStringFromTableInBundle(@"Can't open file '%@'. Error = %@", nil, [NSBundle bundleForClass: [NSDocument class]], @""),
					   NSLocalizedStringFromTableInBundle(@"OK", nil, [NSBundle bundleForClass: [NSDocument class]], @""),
					   nil, nil, path, error);
    [self dealloc];
    return nil;
   }
   [self _updateFileModificationDate];

   return self;
}


-initWithContentsOfURL:(NSURL *)url ofType:(NSString *)type {
   NSError  *error;
   
   [self init];

   error=nil;
	[self setFileURL:url];
	[self setFileType:type];
   if(![self readFromURL:url ofType:type error:&error]){
		NSRunAlertPanel(nil,
						NSLocalizedStringFromTableInBundle(@"Can't open file '%@'. Error = %@", nil, [NSBundle bundleForClass: [NSDocument class]], @""),
						NSLocalizedStringFromTableInBundle(@"OK", nil, [NSBundle bundleForClass: [NSDocument class]], @""),
						nil, nil, url, error);
    [self dealloc];
    return nil;
   }
   [self _updateFileModificationDate];

   return self;
}

-(BOOL)loadDataRepresentation:(NSData *)data ofType:(NSString *)type {
   [NSException raise:NSInternalInconsistencyException format:@"-[%@ %s]",isa,sel_getName(_cmd)];
   return NO;
}

-(BOOL)loadFileWrapperRepresentation:(NSFileWrapper *)fileWrapper ofType:(NSString *)type {

   if([fileWrapper isRegularFile])
    return [self loadDataRepresentation:[fileWrapper regularFileContents] ofType:type];

   return NO;
}

-(void)printShowingPrintPanel:(BOOL)flag {
   // do nothing
}

-(BOOL)readFromFile:(NSString *)path ofType:(NSString *)type {
   NSData *data=[[NSData alloc] initWithContentsOfFile:path];

   if(data==nil)
    return NO;

   if(![self loadDataRepresentation:data ofType:type]){
    [data release];
    return NO;
   }

   [data release];
   return YES;
}

-(BOOL)readFromURL:(NSURL *)url ofType:(NSString *)type {
   return [self readFromFile:[url path] ofType:type];
}

-(BOOL)revertToSavedFromFile:(NSString *)path ofType:(NSString *)type {
   if([self readFromFile:path ofType:type]){
    [self updateChangeCount:NSChangeCleared];
    return YES;
   }

   return NO;
}

-(BOOL)revertToSavedFromURL:(NSURL *)url ofType:(NSString *)type {
   if([self readFromURL:url ofType:type]){
    [self updateChangeCount:NSChangeCleared];
    return YES;
   }

   return NO;
}

-(int)runModalSavePanel:(NSSavePanel *)savePanel withAccessoryView:(NSView *)accessoryView {
   NSUnimplementedMethod();
   return 0;
}

-(int)runModalPageLayoutWithPrintInfo:(NSPrintInfo *)printInfo {
   return [[NSPageLayout pageLayout] runModalWithPrintInfo:printInfo];
}

-(void)setFileName:(NSString *)path {
   if (path!=nil)
    [self setFileURL:[NSURL fileURLWithPath:path]];
   else
    [self setFileURL:nil];
}

-(void)saveToFile:(NSString *)path saveOperation:(NSSaveOperationType)operation delegate:delegate didSaveSelector:(SEL)selector contextInfo:(void *)context {
   if(path!=nil){
    BOOL success=[self writeWithBackupToFile:path ofType:_fileType saveOperation:operation];

    if(success){
     if(operation!=NSSaveToOperation)
      [self setFileName:path];
      [self updateChangeCount:NSChangeCleared];
    }

    if ([delegate respondsToSelector:selector])
      {
        void (*delegateMethod)(id, SEL, id, BOOL, void *);
        delegateMethod = (void (*)(id, SEL, id, BOOL, void *))[delegate methodForSelector:selector];
        delegateMethod(delegate, selector, self, success, context);
      }
   }
}


-(BOOL)shouldCloseWindowController:(NSWindowController *)controller {
   if(![controller shouldCloseDocument])
    return NO;
   
   [self canCloseDocumentWithDelegate:nil shouldCloseSelector:NULL contextInfo:NULL];
   return YES;
}

-(BOOL)writeToFile:(NSString *)path ofType:(NSString *)type {
	NSData *data;
	
	if ([self _isSelectorOverridden:@selector(dataRepresentationOfType:)])
		data = [self dataRepresentationOfType:type];
	else
		data = [self dataOfType:type error:NULL];
	
	return [data writeToFile:path atomically:YES];
}

-(BOOL)writeToFile:(NSString *)path ofType:(NSString *)type originalFile:(NSString *)original saveOperation:(NSSaveOperationType)operation {
   NSUnimplementedMethod();
   return 0;
}

-(BOOL)writeToURL:(NSURL *)url ofType:(NSString *)type {
   NSUnimplementedMethod();
   return 0;
}

-(BOOL)writeWithBackupToFile:(NSString *)path ofType:(NSString *)type saveOperation:(NSSaveOperationType)operation {
   // move original to backup

   if(![self writeToFile:path ofType:type])
    return NO;

   if(![self keepBackupFile]){
    // delete backup
   }
   return YES;
}

-(void)_setUntitledNumber:(int)number {
   _untitledNumber=number;
}



-(void)_undoManagerDidUndoChange:(NSNotification *)note {
    [self updateChangeCount:NSChangeUndone];
}

-(void)_undoManagerDidRedoChange:(NSNotification *)note {
    [self updateChangeCount:NSChangeDone];
}

-(void)_undoManagerDidCloseGroup:(NSNotification *)note {
// FIXME: the following would mark just opened and otherwise pristine documents as changed
//  [self updateChangeCount:NSChangeDone];
}


-(BOOL)windowShouldClose:sender {
   if([[NSUserDefaults standardUserDefaults] boolForKey:@"useSheets"]){
    NSBeginAlertSheet(nil,
					  NSLocalizedStringFromTableInBundle(@"Yes", nil, [NSBundle bundleForClass: [NSDocument class]], @""), NSLocalizedStringFromTableInBundle(@"No", nil, [NSBundle bundleForClass: [NSDocument class]], @""), NSLocalizedStringFromTableInBundle(@"Cancel", nil, [NSBundle bundleForClass: [NSDocument class]], @""),
					  sender, self,
					  @selector(didEndShouldCloseSheet:returnCode:contextInfo:),
					  NULL, sender,
					  NSLocalizedStringFromTableInBundle(@"Save changes to %@?", nil, [NSBundle bundleForClass: [NSDocument class]], @""), [self displayName]);

    return NO;
   }
   else {
    if(![self isDocumentEdited])
     return YES;
    else {
     int result=NSRunAlertPanel(nil, 
								NSLocalizedStringFromTableInBundle(@"Save changes to %@?", nil, [NSBundle bundleForClass: [NSDocument class]], @""),
								NSLocalizedStringFromTableInBundle(@"Yes", nil, [NSBundle bundleForClass: [NSDocument class]], @""), NSLocalizedStringFromTableInBundle(@"No", nil, [NSBundle bundleForClass: [NSDocument class]], @""), NSLocalizedStringFromTableInBundle(@"Cancel", nil, [NSBundle bundleForClass: [NSDocument class]], @""),
								[self displayName]);

     switch(result){
      case NSAlertDefaultReturn:
       [self saveDocument:nil];
       return YES;

      case NSAlertAlternateReturn:
			 // Let's prep the document to close cleanly.
			 for (id editor in [[_activeEditors copy] autorelease]) {
				[editor discardEditing];
			 }
			 [self updateChangeCount: NSChangeCleared];
       return YES;

      case NSAlertOtherReturn:
      default:
       return NO;
     }
    }
   }
}

-(void)didEndShouldCloseSheet:(NSWindow *)sheet
        returnCode:(int)returnCode 
        contextInfo:(void *)contextInfo {
   NSWindow *window=(NSWindow *)contextInfo;

   switch(returnCode){
    case NSAlertDefaultReturn:
     [self saveDocument:nil];
     [window close];
     break;

    case NSAlertAlternateReturn:
     [window close];
     break;

    case NSAlertOtherReturn:
    default:
     break;
   }
}

#if 0
-(void)windowWillClose:(NSNotification *)note {
   NSWindow *window=[note object];
   int       count=[_windowControllers count];

   while(--count>=0){
    NSWindowController *controller=[_windowControllers objectAtIndex:count];

    if([controller isWindowLoaded] && window==[controller window]){
     BOOL closeMe = [controller shouldCloseDocument]; 
     [_windowControllers removeObjectAtIndex:count]; 
     if (closeMe) 
      [self close]; 
     return; 
    }
   }
}
#endif

- (void)textDidChange:(NSNotification *)aNotification
{
   [self updateChangeCount:NSChangeDone];
}

@end
