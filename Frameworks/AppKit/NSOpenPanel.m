/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSOpenPanel.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSDisplay.h>
#import <Foundation/NSURL.h>

@interface NSSavePanel (Private) 
-(void)_setFilename:(NSString*)filename;
@end

@implementation NSOpenPanel

+(NSOpenPanel *)openPanel {
   return [[self new] autorelease];
}

-init {
   [super init];
   _filenames=[NSArray new];
   [_dialogTitle release];
   _dialogTitle= [NSLocalizedStringFromTableInBundle(@"Open", nil, [NSBundle bundleForClass: [NSOpenPanel class]], @"The title of the open panel") copy];
   _allowsMultipleSelection=NO;
   _canChooseDirectories=NO;
   _canChooseFiles=YES;
   _resolvesAliases=YES;
   return self;
}

-(void)dealloc {
   [_filenames release];
   [super dealloc];
}

-(NSArray *)filenames {
   id ret=nil;
   @synchronized(self)
   {
      ret=[[_filenames copy] autorelease];
   }
   return ret;
}

-(NSArray *)URLs {
   NSArray        *paths=[self filenames];
   NSMutableArray *result=[NSMutableArray arrayWithCapacity:[paths count]];
   int             i,count=[paths count];
   
   for(i=0;i<count;i++)
    [result addObject:[NSURL fileURLWithPath:[paths objectAtIndex:i]]];
    
   return result;
}

-(int)runModalForDirectory:(NSString *)directory file:(NSString *)file types:(NSArray *)types {
   [self _setFilename:file];
   [self setDirectory:directory];
   return [[NSDisplay currentDisplay] openPanel:self runModalForDirectory:directory file:file types:types];
}

-(int)runModalForTypes:(NSArray *)types {
   return [self runModalForDirectory:[self directory] file:nil types:types];
}

-(int)runModalForDirectory:(NSString *)directory file:(NSString *)file {
   return [self runModalForDirectory:directory file:file types:nil];
}

-(int)runModal {
   return [self runModalForDirectory:nil file:nil types:nil];
}

-(BOOL)allowsMultipleSelection {
   return _allowsMultipleSelection;
}

-(BOOL)canChooseDirectories {
   return _canChooseDirectories;
}

-(BOOL)canChooseFiles {
   return _canChooseFiles;
}

-(BOOL)resolvesAliases {
   return _resolvesAliases;
}

-(void)setAllowsMultipleSelection:(BOOL)flag {
   _allowsMultipleSelection=flag;
}

-(void)setCanChooseDirectories:(BOOL)flag {
   _canChooseDirectories=flag;
}

-(void)setCanChooseFiles:(BOOL)flag {
   _canChooseFiles=flag;
}

-(void)setResolvesAliases:(BOOL)value {
   _resolvesAliases=value;
}

#pragma mark -
#pragma mark Sheet methods
- (void)beginSheetForDirectory:(NSString *)path
                          file:(NSString *)name 
                         types:(NSArray *)fileTypes
                modalForWindow:(NSWindow *)docWindow
                 modalDelegate:(id)modalDelegate 
                didEndSelector:(SEL)didEndSelector 
                   contextInfo:(void *)contextInfo
{
	id inv=[NSInvocation invocationWithMethodSignature:[self methodSignatureForSelector:@selector(_background_beginSheetForDirectory:file:types:modalForWindow:modalDelegate:didEndSelector:contextInfo:)]];
	[inv setTarget:self];
	[inv setSelector:@selector(_background_beginSheetForDirectory:file:types:modalForWindow:modalDelegate:didEndSelector:contextInfo:)];
	[inv setArgument:&path atIndex:2];
	[inv setArgument:&name atIndex:3];
   [inv setArgument:&fileTypes atIndex:4];
	[inv setArgument:&docWindow atIndex:5];
	[inv setArgument:&modalDelegate atIndex:6];
	[inv setArgument:&didEndSelector atIndex:7];
	[inv setArgument:&contextInfo atIndex:8];
	[inv retainArguments];
	[inv performSelectorInBackground:@selector(invoke) withObject:nil];
}

- (void)_background_beginSheetForDirectory:(NSString *)path
                                      file:(NSString *)name 
                                     types:(NSArray *)fileTypes
                            modalForWindow:(NSWindow *)docWindow
                             modalDelegate:(id)modalDelegate 
                            didEndSelector:(SEL)didEndSelector 
                               contextInfo:(void *)contextInfo
{
	id pool=[NSAutoreleasePool new];
	int ret=[self runModalForDirectory:path file:name types:fileTypes];
	
	id inv=[NSInvocation invocationWithMethodSignature:[self methodSignatureForSelector:@selector(_selector_savePanelDidEnd:returnCode:contextInfo:)]];
	
	[inv setTarget:modalDelegate];
	[inv setSelector:didEndSelector];
	[inv setArgument:&self atIndex:2];
	[inv setArgument:&ret atIndex:3];
	[inv setArgument:&contextInfo atIndex:4];
	[inv retainArguments];
	
	[inv performSelectorOnMainThread:@selector(invoke) withObject:nil waitUntilDone:NO];
   [pool release];
}

@end
