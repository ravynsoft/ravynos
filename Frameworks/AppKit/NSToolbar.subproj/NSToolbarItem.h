/* Copyright (c) 2006-2009 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>
#import <AppKit/AppKitExport.h>
#import <AppKit/NSUserInterfaceValidation.h>

@class NSString, NSImage, NSView, NSMenuItem, NSToolbar, NSToolbarItemView;

enum {
    NSToolbarItemVisibilityPriorityStandard = 0,
    NSToolbarItemVisibilityPriorityLow = -1000,
    NSToolbarItemVisibilityPriorityHigh = 1000,
    NSToolbarItemVisibilityPriorityUser = 2000
};

APPKIT_EXPORT NSString *const NSToolbarCustomizeToolbarItemIdentifier;
APPKIT_EXPORT NSString *const NSToolbarFlexibleSpaceItemIdentifier;
APPKIT_EXPORT NSString *const NSToolbarPrintItemIdentifier;
APPKIT_EXPORT NSString *const NSToolbarSeparatorItemIdentifier;
APPKIT_EXPORT NSString *const NSToolbarShowColorsItemIdentifier;
APPKIT_EXPORT NSString *const NSToolbarShowFontsItemIdentifier;
APPKIT_EXPORT NSString *const NSToolbarSpaceItemIdentifier;

@interface NSToolbarItem : NSObject <NSCopying, NSValidatedUserInterfaceItem> {
    NSString *_itemIdentifier;
    NSToolbar *_toolbar;
    NSToolbarItemView *_enclosingView;
    NSString *_toolTip;
    NSInteger _tag;
    NSImage *_image;
    NSString *_label;
    NSString *_paletteLabel;
    id _target;
    SEL _action;

    NSMenuItem *_menuFormRepresentation;

    NSView *_view;

    NSSize _minSize;
    NSSize _maxSize;
    NSInteger _visibilityPriority;
    BOOL _autovalidates;
    BOOL _isEnabled;
}

- initWithItemIdentifier:(NSString *)identifier;

- (NSString *)itemIdentifier;
- (NSToolbar *)toolbar;

- (NSString *)label;
- (NSString *)paletteLabel;
- (NSMenuItem *)menuFormRepresentation;
- (NSView *)view;
- (NSSize)minSize;
- (NSSize)maxSize;
- (NSInteger)visibilityPriority;
- (BOOL)autovalidates;
- (BOOL)allowsDuplicatesInToolbar;

- (void)setLabel:(NSString *)label;
- (void)setPaletteLabel:(NSString *)label;
- (void)setMenuFormRepresentation:(NSMenuItem *)menuItem;
- (void)setView:(NSView *)view;
- (void)setMinSize:(NSSize)size;
- (void)setMaxSize:(NSSize)size;
- (void)setVisibilityPriority:(NSInteger)value;
- (void)setAutovalidates:(BOOL)value;

- (NSImage *)image;
- target;
- (SEL)action;
- (NSInteger)tag;
- (BOOL)isEnabled;
- (NSString *)toolTip;

- (void)setImage:(NSImage *)image;
- (void)setTarget:target;
- (void)setAction:(SEL)action;
- (void)setTag:(NSInteger)tag;
- (void)setEnabled:(BOOL)enabled;
- (void)setToolTip:(NSString *)tip;

- (void)validate;

@end

@interface NSObject (NSToolbarItem_validation)
- (BOOL)validateToolbarItem:(NSToolbarItem *)item;
@end
