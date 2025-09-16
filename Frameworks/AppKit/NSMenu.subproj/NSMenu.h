/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */


/* Modified by Mr. Walls in 2025 - This version is a derivative Contributed under MIT */
#import <Foundation/Foundation.h>

// FIXME: this probably goes elsewhere
#ifndef OBJC_DESIGNATED_INITIALIZER
/// Used to check for modern Objective-C attribute of objc_designated_initializer in compiler
/// see https://clang.llvm.org/docs/AttributeReference.html#objc-designated-initializer
#if __has_attribute(objc_designated_initializer)
/// defined macro for adding the objc_designated_initializer attribute to objective-c constructors
#define OBJC_DESIGNATED_INITIALIZER __attribute__((objc_designated_initializer))
#else
/// empty macro for skipping the objc_designated_initializer attribute on objective-c constructors
///  still helps developers read code
#define OBJC_DESIGNATED_INITIALIZER
#endif /* !__has_attribute(objc_designated_initializer) */
#endif /* !OBJC_DESIGNATED_INITIALIZER */

#if defined(__RAVYNOS__)
#if !defined(NSMENU_RAVYN_OS_PATTERNS)
/// Defined as "NSMainMenu".
#define NSMENU_RAVYN_OS_PATTERN_MENU_TOP_NAME (NSString *)(@"NSMainMenu")
/// Defined as "NSApplicationMenu".
#define NSMENU_RAVYN_OS_PATTERN_APPMENU_NAME (NSString *)(@"NSApplicationMenu")
/// Defined as "About %NewApplication%".
#define NSMENU_RAVYN_OS_PATTERN_APPMENU_ABOUT_TITLE (NSString *)(@"About %@")
/// Defined as "Preferences...".
#define NSMENU_RAVYN_OS_PATTERN_APPMENU_PREFERENCES_TITLE (NSString *)(@"Preferences...")
/// Defined as "Hide %NewApplication%".
#define NSMENU_RAVYN_OS_PATTERN_APPMENU_HIDE_SELF_TITLE (NSString *)(@"Hide %@")
/// Defined as "Show %NewApplication%".
#define NSMENU_RAVYN_OS_PATTERN_APPMENU_SHOW_SELF_TITLE (NSString *)(@"Show %@")
/// Defined as "Hide Others".
#define NSMENU_RAVYN_OS_PATTERN_APPMENU_HIDE_OTHERS_TITLE (NSString *)(@"Hide Others")
/// Defined as "Show All".
#define NSMENU_RAVYN_OS_PATTERN_APPMENU_SHOW_ALL_TITLE (NSString *)(@"Show All")
/// Defined as "Quit %NewApplication%".
#define NSMENU_RAVYN_OS_PATTERN_APPMENU_QUIT_TITLE (NSString *)(@"Quit %@")
#endif /* !defined(NSMENU_RAVYN_OS_PATTERNS) */
#endif /* !defined(__RAVYNOS__) */

@class NSEvent, NSView;
#if defined(NSFont)
#if defined(__RAVYNOS__)
#warning "NSMenu does not support Fonts in this implementation"
#endif
@class NSFont;
#endif
// FIXME: also need to fix NSMenuItem.h
#if !defined(NSMenuItem)
#import <AppKit/NSMenuItem.h>
@class NSMenuItem;
#endif

@class NSMenu;

@protocol NSMenuDelegate;

// FIXME: this probably goes elsewhere
#if !defined(DBusInstanceID)
typedef uint64_t DBusInstanceID;
#endif

// TODO: mention no NSNull as menu is not a true collection
// see more on null/nil: https://stackoverflow.com/questions/1564410/when-should-i-use-nil-and-null-in-objective-c
// Coding is not expected to be storing null/nil data in this implementation

//FIXME: add APKIT_EXPORT macro here if defined
//FIXME: should conform to NSUserInterfaceItemIdentification and accessibility stuff
@interface NSMenu : NSObject <NSCopying, NSCoding> {
@protected
	NSString *_title;
@private
#if !__has_feature(nullability)
	NSMenu *_supermenu;
#if defined(__RAVYNOS__)
	NSString *_name;
#endif /* !defined(__RAVYNOS__) */
	NSMutableArray *_itemArray;
#if !__has_feature(objc_protocol_qualifier_mangling)
	id _delegate;
#else
	id<NSMenuDelegate> _delegate;
#endif
#else /* __has_feature(nullability) */
	NSMenu * __nullable _supermenu;
#if defined(__RAVYNOS__)
	NSString * __null_unspecified _name; // but can be empty string
#endif /* !defined(__RAVYNOS__) */
	NSMutableArray * __null_unspecified _itemArray; // but can be empty array
#if !__has_feature(objc_protocol_qualifier_mangling)
	id __nullable _delegate;
#else
	id<NSMenuDelegate> __nullable _delegate;
#endif
#endif /* end !__has_feature(nullability) */
#if defined(DBusInstanceID)
	DBusInstanceID _DBusItemID;
#else
	uint64_t DBusInstanceID;
#endif
	BOOL _autoenablesItems;
}

//TODO: implement class load to setup globals like menubar height and appearance, and legacy implementation of legacy menuZone api etc.

+ (void)popUpContextMenu:(NSMenu *)menu withEvent:(NSEvent *)event forView:(NSView *)view;

/// Designated initializer
///
/// - Parameters:
///   - title: The menu title `NSString`.
///     Title should already be localized before being passed.
///     If passed a nil title the title will be set to the zero-length string "".
///
/// - Returns: The empty `NSMenu` with the given title.
- (id)initWithTitle:(NSString *)title OBJC_DESIGNATED_INITIALIZER;

#if !__has_feature(nullability)
#if __has_feature(objc_arc)
@property (weak) NSMenu *supermenu;
@property (copy) NSString *title;
#else
@property (assign) NSMenu *supermenu;
@property (retain) NSString *title;
#endif
- (NSMenu *)supermenu;
- (NSString *)title;
#else
#if __has_feature(objc_arc)
@property (nullable, weak) NSMenu *supermenu;
@property (copy) NSString *title;
#else
@property (nullable, assign) NSMenu *supermenu;
@property (nullable, retain) NSString *title;
#endif
- (NSMenu * __nullable)supermenu;
- (NSString * __nullable)title;
#endif
- (int)numberOfItems;

#if !__has_feature(nullability)
#if __has_feature(objc_arc)
@property (copy) NSArray *itemArray;
#else
@property (retain) NSArray *itemArray;
#endif
- (NSArray *)itemArray;
- (void)setItemArray:(NSMutableArray *)newItemArray;
#else
#if __has_feature(objc_arc)
@property (copy) __kindof NSArray *itemArray;
#else
@property (retain) __kindof NSArray *itemArray;
#endif
- (NSArray * __nonnull)itemArray;
- (void)setItemArray:(__kindof NSArray * __nonnull)newItemArray;
#endif
- (BOOL)autoenablesItems;
#if defined(CGFLOAT_DEFINED) && CGFLOAT_DEFINED
- (CGFloat)menuBarHeight;
#endif

#if !__has_feature(nullability)
- (NSMenuItem *)itemAtIndex:(int)index;
- (NSMenuItem *)itemWithTag:(int)tag;
- (NSMenuItem *)itemWithTitle:(NSString *)title;
#else
- (NSMenuItem * __nullable)itemAtIndex:(int)index;
- (NSMenuItem * __nullable)itemWithTag:(int)tag;
- (NSMenuItem * __nullable)itemWithTitle:(NSString * __nullable)title;
#endif

#if defined(NSINTEGER_DEFINED) && NSINTEGER_DEFINED
#warning "NSInteger is defined but NSMenu implements with int (incompatibility)"
///FIXME: developers expect this to use NSInteger (typically defined as a long, not an int)
#endif

#if !__has_feature(nullability)
- (int)indexOfItem:(NSMenuItem *)item;
#else
- (int)indexOfItem:(NSMenuItem * __nullable)item;
#endif
- (int)indexOfItemWithTag:(int)tag;
#if !__has_feature(nullability)
- (int)indexOfItemWithTitle:(NSString *)title;
- (int)indexOfItemWithRepresentedObject:(id)object;
- (int)indexOfItemWithTarget:(id)target andAction:(SEL)action;
- (int)indexOfItemWithSubmenu:(NSMenu *)menu;
#else
- (int)indexOfItemWithTitle:(NSString *__nullable)title;
- (int)indexOfItemWithRepresentedObject:(id __nullable)object;
- (int)indexOfItemWithTarget:(id __nullable)target andAction:(SEL __nullable)action;
- (int)indexOfItemWithSubmenu:(NSMenu * __nullable)menu;
#endif

#if !__has_feature(nullability)
- (void)setSupermenu:(NSMenu *)value;
- (void)setTitle:(NSString *)title;
#else
- (void)setSupermenu:(NSMenu * __nullable)value;
- (void)setTitle:(NSString * __nullable)title;
#endif
- (void)setAutoenablesItems:(BOOL)flag;

#if !__has_feature(nullability)
- (void)addItem:(NSMenuItem *)item;
- (NSMenuItem *)addItemWithTitle:(NSString *)title action:(SEL)action keyEquivalent:(NSString *)keyEquivalent;
#else
- (void)addItem:(NSMenuItem *)item;
- (NSMenuItem *__null_unspecified)addItemWithTitle:(NSString * __null_unspecified)title action:(SEL __null_unspecified)action keyEquivalent:(NSString * __nullable)keyEquivalent;
#endif

- (void)removeAllItems; // TODO: add import guard for rayvnOS to ensure devs don't use outside framework (compatibility: MacOSX 10.5+ provides this but notes it should not be used outside framework)
#if !__has_feature(nullability)
- (void)removeItem:(NSMenuItem *)item;
#else
- (void)removeItem:(NSMenuItem * __nullable)item;
#endif
- (void)removeItemAtIndex:(int)index;

#if !__has_feature(nullability)
- (void)insertItem:(NSMenuItem *)item atIndex:(int)index;
- (NSMenuItem *)insertItemWithTitle:(NSString *)title action:(SEL)action keyEquivalent:(NSString *)keyEquivalent atIndex:(int)index;

- (void)setSubmenu:(NSMenu *)submenu forItem:(NSMenuItem *)item;
#else
- (void)insertItem:(NSMenuItem * __nullable)item atIndex:(int)index;
- (NSMenuItem *)insertItemWithTitle:(NSString * __null_unspecified)title action:(SEL __null_unspecified)action keyEquivalent:(NSString * __nullable)keyEquivalent atIndex:(int)index;

- (void)setSubmenu:(NSMenu * __null_unspecified)submenu forItem:(NSMenuItem * __null_unspecified)item;
#endif

- (void)update;

#if !__has_feature(nullability)
- (void)itemChanged:(NSMenuItem *)item;
#else
- (void)itemChanged:(NSMenuItem * __nullable)item;
#endif

- (BOOL)performKeyEquivalent:(NSEvent *)event;

#if !__has_feature(objc_protocol_qualifier_mangling)
#if !__has_feature(nullability)
#if __has_feature(objc_arc)
@property (weak) id delegate;
#else
@property (assign) id delegate;
#endif /* !__has_feature(objc_arc) */
- (void)setDelegate:(id)object;
- (id __nullable)delegate;
#else
#if __has_feature(objc_arc)
@property (nullable, weak) id delegate;
#else
@property (nullable, assign) id delegate;
#endif /* !__has_feature(objc_arc) */
- (void)setDelegate:(id __nullable)object;
- (id __nullable)delegate;
#endif /* end __has_feature(nullability) */
#else
#if !__has_feature(nullability)
#if __has_feature(objc_arc)
@property (weak) id<NSMenuDelegate> delegate;
#else
@property (assign) id<NSMenuDelegate> delegate;
#endif /* !__has_feature(objc_arc) */
- (void)setDelegate:(id<NSMenuDelegate>)object;
- (id<NSMenuDelegate>)delegate;
#else
#if __has_feature(objc_arc)
@property (nullable, weak) id<NSMenuDelegate> delegate;
#else
@property (nullable, assign) id<NSMenuDelegate> delegate;
#endif /* !__has_feature(objc_arc) */
- (id<NSMenuDelegate> __nullable)delegate;
- (void)setDelegate:(id<NSMenuDelegate> __nullable)object;
#endif /* end __has_feature(nullability) */
#endif /* end __has_feature(objc_protocol_qualifier_mangling) */

#if defined(__RAVYNOS__)

#if !__has_feature(nullability)
#if __has_feature(objc_arc)
@property (weak) NSString *_name;
#else
@property (assign) NSString *title;
#endif
- (NSString *)_name;
- (NSMenu *)_menuWithName:(NSString *)name;
#else
#if __has_feature(objc_arc)
@property (nullable, weak) NSString *_name; //FIXME: should probably be implemented as copy
#else
@property (nullable, assign) NSString *title;
#endif
- (NSString *__nullable)_name;
- (NSMenu *__nullable)_menuWithName:(NSString *__nullable)name;
#endif

#endif /* !defined(__RAVYNOS__) */

#if __has_feature(objc_categories)
@end

@interface NSMenu (MenuHelpers)
#endif
#if defined(__RAVYNOS__)
/*
 Circa 2025
 Modification provided by Mr. Walls under MIT
 */

#if !__has_feature(nullability)
+(NSMenu *)newMenuAsApplicationMenu:(NSString*)appName;
#else
+(NSMenu * __nonnull)newMenuAsApplicationMenu:(NSString*)appName;
#endif /* end __has_feature(nullability) */

#endif /* !defined(__RAVYNOS__) */
@end

@interface NSObject (NSMenu_validateItem)
- (BOOL)validateMenuItem:(NSMenuItem *)item;
@end

#if !defined(NSScreen)
#if defined(__RAVYNOS__)
#warning "NSMenu does not support NSScreen in this implementation"
#endif
@class NSScreen;
#endif

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
