/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSSavePanel.h>

@interface NSOpenPanel : NSSavePanel {
    NSArray *_filenames;
    BOOL _allowsMultipleSelection;
    BOOL _canChooseDirectories;
    BOOL _canChooseFiles;
    BOOL _resolvesAliases;
}

+ (NSOpenPanel *)openPanel;

- (NSArray *)filenames;
- (NSArray *)URLs;

- (int)runModalForDirectory:(NSString *)directory file:(NSString *)file types:(NSArray *)types;
- (int)runModalForTypes:(NSArray *)types;

- (BOOL)allowsMultipleSelection;
- (BOOL)canChooseDirectories;
- (BOOL)canChooseFiles;
- (BOOL)resolvesAliases;

- (void)setAllowsMultipleSelection:(BOOL)flag;
- (void)setCanChooseDirectories:(BOOL)flag;
- (void)setCanChooseFiles:(BOOL)flag;
- (void)setResolvesAliases:(BOOL)value;

- (void)beginSheetForDirectory:(NSString *)path
                          file:(NSString *)name
                         types:(NSArray *)fileTypes
                modalForWindow:(NSWindow *)docWindow
                 modalDelegate:(id)modalDelegate
                didEndSelector:(SEL)didEndSelector
                   contextInfo:(void *)contextInfo;

@end
