/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - David Young <daver@geeks.org>
#import <AppKit/AppKit.h>
#import <AppKit/NSColorPickerColorList.h>
#import <AppKit/NSBrowserCellColorList.h>

@implementation NSColorPickerColorList

-initWithPickerMask:(NSUInteger)mask colorPanel:(NSColorPanel *)colorPanel {
    NSEnumerator *colorListsEnumerator = [[NSColorList availableColorLists] objectEnumerator];
    NSColorList *colorList;

    [super initWithPickerMask:mask colorPanel:colorPanel];
    [colorBrowser setCellClass:[NSBrowserCellColorList class]];

    [colorListPopUp removeAllItems];
    while ((colorList = [colorListsEnumerator nextObject])!=nil)
        [self attachColorList:colorList];

    [self colorListPopUpClicked:colorListPopUp];

    return self;
}

- (void)attachColorList:(NSColorList *)colorList
{
    [colorListPopUp addItemWithTitle:[colorList name]];
}

- (void)colorListPopUpClicked:(id)sender
{
    [_pickedColorList release];
    _pickedColorList = [[NSColorList colorListNamed:[[sender itemAtIndex:[sender indexOfSelectedItem]] title]] retain];

    [colorBrowser reloadColumn:0];
}

- (void)colorListBrowserClicked:(id)sender
{
    [[NSColorPanel sharedColorPanel] setColor:[_pickedColorList colorWithKey:[[sender selectedCell] stringValue]]];
}

- (NSInteger)browser:(NSBrowser *)sender numberOfRowsInColumn:(NSInteger)column
{
    return [[_pickedColorList allKeys] count];
}

- (NSImage *)provideNewButtonImage
{
    return [NSImage imageNamed:@"NSColorPickerListIcon"];
}

- (void)browser:(NSBrowser *)sender willDisplayCell:(id)cell atRow:(NSInteger)row column:(NSInteger)column
{
    [cell setStringValue:[[_pickedColorList allKeys] objectAtIndex:row]];
    [cell setLeaf:YES];
    [cell setLoaded:YES];
    [cell setColor:[_pickedColorList colorWithKey:[[_pickedColorList allKeys] objectAtIndex:row]]];
}

@end
