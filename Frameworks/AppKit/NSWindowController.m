/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSWindowController.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSNibLoading.h>
#import <AppKit/NSDocument.h>
#import <AppKit/NSNib.h>
#import <AppKit/NSApplication.h>

@implementation NSWindowController

-initWithWindow:(NSWindow *)window {
   _window=[window retain];
   [_window setWindowController:self];
   [_window setReleasedWhenClosed:NO];
   
   if(_window!=nil)
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_windowWillClose:) name:NSWindowWillCloseNotification object:_window];

   _nibName=nil;
   _nibPath=nil;
   _owner=self;
   _document=nil;
   _shouldCloseDocument=NO;
   _shouldCascadeWindows=YES;
   _windowFrameAutosaveName=nil;
   return self;
}

-initWithWindowNibName:(NSString *)nibName {
   return [self initWithWindowNibName:nibName owner:self];
}

-initWithWindowNibName:(NSString *)nibName owner:owner {
   [self initWithWindow:nil];
   _nibName=[nibName copy];
   _nibPath=nil;
   _owner=owner;
   return self;
}

-initWithWindowNibPath:(NSString *)nibPath owner:owner {
   [self initWithWindow:nil];
   _nibName=[[[nibPath lastPathComponent] stringByDeletingPathExtension] copy];
   _nibPath=[nibPath copy];
   _owner=owner;
   return self;
}

-init {
   return [self initWithWindow:nil];
}

-(void)dealloc {
   [[NSNotificationCenter defaultCenter] removeObserver:self];
   [_window setWindowController:nil];
   [_window release];
   _window=nil;
   [_nibName release];
   [_nibPath release];
   [_windowFrameAutosaveName release];
	[_topLevelObjects makeObjectsPerformSelector:@selector(release)];
   [_topLevelObjects release];
   [super dealloc];
}

-(NSWindow *)window {
   if(_window==nil && [self windowNibPath]!=nil){
    [self windowWillLoad];
    [_document windowControllerWillLoadNib:self];

    [self loadWindow];

    [self windowDidLoad];
    [_document windowControllerDidLoadNib:self];
   }

   return _window;
}

-(void)setWindow:(NSWindow *)window {
   NSNotificationCenter *center=[NSNotificationCenter defaultCenter];
   
   
   if (_window)
    [center removeObserver:self name:NSWindowWillCloseNotification object:_window];
    
   window=[window retain];

   [_window setWindowController:nil];
   [_window release];

   _window=window;
   
   [_window setWindowController:self];
   [_window setReleasedWhenClosed:NO];
   
   if (_window)
    [center addObserver:self selector:@selector(_windowWillClose:) name:NSWindowWillCloseNotification object:_window];
}

-(void)_windowWillClose:(NSNotification *)note {
	[self setWindow:nil];

	if (_document){
   [[self retain] autorelease];

   if([self shouldCloseDocument] || [[_document windowControllers] count]==1)
    [_document close];
   else {
    [_document removeWindowController:self];
   }
  }
}

-(BOOL)isWindowLoaded {
   return (_window!=nil)?YES:NO;
}

-(void)loadWindow {
   if(![self isWindowLoaded]){
    static       NSPoint cascadeTopLeftSavedPoint={0.0, 0.0};
    NSString     *path=[self windowNibPath];
    NSDictionary *nameTable;
   
    _topLevelObjects = [[NSMutableArray alloc] init];
    nameTable=[NSDictionary dictionaryWithObjectsAndKeys:_owner, NSNibOwner, _topLevelObjects, NSNibTopLevelObjects, nil];

    if(![NSBundle loadNibFile:path externalNameTable:nameTable withZone:NULL]){
     NSLog(@"%s: unable to load nib from file '%@'", __PRETTY_FUNCTION__, path);
    }

    [self synchronizeWindowTitleWithDocumentName];
   
    if (_shouldCascadeWindows)
       cascadeTopLeftSavedPoint=[_window cascadeTopLeftFromPoint:cascadeTopLeftSavedPoint];
   }
}

-(void)windowWillLoad {
  // do nothing
}

-(void)windowDidLoad {
  // do nothing
}

-(void)showWindow:sender {
   [[self window] makeKeyAndOrderFront:sender];
}

-(void)setDocument:(NSDocument *)document {
   _document=document;
}

-(id)document {
   return _document;
}

-(void)setDocumentEdited:(BOOL)flag {
   [_window setDocumentEdited:flag];
}

-(void)close {
   [_window close];
}

-(BOOL)shouldCloseDocument {
   return _shouldCloseDocument;
}

-(void)setShouldCloseDocument:(BOOL)flag {
   _shouldCloseDocument=flag;
}

-owner {
   return _owner;
}

-(NSString *)windowNibName {
   return _nibName;
}

-(NSString *)windowNibPath {
   if(_nibPath!=nil)
    return _nibPath;
   else {
    NSString *name=[self windowNibName];
    NSBundle *bundle=[NSBundle bundleForClass:[_owner class]];
    NSString *path=[bundle pathForResource:name ofType:@"nib"];

    if(path==nil)
     path=[[NSBundle mainBundle] pathForResource:name ofType:@"nib"];

    return path;
   }
}

-(void)setShouldCascadeWindows:(BOOL)flag {
   _shouldCascadeWindows=flag;
}

-(BOOL)shouldCascadeWindows {
   return _shouldCascadeWindows;
}

-(void)setWindowFrameAutosaveName:(NSString *)name {
   name=[name copy];
   [_windowFrameAutosaveName release];
   _windowFrameAutosaveName=name;
}

-(NSString *)windowFrameAutosaveName {
   return _windowFrameAutosaveName;
}

-(void)synchronizeWindowTitleWithDocumentName {
   if(_document!=nil && _window!=nil){
    NSString *displayName=[_document displayName];
    NSString *title=[self windowTitleForDocumentDisplayName:displayName];
    NSString *path=[_document fileName];

    [_window setTitle:title];
   }
}

-(NSString *)windowTitleForDocumentDisplayName:(NSString *)displayName {
  NSString *appName = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleName"]; 
  if (appName)
    return [NSString stringWithFormat:@"%@ - %@", displayName, appName];
  else
    return displayName;
}

@end
