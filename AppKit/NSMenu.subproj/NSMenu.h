/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/Foundation.h>

@class NSScreen, NSMenu, NSMenuItem, NSWindow, NSEvent, NSView;

@protocol NSMenuDelegate;

@interface NSMenu : NSObject <NSCopying> {
    NSMenu *_supermenu;
    NSString *_title;
    NSString *_name;
    NSMutableArray *_itemArray;
    BOOL _autoenablesItems;
    id<NSMenuDelegate> _delegate;
    int _DBusItemID;
}

+ (void)popUpContextMenu:(NSMenu *)menu withEvent:(NSEvent *)event forView:(NSView *)view;

- initWithTitle:(NSString *)title;

- (NSMenu *)supermenu;
- (NSString *)title;
- (int)numberOfItems;
- (NSArray *)itemArray;
- (BOOL)autoenablesItems;

- (NSMenuItem *)itemAtIndex:(int)index;
- (NSMenuItem *)itemWithTag:(int)tag;
- (NSMenuItem *)itemWithTitle:(NSString *)title;

- (int)indexOfItem:(NSMenuItem *)item;
- (int)indexOfItemWithTag:(int)tag;
- (int)indexOfItemWithTitle:(NSString *)title;
- (int)indexOfItemWithRepresentedObject:object;
- (int)indexOfItemWithTarget:(id)target andAction:(SEL)action;
- (int)indexOfItemWithSubmenu:(NSMenu *)menu;

- (void)setSupermenu:(NSMenu *)value;
- (void)setTitle:(NSString *)title;
- (void)setAutoenablesItems:(BOOL)flag;

- (void)addItem:(NSMenuItem *)item;
- (NSMenuItem *)addItemWithTitle:(NSString *)title action:(SEL)action keyEquivalent:(NSString *)keyEquivalent;

- (void)removeAllItems; // private, don't use outside framework
- (void)removeItem:(NSMenuItem *)item;
- (void)removeItemAtIndex:(int)index;

- (void)insertItem:(NSMenuItem *)item atIndex:(int)index;
- (NSMenuItem *)insertItemWithTitle:(NSString *)title action:(SEL)action keyEquivalent:(NSString *)keyEquivalent atIndex:(int)index;

- (void)setSubmenu:(NSMenu *)submenu forItem:(NSMenuItem *)item;

- (void)update;

- (void)itemChanged:(NSMenuItem *)item;

- (BOOL)performKeyEquivalent:(NSEvent *)event;

- (void)setDelegate:(id<NSMenuDelegate>)object;
- (id<NSMenuDelegate>)delegate;

@end

@interface NSObject (NSMenu_validateItem)
- (BOOL)validateMenuItem:(NSMenuItem *)item;
@end

@protocol NSMenuDelegate <NSObject>

@optional

- (NSRect)confinementRectForMenu:(NSMenu *)menu onScreen:(NSScreen *)screen;
- (BOOL)menuHasKeyEquivalent:(NSMenu *)menu forEvent:(NSEvent *)event target:(id *)target action:(SEL *)action;
- (void)menuNeedsUpdate:(NSMenu *)menu;
- (void)menuWillOpen:(NSMenu *)menu;
- (void)menuDidClose:(NSMenu *)menu;
- (NSInteger)numberOfItemsInMenu:(NSMenu *)menu;
- (void)menu:(NSMenu *)menu willHighlightItem:(NSMenuItem *)item;
- (BOOL)menu:(NSMenu *)menu updateItem:(NSMenuItem *)item atIndex:(NSInteger)index shouldCancel:(BOOL)shouldCancel;

@end
