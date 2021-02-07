/** <title>NSDocument</title>

   <abstract>The abstract document class</abstract>

   Copyright (C) 1999 Free Software Foundation, Inc.

   Author: Carl Lindberg <Carl.Lindberg@hbo.com>
   Date: 1999
   Modifications: Fred Kiefer <FredKiefer@gmx.de>
   Date: June 2000, Dec 2006

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

#import <Foundation/NSData.h>
#import <Foundation/NSError.h>
#import <Foundation/NSException.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSProcessInfo.h>
#import <Foundation/NSUndoManager.h>
#import <Foundation/NSURL.h>
#import "AppKit/NSBox.h"
#import "AppKit/NSDocument.h"
#import "AppKit/NSFileWrapper.h"
#import "AppKit/NSSavePanel.h"
#import "AppKit/NSPageLayout.h"
#import "AppKit/NSPopUpButton.h"
#import "AppKit/NSPrintInfo.h"
#import "AppKit/NSPrintOperation.h"
#import "AppKit/NSView.h"
#import "NSDocumentFrameworkPrivate.h"

#import "GSGuiPrivate.h"

static inline NSError*
create_error(int code, NSString* desc)
{
  return [NSError errorWithDomain: @"NSDocument"
                  code: code 
                  userInfo: [NSDictionary 
                                dictionaryWithObjectsAndKeys: desc,
                                NSLocalizedDescriptionKey, nil]];
}

@implementation NSDocument

+ (NSArray *) readableTypes
{
  // FIXME: Should allow for filterable types
  return [[NSDocumentController sharedDocumentController]
          _readableTypesForClass: self];
}

+ (NSArray *) writableTypes
{
  // FIXME: Should allow for filterable types
  return [[NSDocumentController sharedDocumentController] 
          _writableTypesForClass: self];
}

+ (BOOL) isNativeType: (NSString *)type
{
  return ([[self readableTypes] containsObject: type] &&
          [[self writableTypes] containsObject: type]);
}

/* 
 * Private helper macro to check, if the method given via the selector sel 
 * has been overridden in the current subclass.
 */
#define OVERRIDDEN(sel) ([self methodForSelector: @selector(sel)] != [[NSDocument class] instanceMethodForSelector: @selector(sel)])

- (id) init
{
  static int untitledCount = 1;
  NSArray *fileTypes;
  
  self = [super init];
  if (self != nil)
    {
      _document_index = untitledCount++;
      _window_controllers = [[NSMutableArray alloc] init];
      fileTypes = [[self class] readableTypes];
      _doc_flags.has_undo_manager = YES;

      /* Set our default type */
      if ([fileTypes count])
       { 
         [self setFileType: [fileTypes objectAtIndex: 0]];
         ASSIGN(_save_type, [fileTypes objectAtIndex: 0]);
       }
    }
  return self;
}

/**
 * Initialises the receiver with the contents of the document at fileName
 * assuming that the type of data is as specified by fileType.<br />
 * Destroys the receiver and returns nil on failure.
 */
- (id) initWithContentsOfFile: (NSString*)fileName ofType: (NSString*)fileType
{
  self = [self init];
  if (self != nil)
    {
      // Setting these values first is contrary to the documentation,
      // but mathces the reported behaviour on Cocoa.
      [self setFileType: fileType];
      [self setFileName: fileName];
      if (![self readFromFile: fileName ofType: fileType])
        {
          NSRunAlertPanel (_(@"Load failed"),
                          _(@"Could not load file %@."),
                           nil, nil, nil, fileName);
          DESTROY(self);
        }
    }
  return self;
}

/**
 * Initialises the receiver with the contents of the document at url
 * assuming that the type of data is as specified by fileType.<br />
 * Destroys the receiver and returns nil on failure.
 */
- (id) initWithContentsOfURL: (NSURL*)url ofType: (NSString*)fileType
{
  self = [self init];
  if (self != nil)
    {
      [self setFileType: fileType];
      [self setFileName: [url path]];
      if (![self readFromURL: url ofType: fileType])
        {
          NSRunAlertPanel(_(@"Load failed"),
                          _(@"Could not load URL %@."),
                          nil, nil, nil, [url absoluteString]);
          DESTROY(self);
        }
    }  
  return self;
}

- (id) initForURL: (NSURL *)forUrl
withContentsOfURL: (NSURL *)url
           ofType: (NSString *)type
            error: (NSError **)error
{
  self = [self initWithType: type error: error];
  if (self != nil)
    {
      [self setFileType: type];
      if (forUrl)
        [self setFileURL: forUrl];
      if ([self readFromURL: url
                     ofType: type
                      error: error])
        {
          if (![url isEqual: forUrl])
            {
              [self setAutosavedContentsFileURL: url];
              [self updateChangeCount: NSChangeReadOtherContents];
            }
        }
      else 
        {
          DESTROY(self);
        }
    }
  return self;
}

- (id) initWithContentsOfURL: (NSURL *)url
                      ofType: (NSString *)type
                       error: (NSError **)error
{
  if (OVERRIDDEN(initWithContentsOfFile:ofType:) && [url isFileURL])
    {
      self = [self initWithContentsOfFile: [url path] ofType: type];
    }
  else
    {
      self = [self initForURL: url
                   withContentsOfURL: url
                   ofType: type
                   error: error];
    }

  [self setFileModificationDate: [NSDate date]];
  return self;
}

- (id) initWithType: (NSString *)type
              error: (NSError **)error
{
  self = [self init];
  if (self != nil)
    {
      [self setFileType: type];
    }
  return self;
}

- (void) dealloc
{
  [[NSNotificationCenter defaultCenter] removeObserver: self];
  RELEASE(_undo_manager);
  RELEASE(_file_name);
  RELEASE(_file_url);
  RELEASE(_file_type);
  RELEASE(_last_component_file_name);
  RELEASE(_autosaved_file_url);
  RELEASE(_file_modification_date);
  RELEASE(_window_controllers);
  RELEASE(_window);
  RELEASE(_print_info);
  RELEASE(_printOp_delegate);
  RELEASE(_save_panel_accessory);
  RELEASE(_spa_button);
  RELEASE(_save_type);
  [super dealloc];
}

- (NSString *) fileName
{
  return _file_name;
}

- (void) setFileName: (NSString *)fileName
{
  NSURL *fileUrl;

  if (fileName && ![fileName isAbsolutePath])
    {
      NSString *dir = [[NSFileManager defaultManager] currentDirectoryPath];
      
      if (dir)
	{
	  fileName = [dir stringByAppendingPathComponent: fileName];
	}
    }

  fileUrl = fileName ? [NSURL fileURLWithPath: fileName] : nil;

  // This check is to prevent super calls from recursing.
  if (!OVERRIDDEN(setFileName:))
    {
      [self setFileURL: fileUrl];
    }
  else
    {
      ASSIGN(_file_name, fileName);
      ASSIGN(_file_url, fileUrl);
      [self setLastComponentOfFileName: [_file_name lastPathComponent]];
    }
}

- (NSString *) fileType
{
  return _file_type;
}

- (void) setFileType: (NSString *)type
{
  ASSIGN(_file_type, type);
}

- (NSURL *) fileURL
{
  if (OVERRIDDEN(fileName))
    {
      NSString *fileName = [self fileName];

      return fileName ? [NSURL fileURLWithPath: fileName] : nil;
    }
  else
    {
      return _file_url;
    }
}

- (void) setFileURL: (NSURL *)url
{
  if (OVERRIDDEN(setFileName:) && 
      ((url == nil) || [url isFileURL]))
    {
      [self setFileName: [url path]];
    }
  else
    {
      ASSIGN(_file_url, url);
      ASSIGN(_file_name, (url && [url isFileURL]) ? [url path] : (NSString*)nil);
      [self setLastComponentOfFileName: [[_file_url path] lastPathComponent]];
    }
}

- (NSDate *) fileModificationDate
{
  return _file_modification_date;
}

- (void) setFileModificationDate: (NSDate *)date
{
  ASSIGN(_file_modification_date, date);
}

- (NSString *) lastComponentOfFileName
{
  return _last_component_file_name;
}

- (void) setLastComponentOfFileName: (NSString *)str
{
  ASSIGN(_last_component_file_name, str);

  [[self windowControllers] makeObjectsPerformSelector:
                                @selector(synchronizeWindowTitleWithDocumentName)];
}

- (NSArray *) windowControllers
{
  return _window_controllers;
}

- (void) addWindowController: (NSWindowController *)windowController
{
  [_window_controllers addObject: windowController];
  if ([windowController document] != self)
    {
      [windowController setDocument: self];
      [windowController setDocumentEdited: [self isDocumentEdited]];
    }
}

- (void) removeWindowController: (NSWindowController *)windowController
{
  if ([_window_controllers containsObject: windowController])
    {
      [windowController setDocumentEdited: NO];
      [windowController setDocument: nil];
      [_window_controllers removeObject: windowController];
    }
}

- (NSString *) windowNibName
{
  return nil;
}

// private; called during nib load.  
// we do not retain the window, since it should
// already have a retain from the nib.
- (void) setWindow: (NSWindow *)aWindow
{
  _window = aWindow;
}

- (NSWindow *) windowForSheet
{
  NSWindow *win;

  if (([_window_controllers count] > 0) &&
      ((win = [[_window_controllers objectAtIndex: 0] window]) != nil))
    {
      return win;
    }

  /* Note: While Apple's documentation says that this method returns
   * [NSApp mainWindow] if the document has no window controllers, the
   * actual implementation returns nil and, in fact, the header files
   * on OS X also say so. Since it would be very unreasonable to attach a
   * document modal sheet to a window that doesn't belong to this document,
   * we do the same here, too. */
  return nil;
}

/**
 * Creates the window controllers for the current document.  Calls
 * addWindowController: on the receiver to add them to the controller 
 * array.
 */
- (void) makeWindowControllers
{
  NSString *name = [self windowNibName];

  if (name != nil && [name length] > 0)
    {
      NSWindowController *controller;

      controller = [[NSWindowController alloc] initWithWindowNibName: name
                                                               owner: self];
      [self addWindowController: controller];
      RELEASE(controller);
    }
  else
    {
      [NSException raise: NSInternalInconsistencyException
                  format: @"%@ must override either -windowNibName "
        @"or -makeWindowControllers", NSStringFromClass([self class])];
    }
}

/**
 * Makes all the documents windows visible by ordering them to the
 * front and making them main or key.<br />
 * If the document has no windows, this method has no effect.
 */
- (void) showWindows
{
  [_window_controllers makeObjectsPerformSelector: @selector(showWindow:)
                                      withObject: self];
}

- (BOOL) isDocumentEdited
{
  return _change_count != 0 || _doc_flags.permanently_modified;
}

- (void) updateChangeCount: (NSDocumentChangeType)change
{
  int i, count = [_window_controllers count];
  BOOL isEdited;
  
  switch (change)
    {
    case NSChangeDone:          _change_count++; 
                                _autosave_change_count++; 
                                break;
    case NSChangeUndone:        _change_count--; 
                                _autosave_change_count--; 
                                break;
    case NSChangeReadOtherContents:
                                _doc_flags.permanently_modified = 1;
                                break;
    case NSChangeCleared:        _change_count = 0; 
                                _autosave_change_count = 0; 
                                _doc_flags.permanently_modified = 0;
                                _doc_flags.autosave_permanently_modified = 0;
                                break;
    case NSChangeAutosaved:     _autosave_change_count = 0; 
                                _doc_flags.autosave_permanently_modified = 0;
                                break;
    }
  
    /*
     * NOTE: Apple's implementation seems to not call -isDocumentEdited
     * here but directly checks to see if _changeCount == 0.  It seems it
     * would be better to call the method in case it's overridden by a
     * subclass, but we may want to keep Apple's behavior.
     */
  isEdited = [self isDocumentEdited];

  for (i = 0; i < count; i++)
    {
      [[_window_controllers objectAtIndex: i] setDocumentEdited: isEdited];
    }
}

- (BOOL) canCloseDocument
{
  int result;

  if (![self isDocumentEdited])
    return YES;

  result = NSRunAlertPanel (_(@"Close"), 
                            _(@"%@ has changed.  Save?"),
                            _(@"Save"), _(@"Cancel"), _(@"Don't Save"), 
                            [self displayName]);
  
#define Save     NSAlertDefaultReturn
#define Cancel   NSAlertAlternateReturn
#define DontSave NSAlertOtherReturn

  switch (result)
    {
      // return NO if save failed
    case Save:
      {
        [self saveDocument: nil]; 
        return ![self isDocumentEdited];
      }
    case DontSave:        return YES;
    case Cancel:
    default:              return NO;
    }
}

- (void) canCloseDocumentWithDelegate: (id)delegate 
                  shouldCloseSelector: (SEL)shouldCloseSelector 
                          contextInfo: (void *)contextInfo
{
  BOOL result = [self canCloseDocument];

  if (delegate != nil && shouldCloseSelector != NULL)
    {
      void (*meth)(id, SEL, id, BOOL, void*);
      meth = (void (*)(id, SEL, id, BOOL, void*))[delegate methodForSelector: 
                                                               shouldCloseSelector];
      if (meth)
        meth(delegate, shouldCloseSelector, self, result, contextInfo);
    }
}

- (BOOL) shouldCloseWindowController: (NSWindowController *)windowController
{
  if (![_window_controllers containsObject: windowController]) return YES;

  /* If it's the last window controller, pop up a warning */
  /* maybe we should count only loaded window controllers (or visible windows). */
  if ([windowController shouldCloseDocument]
      || [_window_controllers count] == 1)
    {
      return [self canCloseDocument];
    }
        
  return YES;
}

- (void) shouldCloseWindowController: (NSWindowController *)windowController 
                            delegate: (id)delegate 
                 shouldCloseSelector: (SEL)callback
                         contextInfo: (void *)contextInfo
{
  /* If it's the last window controller, pop up a warning */
  /* maybe we should count only loaded window controllers (or visible windows). */
  if ([_window_controllers containsObject: windowController] 
      && ([windowController shouldCloseDocument]
          || [_window_controllers count] == 1))
    {
      [self canCloseDocumentWithDelegate: delegate 
            shouldCloseSelector: callback 
            contextInfo: contextInfo];
      return;
    }

  if (delegate != nil && callback != NULL)
    {
      void (*meth)(id, SEL, id, BOOL, void*);
      meth = (void (*)(id, SEL, id, BOOL, void*))[delegate methodForSelector: 
                                                               callback];
      
      if (meth)
        meth(delegate, callback, self, YES, contextInfo);
    }
}

- (NSString *) displayName
{
  if ([self lastComponentOfFileName] != nil)
    {
      if ([self fileNameExtensionWasHiddenInLastRunSavePanel])
        {
          return [[self lastComponentOfFileName] stringByDeletingPathExtension];
        }
      else
        {
          return [self lastComponentOfFileName];
        }
    }
  else
    {
      return [NSString stringWithFormat: _(@"Untitled-%d"), _document_index];
    }
}

- (BOOL) keepBackupFile
{
  return NO;
}

- (NSData *) dataRepresentationOfType: (NSString *)type
{
  [NSException raise: NSInternalInconsistencyException format:@"%@ must implement %@",
               NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
  return nil;
}

- (NSData *) dataOfType: (NSString *)type
                  error: (NSError **)error
{
  if (OVERRIDDEN(dataRepresentationOfType:))
    {
      if (error)
	*error = nil; 
      return [self dataRepresentationOfType: type];
    }

  [NSException raise: NSInternalInconsistencyException format:@"%@ must implement %@",
               NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
  return nil;
}

- (BOOL) loadDataRepresentation: (NSData *)data ofType: (NSString *)type
{
  [NSException raise: NSInternalInconsistencyException format:@"%@ must implement %@",
               NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
  return NO;
}

- (NSFileWrapper *) fileWrapperRepresentationOfType: (NSString *)type
{
  NSData *data = [self dataRepresentationOfType: type];
  
  if (data == nil) 
    return nil;

  return AUTORELEASE([[NSFileWrapper alloc] initRegularFileWithContents: data]);
}

- (NSFileWrapper *) fileWrapperOfType: (NSString *)type
                                error: (NSError **)error
{
  NSData *data;
  
  if (OVERRIDDEN(fileWrapperRepresentationOfType:))
    {
      if (error)
	*error = nil; 
      return [self fileWrapperRepresentationOfType: type];
    }

  data = [self dataOfType: type error: error];
  
  if (data == nil) 
    {
      if (error && !(*error))
          *error = create_error(0, NSLocalizedString(@"Could not create data for type.",
                                                     @"Error description"));
      return nil;
    }
  return AUTORELEASE([[NSFileWrapper alloc] initRegularFileWithContents: data]);
}

- (BOOL) loadFileWrapperRepresentation: (NSFileWrapper *)wrapper 
                                ofType: (NSString *)type
{
  if ([wrapper isRegularFile])
    {
      return [self loadDataRepresentation: [wrapper regularFileContents] 
                   ofType: type];
    }
  
    /*
     * This even happens on a symlink.  May want to use
     * -stringByResolvingAllSymlinksInPath somewhere, but Apple doesn't.
     */
  NSLog(@"%@ must be overridden if your document deals with file packages.",
        NSStringFromSelector(_cmd));

  return NO;
}

- (BOOL) writeToFile: (NSString *)fileName ofType: (NSString *)type
{
  return [[self fileWrapperRepresentationOfType: type]
           writeToFile: fileName atomically: YES updateFilenames: YES];
}

- (BOOL) readFromFile: (NSString *)fileName ofType: (NSString *)type
{
  NSFileWrapper *wrapper = AUTORELEASE([[NSFileWrapper alloc] initWithPath: fileName]);
  return [self loadFileWrapperRepresentation: wrapper ofType: type];
}

- (BOOL) revertToSavedFromFile: (NSString *)fileName ofType: (NSString *)type
{
  return [self readFromFile: fileName ofType: type];
}

- (BOOL) writeToURL: (NSURL *)url ofType: (NSString *)type
{
  if ([url isFileURL])
    {
      return [self writeToFile: [url path] ofType: type];
    }

  return NO;
}

- (BOOL) readFromURL: (NSURL *)url ofType: (NSString *)type
{
  if ([url isFileURL])
    {
      return [self readFromFile: [url path] ofType: type];
    }

  return NO;
}

- (BOOL) revertToSavedFromURL: (NSURL *)url ofType: (NSString *)type
{
  return [self readFromURL: url ofType: type];
}

- (BOOL) readFromData: (NSData *)data
               ofType: (NSString *)type
                error: (NSError **)error
{
  if (OVERRIDDEN(loadDataRepresentation:ofType:))
    {
      if (error)
	*error = nil; 
      return [self loadDataRepresentation: data
                   ofType: type];
    }

  [NSException raise: NSInternalInconsistencyException format:@"%@ must implement %@",
               NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
  return NO;
}

- (BOOL) readFromFileWrapper: (NSFileWrapper *)wrapper
                      ofType: (NSString *)type
                       error: (NSError **)error
{
  if (OVERRIDDEN(loadFileWrapperRepresentation:ofType:))
    {
      if (error)
	*error = nil; 
      return [self loadFileWrapperRepresentation: wrapper ofType: type];
    }

  if ([wrapper isRegularFile])
    {
      return [self readFromData: [wrapper regularFileContents]
                         ofType: type
                          error: error];
    }

  if (error)
    {
      *error = create_error(0, NSLocalizedString(@"File wrapper is no file.",
                                                 @"Error description"));
    }
  return NO;
}

- (BOOL) readFromURL: (NSURL *)url
              ofType: (NSString *)type
               error: (NSError **)error
{
  if ([url isFileURL])
    {
      NSString *fileName = [url path];
      
      if (OVERRIDDEN(readFromFile:ofType:))
        {
	  if (error)
	    *error = nil;
          return [self readFromFile: fileName ofType: type];
        }
      else
        {
          NSFileWrapper *wrapper = AUTORELEASE([[NSFileWrapper alloc] initWithPath: fileName]);
          
          return [self readFromFileWrapper: wrapper 
                       ofType: type
                       error: error];
        }
    }
  else
    {
      return [self readFromData: [url resourceDataUsingCache: YES]
                   ofType: type
                   error: error];
    }
}

- (BOOL) revertToContentsOfURL: (NSURL *)url
                        ofType: (NSString *)type
                         error: (NSError **)error
{
  if (OVERRIDDEN(revertToSavedFromURL:ofType:))
    {
      return [self revertToSavedFromURL: url ofType: type];
    }
  if (OVERRIDDEN(revertToSavedFromFile:ofType:) && [url isFileURL])
    {
      return [self revertToSavedFromFile:[url path] ofType: type];
    }
  return [self readFromURL: url
                    ofType: type
                     error: error];
}

- (BOOL) writeToFile: (NSString *)fileName 
              ofType: (NSString *)type 
        originalFile: (NSString *)origFileName
       saveOperation: (NSSaveOperationType)saveOp
{
  return [self writeToFile: fileName ofType: type];
}


- (NSString *) _backupFileNameFor: (NSString *)newFileName 
{
  NSString *extension = [newFileName pathExtension];
  NSString *backupFilename = [newFileName stringByDeletingPathExtension];

  backupFilename = [backupFilename stringByAppendingString:@"~"];
  return [backupFilename stringByAppendingPathExtension: extension];
}

- (BOOL) _writeBackupForFile: (NSString *)newFileName 
                      toFile: (NSString *)backupFilename
{
  NSFileManager *fileManager = [NSFileManager defaultManager];

  /* NSFileManager movePath: will fail if destination exists */
  /* Save panel has already asked if the user wants to replace it */
  if ([fileManager fileExistsAtPath: backupFilename])
    {
      [fileManager removeFileAtPath: backupFilename handler: nil];
    }
      
  // Move or copy?
  if (![fileManager movePath: newFileName toPath: backupFilename handler: nil] &&
      [self keepBackupFile])
    {
      int result = NSRunAlertPanel(_(@"File Error"),
                                   _(@"Can't create backup file.  Save anyways?"),
                                   _(@"Save"), _(@"Cancel"), nil);
      
      if (result != NSAlertDefaultReturn) return NO;
    }

  return YES;
}

- (BOOL) writeWithBackupToFile: (NSString *)fileName 
                        ofType: (NSString *)fileType 
                 saveOperation: (NSSaveOperationType)saveOp
{
  NSFileManager *fileManager = [NSFileManager defaultManager];
  NSString *backupFilename = nil;
  BOOL isNativeType = [[self class] isNativeType: fileType];

  if (fileName && isNativeType)
    {
      if ([fileManager fileExistsAtPath: fileName])
        {
          backupFilename = [self _backupFileNameFor: fileName];
          
          if (![self _writeBackupForFile: fileName
                     toFile: backupFilename])
            {
              return NO;
            }
        }

      if ([self writeToFile: fileName 
                ofType: fileType
                originalFile: backupFilename
                saveOperation: saveOp])
        {
          // FIXME: Should set the file attributes
          
          if (saveOp != NSSaveToOperation)
            {
              [self _removeAutosavedContentsFile];
              [self setFileName: fileName];
              [self setFileType: fileType];
              [self updateChangeCount: NSChangeCleared];
            }
          
          if (backupFilename && ![self keepBackupFile])
            {
              [fileManager removeFileAtPath: backupFilename handler: nil];
            }
          
          return YES;
        }
    }

  return NO;
}

- (BOOL) writeSafelyToURL: (NSURL *)url
                   ofType: (NSString *)type
         forSaveOperation: (NSSaveOperationType)op
                    error: (NSError **)error
{
  NSURL *original = [self fileURL];
  NSFileManager *fileManager = [NSFileManager defaultManager];
  NSString *backupFilename = nil;
  BOOL isNativeType = [[self class] isNativeType: type];
      
  if (OVERRIDDEN(writeWithBackupToFile:ofType:saveOperation:))
    {
      BOOL isAutosave = NO;

      if (op == NSAutosaveOperation)
        {
          op = NSSaveToOperation;
          isAutosave = YES;
        }

      if (error)
	*error = nil; 
      if (![self writeWithBackupToFile: [url path] 
                 ofType: type 
                 saveOperation: op])
	{
          if (error)
            {
              *error = create_error(0, NSLocalizedString(@"Could not write backup file.",
                                                         @"Error description"));
            }
	  return NO;
	}

      if (isAutosave)
	{
	  [self setAutosavedContentsFileURL: url];
	  [self updateChangeCount: NSChangeAutosaved];
	}
      return YES;
    }

  if (!isNativeType || (url == nil))
    {
      if (error)
        {
          *error = create_error(0, NSLocalizedString(@"Not a writable type or no URL given.",
                                                     @"Error description"));
        }
      return NO;
    }
  
  if (op == NSSaveOperation)
    {
      if ([url isFileURL])
        {
          NSString *fileName;
          
          fileName = [url path];
          if ([fileManager fileExistsAtPath: fileName])
            {
              backupFilename = [self _backupFileNameFor: fileName];
              
              if (![self _writeBackupForFile: fileName
                         toFile: backupFilename])
                {
		  if (error)
                    {
                      *error = create_error(0, NSLocalizedString(@"Could not write backup file.",
                                                                 @"Error description"));
                    }
                  return NO;
                }
            }
        }
    }
      
  if (![self writeToURL: url 
             ofType: type 
             forSaveOperation: op 
             originalContentsURL: original
             error: error])
    {
      return NO;
    }

  if ([url isFileURL])
    {
      NSDictionary *attrs;

      attrs = [self fileAttributesToWriteToURL: url
                    ofType: type 
                    forSaveOperation: op 
                    originalContentsURL: original
                    error: error];
      [fileManager changeFileAttributes: attrs atPath: [url path]];
    }

  if (op == NSAutosaveOperation)
    {
      [self setAutosavedContentsFileURL: url];
      [self updateChangeCount: NSChangeAutosaved];
    }
  else if (op != NSSaveToOperation)
    {
      [self _removeAutosavedContentsFile];
      [self setFileURL: url];
      [self setFileType: type];
      [self updateChangeCount: NSChangeCleared];
    }

  if (backupFilename && ![self keepBackupFile])
    {
      [fileManager removeFileAtPath: backupFilename handler: nil];
    }
              
  return YES;
}

- (BOOL) writeToURL: (NSURL *)url 
             ofType: (NSString *)type 
              error: (NSError **)error
{
  if ([url isFileURL])
    {
      NSFileWrapper *wrapper;

      if (OVERRIDDEN(writeToFile:ofType:))
        {
	  if (error)
	    *error = nil; 
          return [self writeToFile: [url path] ofType: type];
        }

      wrapper = [self fileWrapperOfType: type
                      error: error];
      if (wrapper == nil)
        {
	  if (error && !(*error))
            {
              *error = create_error(0, NSLocalizedString(@"Could not write file wrapper.",
                                                         @"Error description"));
            }
          return NO;
        }

      if (error)
	*error = nil; 
      return [wrapper writeToFile: [url path] atomically: YES updateFilenames: YES];
    }
  else
    {
      NSData *data =  [self dataOfType: type error: error];
  
      if (data == nil)
          return NO;
      
      return [url setResourceData: data];
    }
}

- (BOOL) writeToURL: (NSURL *)url
             ofType: (NSString *)type
   forSaveOperation: (NSSaveOperationType)op
originalContentsURL: (NSURL *)orig
              error: (NSError **)error
{
  if (OVERRIDDEN(writeToFile:ofType:originalFile:saveOperation:))
    {
      if (op == NSAutosaveOperation)
        {
          op = NSSaveToOperation;
        }

      if (error)
	*error = nil; 
      return [self writeToFile: [url path] 
                   ofType: type 
                   originalFile: [orig path] 
                   saveOperation: op];
    }

  return [self writeToURL: url
               ofType: type
               error: error];
}

- (IBAction) changeSaveType: (id)sender
{ 
  NSDocumentController *controller = 
    [NSDocumentController sharedDocumentController];
  NSArray *extensions;

  ASSIGN(_save_type, [[sender selectedItem] representedObject]);
  extensions = [controller fileExtensionsFromType: _save_type];
  if ([extensions containsObject: @"*"])
    extensions = nil;
  [(NSSavePanel *)[sender window] setAllowedFileTypes: extensions];
}

- (NSInteger) runModalSavePanel: (NSSavePanel *)savePanel 
              withAccessoryView: (NSView *)accessoryView
{
  NSString *directory, *file;

  if (accessoryView)
    {
      [savePanel setAccessoryView: accessoryView];
    }

  if ([self fileName])
    {
      directory = [[self fileName] stringByDeletingLastPathComponent];
      file = [[self fileName] lastPathComponent];
      if (![savePanel allowsOtherFileTypes])
	{
	  NSArray *exts = [savePanel allowedFileTypes];
	  if ([exts count] && ![exts containsObject: [file pathExtension]] &&
	      ![exts containsObject: @"*"])
	    {
	      file = [file stringByDeletingPathExtension];
	      file = [file stringByAppendingPathExtension:
			     [exts objectAtIndex: 0]];
	    }
	}
      return [savePanel runModalForDirectory: directory file: file];
    }

  return [savePanel runModal];
}

- (BOOL) prepareSavePanel: (NSSavePanel *)savePanel
{
  return YES;
}

- (BOOL) shouldRunSavePanelWithAccessoryView
{
  return YES;
}

- (void) _createPanelAccessory
{
  if (_save_panel_accessory == nil)
    {
      NSRect accessoryFrame = NSMakeRect(0,0,380,70);
      NSRect spaFrame = NSMakeRect(115,14,150,22);

      _save_panel_accessory = [[NSBox alloc] initWithFrame: accessoryFrame];
      [(NSBox *)_save_panel_accessory setTitle: _(@"File Type")];
      [_save_panel_accessory setAutoresizingMask: 
                            NSViewWidthSizable | NSViewHeightSizable];
      _spa_button = [[NSPopUpButton alloc] initWithFrame: spaFrame];
      [_spa_button setAutoresizingMask: NSViewWidthSizable | NSViewHeightSizable | NSViewMinYMargin |
                 NSViewMaxYMargin | NSViewMinXMargin | NSViewMaxXMargin];
      [_spa_button setTarget: self];
      [_spa_button setAction: @selector(changeSaveType:)];
      [_save_panel_accessory addSubview: _spa_button];
    }
}

- (void) _addItemsToSpaButtonFromArray: (NSArray *)types
{
  NSString *type, *title;
  int i, count = [types count];

  [_spa_button removeAllItems];
  for (i = 0; i < count; i++)
    {
      type = [types objectAtIndex: i];
      title = [[NSDocumentController sharedDocumentController]
		displayNameForType: type];
      [_spa_button addItemWithTitle: title];
      [[_spa_button itemAtIndex: i] setRepresentedObject: type];
    }

  // if it's more than one, then
  [_spa_button setEnabled: (count > 0)];
  
  // if we have some items, select the current filetype.
  if (count > 0)
    {
      [_spa_button selectItemAtIndex:
	[_spa_button indexOfItemWithRepresentedObject: [self fileType]]];
    }
}

- (NSSavePanel *)_runSavePanelForSaveOperation: (NSSaveOperationType)saveOperation
{
  NSString *title;
  NSString *directory;
  NSArray *types;
  NSDocumentController *controller;
  NSSavePanel *savePanel = [NSSavePanel savePanel];

  ASSIGN(_save_type, [self fileType]); 
  controller = [NSDocumentController sharedDocumentController];
  types = [self writableTypesForSaveOperation: saveOperation];

  if ([self shouldRunSavePanelWithAccessoryView])
    {
      if (_save_panel_accessory == nil)
        [self _createPanelAccessory];
      
      [self _addItemsToSpaButtonFromArray: types];
      
      [savePanel setAccessoryView: _save_panel_accessory];
    }

  if ([types count] > 0)
    {
      NSArray *extensions = [controller fileExtensionsFromType: [self fileType]];
      if ([extensions containsObject: @"*"])
	extensions = nil;
      [savePanel setAllowedFileTypes: extensions];
    }

  switch (saveOperation)
    {
    case NSSaveAsOperation: title = _(@"Save As"); break;
    case NSSaveToOperation: title = _(@"Save To"); break; 
    case NSSaveOperation: 
    default:
      title = _(@"Save");    
      break;
   }
  
  [savePanel setTitle: title];
  
  if ([self fileName])
    directory = [[self fileName] stringByDeletingLastPathComponent];
  else
    directory = [controller currentDirectory];
  [savePanel setDirectory: directory];

  if (!OVERRIDDEN(runModalSavePanel:withAccessoryView:))
    {
      if (![self prepareSavePanel: savePanel])
	{
	  return nil;
	}
    }
  if ([self runModalSavePanel: savePanel
	    withAccessoryView: [savePanel accessoryView]])
    {
      return savePanel;
    }

  return nil;
}

- (NSString *) fileNameFromRunningSavePanelForSaveOperation: (NSSaveOperationType)saveOperation
{
  NSSavePanel *savePanel = [self _runSavePanelForSaveOperation: saveOperation];

  if (savePanel)
    {
      return [savePanel filename];
    }
  
  return nil;
}

- (void) runModalSavePanelForSaveOperation: (NSSaveOperationType)saveOperation 
                                  delegate: (id)delegate
                           didSaveSelector: (SEL)didSaveSelector 
                               contextInfo: (void *)contextInfo
{
  // FIXME: Commit registered editors
  NSSavePanel *savePanel = [self _runSavePanelForSaveOperation: saveOperation];

  if (savePanel)
    {
      if (OVERRIDDEN(saveToFile:saveOperation:delegate:didSaveSelector:contextInfo:))
        {
          [self saveToFile: [savePanel filename] 
                saveOperation: saveOperation 
                delegate: delegate
                didSaveSelector: didSaveSelector 
                contextInfo: contextInfo];
        }
      else
        {
          [self saveToURL: [savePanel URL] 
                ofType: [self fileTypeFromLastRunSavePanel]
                forSaveOperation: saveOperation 
                delegate: delegate
                didSaveSelector: didSaveSelector 
                contextInfo: contextInfo];
        }
    }
}

- (NSArray *) writableTypesForSaveOperation: (NSSaveOperationType)op
{
  NSArray *types = [object_getClass(self) writableTypes];
  NSMutableArray *muTypes;
  int i, len;

  if (op == NSSaveToOperation)
    {
      return types;
    }

  len = [types count];
  muTypes = [NSMutableArray arrayWithCapacity: len];
  for (i = 0; i < len; i++)
    {
      NSString *type;
        
      type = [types objectAtIndex: i];
      if ([[self class] isNativeType: type])
        {
          [muTypes addObject: type];
        }
    }

  return muTypes;
}

- (BOOL) fileNameExtensionWasHiddenInLastRunSavePanel
{
  // FIXME
  return NO;
}

- (NSString *) fileNameExtensionForType: (NSString *)typeName
                          saveOperation: (NSSaveOperationType)saveOperation
{
  NSArray *exts = [[NSDocumentController sharedDocumentController]
                      fileExtensionsFromType: typeName];
  
  if ([exts count] && ![exts containsObject: @"*"])
    return (NSString *)[exts objectAtIndex: 0];

  return @"";
}

- (BOOL) shouldChangePrintInfo: (NSPrintInfo *)newPrintInfo
{
  return YES;
}

- (NSPrintInfo *) printInfo
{
  return _print_info? _print_info : [NSPrintInfo sharedPrintInfo];
}

- (void) setPrintInfo: (NSPrintInfo *)printInfo
{
  NSUndoManager *undoManager = [self undoManager];

  if (undoManager != nil)
    {
      [[undoManager prepareWithInvocationTarget: self]
	setPrintInfo: _print_info];
      // FIXME undoManager -setActionName:
    }
  ASSIGN(_print_info, printInfo);
  if (undoManager == nil)
    {
      [self updateChangeCount: NSChangeDone];
    }
}


// Page layout panel (Page Setup)
- (BOOL) preparePageLayout: (NSPageLayout *)pageLayout
{
  return YES;
}

- (NSInteger) runModalPageLayoutWithPrintInfo: (NSPrintInfo *)printInfo
{
  NSPageLayout *pageLayout;

  pageLayout = [NSPageLayout pageLayout];
  if ([self preparePageLayout: pageLayout])
    {
      return [pageLayout runModalWithPrintInfo: printInfo];
    }
  else
    {
      return NSCancelButton;
    }
}

- (void) _documentDidRunModalPageLayout: (NSDocument*)doc
                               accepted: (BOOL)accepted
                            contextInfo: (void *)contextInfo
{
  NSPrintInfo *printInfo = (NSPrintInfo *)contextInfo;

  if (accepted && [self shouldChangePrintInfo: printInfo])
    {
      [self setPrintInfo: printInfo];
    }
}

- (IBAction) runPageLayout: (id)sender
{
  NSPrintInfo *printInfo = AUTORELEASE([[self printInfo] copy]);

  [self runModalPageLayoutWithPrintInfo: printInfo
        delegate: self
        didRunSelector: @selector(_documentDidRunModalPageLayout:accepted:contextInfo:)
        contextInfo: printInfo];
}

- (void) runModalPageLayoutWithPrintInfo: (NSPrintInfo *)info
                                delegate: (id)delegate
                          didRunSelector: (SEL)sel
                             contextInfo: (void *)context
{
  NSPageLayout *pageLayout;

  pageLayout = [NSPageLayout pageLayout];
  if ([self preparePageLayout: pageLayout])
    {
      [pageLayout beginSheetWithPrintInfo: info
                  modalForWindow: [self windowForSheet]
                  delegate: delegate
                  didEndSelector: sel
                  contextInfo: context];
    }
}

/* This is overridden by subclassers; the default implementation does nothing. */
- (void) printShowingPrintPanel: (BOOL)flag
{
}

- (IBAction) printDocument: (id)sender
{
  [self printDocumentWithSettings: [NSDictionary dictionary]
        showPrintPanel: YES
        delegate: nil
        didPrintSelector: NULL
        contextInfo: NULL];
}

- (void) printDocumentWithSettings: (NSDictionary *)settings
                    showPrintPanel: (BOOL)flag
                          delegate: (id)delegate
                  didPrintSelector: (SEL)sel
                       contextInfo: (void *)context
{
  NSPrintOperation *printOp;
  NSError *error;

  if (OVERRIDDEN(printShowingPrintPanel:))
    {
      // FIXME: More communication with the panel is needed.
      return [self printShowingPrintPanel: flag];
    }

  printOp = [self printOperationWithSettings: settings
                  error: &error];
  if (printOp != nil)
    {
      [printOp setShowsPrintPanel: flag];
      [self runModalPrintOperation: printOp
            delegate: delegate
            didRunSelector: sel
            contextInfo: context];
    }
  else
    {
      [self presentError: error];

      if (delegate != nil && sel != NULL)
        {
          void (*meth)(id, SEL, id, BOOL, void*);
          meth = (void (*)(id, SEL, id, BOOL, void*))[delegate methodForSelector: sel];
          if (meth)
              meth(delegate, sel, self, NO, context);
        }
    }
}

- (NSPrintOperation *) printOperationWithSettings: (NSDictionary *)settings
                                            error: (NSError **)error
{
  if (error)
    *error = nil; 
  return nil;
}

- (void) runModalPrintOperation: (NSPrintOperation *)op
                       delegate: (id)delegate
                 didRunSelector: (SEL)sel
                    contextInfo: (void *)context
{
  ASSIGN(_printOp_delegate, delegate);
  _printOp_didRunSelector = sel;
  [op runOperationModalForWindow: [self windowForSheet]
      delegate: self 
      didRunSelector: @selector(_runModalPrintOperationDidSucceed:contextInfo:)
      contextInfo: context];
}

- (void) _runModalPrintOperationDidSucceed: (BOOL)success
                               contextInfo: (void *)context
{
  id delegate = _printOp_delegate;
  SEL didRunSelector = _printOp_didRunSelector;
  void (*didRun)(id, SEL, NSDocument *, BOOL, id);

  if (delegate && [delegate respondsToSelector: didRunSelector])
    {
      didRun = (void (*)(id, SEL, NSDocument *, BOOL, id))
          [delegate methodForSelector: didRunSelector];
      didRun(delegate, didRunSelector, self, success, context);
    }
  DESTROY(_printOp_delegate);
}

- (BOOL) validateMenuItem: (NSMenuItem *)anItem
{
  return [self validateUserInterfaceItem: anItem];
}

- (BOOL) validateUserInterfaceItem: (id <NSValidatedUserInterfaceItem>)anItem
{
  if (sel_isEqual([anItem action], @selector(revertDocumentToSaved:)))
    return ([self fileName] != nil && [self isDocumentEdited]);
  if (sel_isEqual([anItem action], @selector(saveDocument:)))
    return [self isDocumentEdited];

  // FIXME should validate spa popup items; return YES if it's a native type.

  return YES;
}

- (NSString *) fileTypeFromLastRunSavePanel
{
  return _save_type;
}

- (NSDictionary *) fileAttributesToWriteToFile: (NSString *)fullDocumentPath 
                                        ofType: (NSString *)docType 
                                 saveOperation: (NSSaveOperationType)saveOperationType
{
  // FIXME: Implement. Should set NSFileExtensionHidden
  return [NSDictionary dictionary];
}

- (NSDictionary *) fileAttributesToWriteToURL: (NSURL *)url
                                       ofType: (NSString *)type
                             forSaveOperation: (NSSaveOperationType)op
                          originalContentsURL: (NSURL *)original
                                        error: (NSError **)error
{
  // FIXME: Implement. Should set NSFileExtensionHidden
  if (error)
    *error = nil; 

  return [NSDictionary dictionary];
}

- (IBAction) saveDocument: (id)sender
{
  [self saveDocumentWithDelegate: nil
        didSaveSelector: NULL
        contextInfo: NULL];
}

- (IBAction) saveDocumentAs: (id)sender
{
  [self runModalSavePanelForSaveOperation: NSSaveAsOperation 
        delegate: nil
        didSaveSelector: NULL 
        contextInfo: NULL];
}

- (IBAction) saveDocumentTo: (id)sender
{
  [self runModalSavePanelForSaveOperation: NSSaveToOperation 
        delegate: nil
        didSaveSelector: NULL 
        contextInfo: NULL];
}

- (void) saveDocumentWithDelegate: (id)delegate 
                  didSaveSelector: (SEL)didSaveSelector 
                      contextInfo: (void *)contextInfo
{
  NSURL *fileURL = [self fileURL];
  NSString *type = [self fileType];

  if ((fileURL != nil) && (type != nil))
    {
      [self saveToURL: fileURL
            ofType: type
            forSaveOperation: NSSaveOperation
            delegate: delegate
            didSaveSelector: didSaveSelector 
            contextInfo: contextInfo];
    }
  else
    {
      [self runModalSavePanelForSaveOperation: NSSaveOperation 
            delegate: delegate
            didSaveSelector: didSaveSelector 
            contextInfo: contextInfo];
    }
}

- (void) saveToFile: (NSString *)fileName 
      saveOperation: (NSSaveOperationType)saveOperation 
           delegate: (id)delegate
    didSaveSelector: (SEL)didSaveSelector 
        contextInfo: (void *)contextInfo
{
  BOOL saved = NO;
 
  if (fileName != nil)
    {
      saved = [self writeWithBackupToFile: fileName 
                    ofType: [self fileTypeFromLastRunSavePanel]
                    saveOperation: saveOperation];
      if (saved &&
	  (saveOperation == NSSaveOperation ||
	   saveOperation == NSSaveAsOperation))
	{
	  [[NSDocumentController sharedDocumentController]
	    noteNewRecentDocument: self];
	}
    }

  if (delegate != nil && didSaveSelector != NULL)
    {
      void (*meth)(id, SEL, id, BOOL, void*);
      meth = (void (*)(id, SEL, id, BOOL, void*))[delegate methodForSelector: 
                                                               didSaveSelector];
      if (meth)
        meth(delegate, didSaveSelector, self, saved, contextInfo);
    }
}

- (BOOL) saveToURL: (NSURL *)url
            ofType: (NSString *)type
  forSaveOperation: (NSSaveOperationType)op
             error: (NSError **)error
{
  BOOL saved =
    [self writeSafelyToURL: url
	  ofType: type
          forSaveOperation: op
          error: error];
  if (saved && (op == NSSaveOperation || op == NSSaveAsOperation))
    {
      [[NSDocumentController sharedDocumentController]
	noteNewRecentDocument: self];
    }
  return saved;
}

- (void) saveToURL: (NSURL *)url
            ofType: (NSString *)type
  forSaveOperation: (NSSaveOperationType)op
          delegate: (id)delegate
   didSaveSelector: (SEL)didSaveSelector 
       contextInfo: (void *)contextInfo
{
  NSError *error;
  BOOL saved;

  saved = [self saveToURL: url
                ofType: type
                forSaveOperation: op
                error: &error];
  if (!saved)
    {
      [self presentError: error]; 
    }
  else if (op == NSSaveOperation || op == NSSaveAsOperation)
    {
      [[NSDocumentController sharedDocumentController]
	noteNewRecentDocument: self];
    }

  if (delegate != nil && didSaveSelector != NULL)
    {
      void (*meth)(id, SEL, id, BOOL, void*);
      meth = (void (*)(id, SEL, id, BOOL, void*))[delegate methodForSelector: 
                                                               didSaveSelector];
      if (meth)
        meth(delegate, didSaveSelector, self, saved, contextInfo);
    }
}

- (IBAction) revertDocumentToSaved: (id)sender
{
  int result;
  NSError *error;

  result = NSRunAlertPanel 
    (_(@"Revert"),
     _(@"%@ has been edited.  Are you sure you want to undo changes?"),
     _(@"Revert"), _(@"Cancel"), nil, 
     [self displayName]);
  
  if (result == NSAlertDefaultReturn)
    {
      // FIXME: Revert registered editors
      if ([self revertToContentsOfURL: [self fileURL] 
                ofType: [self fileType]
                error: &error])
        {
          [self updateChangeCount: NSChangeCleared];
          [[self undoManager] removeAllActions];
          [self _removeAutosavedContentsFile];
        }
      else
        {
          [self presentError: error];
        }
    }
}

/** Closes all the windows owned by the document, then removes itself
    from the list of documents known by the NSDocumentController. This
    method does not ask the user if they want to save the document before
    closing. It is closed without saving any information.
 */
- (void) close
{
  if (_doc_flags.in_close == NO)
    {
      int count = [_window_controllers count];
      /* Closing a windowController will also send us a close, so make
         sure we don't go recursive */
      _doc_flags.in_close = YES;

      if (count > 0)
        {
          NSWindowController *array[count];
          [_window_controllers getObjects: array];
          while (count-- > 0)
            [array[count] close];
        }
      [self _removeAutosavedContentsFile];
      AUTORELEASE(RETAIN(self));
      [[NSDocumentController sharedDocumentController] removeDocument: self];
    }
}

- (void) windowControllerWillLoadNib: (NSWindowController *)windowController {}
- (void) windowControllerDidLoadNib: (NSWindowController *)windowController  {}

- (NSUndoManager *) undoManager
{
  if (_undo_manager == nil && [self hasUndoManager])
    {
      [self setUndoManager: AUTORELEASE([[NSUndoManager alloc] init])];
    }
  
  return _undo_manager;
}

- (void) setUndoManager: (NSUndoManager *)undoManager
{
  if (undoManager != _undo_manager)
    {
      NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
      
      if (_undo_manager)
        {
          [center removeObserver: self
                  name: NSUndoManagerWillCloseUndoGroupNotification
                  object: _undo_manager];
          [center removeObserver: self
                  name: NSUndoManagerDidUndoChangeNotification
                  object: _undo_manager];
          [center removeObserver: self
                  name: NSUndoManagerDidRedoChangeNotification
                  object: _undo_manager];
        }
      
      ASSIGN(_undo_manager, undoManager);
      
      if (_undo_manager == nil)
        {
          [self setHasUndoManager: NO];
        }
      else
        {
          [center addObserver: self
                  selector:@selector(_changeWasDone:)
                  name: NSUndoManagerWillCloseUndoGroupNotification
                  object: _undo_manager];
          [center addObserver: self
                  selector:@selector(_changeWasUndone:)
                  name: NSUndoManagerDidUndoChangeNotification
                  object: _undo_manager];
          [center addObserver: self
            selector:@selector(_changeWasRedone:)
            name: NSUndoManagerDidRedoChangeNotification
            object: _undo_manager];
        }
    }
}

- (BOOL) hasUndoManager
{
  return _doc_flags.has_undo_manager;
}

- (void) setHasUndoManager: (BOOL)flag
{
  if (_undo_manager && !flag)
    [self setUndoManager: nil];
  
  _doc_flags.has_undo_manager = flag;
}

- (BOOL) presentError: (NSError *)error
{
  error = [self willPresentError: error];
  return [[NSDocumentController sharedDocumentController] presentError: error];
}

- (void) presentError: (NSError *)error
       modalForWindow: (NSWindow *)window
             delegate: (id)delegate
   didPresentSelector: (SEL)sel
          contextInfo: (void *)context
{
  error = [self willPresentError: error];
  [[NSDocumentController sharedDocumentController] presentError: error
                                                   modalForWindow: window
                                                   delegate: delegate
                                                   didPresentSelector: sel
                                                   contextInfo: context];
}

- (NSError *) willPresentError: (NSError *)error
{
  return error;
}

- (NSURL *) autosavedContentsFileURL
{
  return _autosaved_file_url;
}

- (void) setAutosavedContentsFileURL: (NSURL *)url
{
  ASSIGN(_autosaved_file_url, url);
  [[NSDocumentController sharedDocumentController]
      _recordAutosavedDocument: self];
}

- (void) autosaveDocumentWithDelegate: (id)delegate
                  didAutosaveSelector: (SEL)didAutosaveSelector
                          contextInfo: (void *)context
{
  NSURL *url = [self autosavedContentsFileURL];
  NSString *type = [self autosavingFileType];

  if (url == nil)
    {
      static NSString *processName = nil;
      NSString *path;
      NSString *ext = [self fileNameExtensionForType: type 
                            saveOperation: NSAutosaveOperation];
      
      if (!processName)
        processName = [[[NSProcessInfo processInfo] processName] copy];

      path = [[NSDocumentController sharedDocumentController]
                 _autosaveDirectory: YES];
      path = [path stringByAppendingPathComponent:
                       [NSString stringWithFormat: @"%@-%d",
                                 processName, _document_index]];
      path = [path stringByAppendingPathExtension: ext];
      url = [NSURL fileURLWithPath: path];
    }

  [self saveToURL: url
        ofType: type
        forSaveOperation: NSAutosaveOperation
        delegate: delegate
        didSaveSelector: didAutosaveSelector 
        contextInfo: context];
}

- (NSString *) autosavingFileType
{
  return [self fileType];
}

- (BOOL) hasUnautosavedChanges
{
  return _autosave_change_count != 0 || _doc_flags.autosave_permanently_modified;
}

@end

@implementation NSDocument(Private)

/*
 * This private method is used to transfer window ownership to the
 * NSWindowController in situations (such as the default) where the
 * document is set to the nib owner, and thus owns the window immediately
 * following the loading of the nib.
 */
- (NSWindow *) _transferWindowOwnership
{
  NSWindow *window = _window;
  _window = nil;
  return AUTORELEASE(window);
}

- (void) _removeWindowController: (NSWindowController *)windowController
{
  if ([_window_controllers containsObject: windowController])
    {
      BOOL autoClose = [windowController shouldCloseDocument];
      
      [windowController setDocument: nil];
      [_window_controllers removeObject: windowController];
      
      if (autoClose || [_window_controllers count] == 0)
        {
          [self close];
        }
    }
}

- (void) _removeAutosavedContentsFile
{
  NSURL *url = [self autosavedContentsFileURL];

  if (url)
    {
      NSString *path = [[url path] retain];

      [self setAutosavedContentsFileURL: nil];
      [[NSFileManager defaultManager] removeFileAtPath: path handler: nil];
      [path release];
    }
}

- (void) _changeWasDone: (NSNotification *)notification
{
  /* Prevent a document from appearing unmodified after saving the
   * document, undoing a number of changes, and then making an equal
   * number of changes.  Ditto for autosaved changes.
   */
  if (_change_count < 0)
    _doc_flags.permanently_modified = 1;
  if (_autosave_change_count < 0)
    _doc_flags.autosave_permanently_modified = 1;
  [self updateChangeCount: NSChangeDone];
}

- (void) _changeWasUndone: (NSNotification *)notification
{
  [self updateChangeCount: NSChangeUndone];
}

- (void) _changeWasRedone: (NSNotification *)notification
{
  /* FIXME
   * Mac OS X 10.5 uses a new constant NSChangeRedone here, but
   * old applications are not prepared to handle this constant
   * and expect NSChangeDone instead.
   */
  [self updateChangeCount: NSChangeDone];
}

@end
