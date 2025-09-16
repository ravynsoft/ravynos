/* Copyright (c) 2025 Mr. Walls -- with 2025 refactor contributed under MIT license.

Derived from code under BSD-2-Clause:

Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSMenu.h>
// may only work with #import "NSMenu.h" // for now

#import <AppKit/NSApplication.h>
@class NSApplication;
#import <AppKit/NSWindow.h>
@class NSWindow;
#import <AppKit/NSEvent.h>

#import <Foundation/NSException.h>
// #warning "no NSException support"
// #import <AppKit/NSRaise.h>

#import <Foundation/NSKeyedArchiver.h>

//FIXME: fix for use https://clang.llvm.org/docs/LanguageExtensions.html#objective-c-retaining-behavior-attributes
//FIXME: fix for use with __has_feature(objc_protocol_qualifier_mangling)
@implementation NSMenu

#if __has_feature(objc_arc)
@dynamic supermenu;
@dynamic title;
@dynamic itemArray;
@dynamic delegate;
#endif

// FIXME: refactor this whole thing without "NSMenuWindow" thing
+(void)popUpContextMenu:(NSMenu *)menu withEvent:(NSEvent *)event forView:(NSView *)view {
#if 0
	// FIXME: 'if (menu==nil) return;' // abort faster if given none menu (user expects nothing)
	// FIXME: 'if (menu==nil) return;' // abort faster if given nil event (what would we do anyway)
	// TODO: if no view then assume event window view -> view under menu etc. -> recurse (and warn developer about performance of not providing view)
	// TODO: also pls document what is a ravynOS NSMenuWindow - b/c they are inconsistent with reference OS menus are not windowed, just views
	[menu update];
	if([[menu itemArray] count]>0){
		//FIXME: no need to use window context, until we have a window, should use absoluteX/absoluteY/absoluteZ for un-windowed NSEvents
		NSPoint       point=[event locationInWindow];
		//FIXME: 'if ([event window]==nil) return;' // abort fast (or use window number to find in NSApp) if given none window (probably got the wrong responder here)
		NSWindow     *window=[event window]; // otherwise this works
		NSMenuWindow *menuWindow=[[NSMenuWindow alloc] initWithMenu:menu]; //FIXME: should use '[window menuView]'
		NSMenuView   *menuView=[menuWindow menuView]; //FIXME: should use '[window menuView]' if even needed
		NSMenuItem   *item;

		[menuWindow setReleasedWhenClosed:YES]; //FIXME: is this needed?
		//TODO: add hook to listen for app will terminate notifications with close window action to prevent leaks
		[menuWindow setFrameTopLeftPoint:[window convertBaseToScreen:point]];
		[menuWindow orderFront:nil]; //FIXME: the menu or event has ordered front this window, so not really a nil sender

		item=[menuView trackForEvent:event];

		[menuWindow close]; // FIXME: why even have window?

		// TODO: depend on NSEvent type: click/move/hover/unclick may be different, and accessibility may be non-mouse
		if(item!=nil)
			[NSApp sendAction:[item action] to:[item target] from:item];
	}
#else
	//FIXME: BUG is NSUnimplementedMethod();
#endif
}

-(void)encodeWithCoder:(NSCoder *)coder {
	if ([coder allowsKeyedCoding])
	{
		// expects to be called by super menu's item array
		/* TODO: add
		 [_io_lock trylock]; // or something that is dead-lock safe
		 */
		NSString *safe_title = [self title]; // allow override support
#if defined(__RAVYNOS__)
		NSString *safe_name = [self _name]; // allow override support
#endif
		// FIXME: handle nesting encoding with support for super menu = NSMMenu
		NSArray *safe_itemArray = [self itemArray]; // allow override support

		if (safe_title!=nil)
			[coder encodeObject:safe_title forKey:@"NSTitle"];
#if defined(__RAVYNOS__)
		if (safe_name!=nil)
			[coder encodeObject:safe_name forKey:@"NSName"];
#endif
		if (safe_itemArray!=nil)
			//TODO: '[safe_itemArray makeObjectsPerformSelector:@selector(_setMenu:) withObject:nil];' // or safe equivalent (see initWithCoder)
			[coder encodeObject:safe_itemArray forKey:@"NSMenuItems"];
		// always encode inverse of boolean
		[coder encodeBool:![self autoenablesItems] forKey:@"NSNoAutoenable"];
		/* TODO: add
		 [_io_lock unlock]; // or something that is dead-lock safe
		 */
	}
	else
	{
#if defined(NSException)
		[NSException raise:NSInvalidArchiveOperationException
					format:@"Only supports Keyed Archiver coders"];
#else
#warning "no NSException support"
		//TODO: perhaps try abort() ?
#endif
	}
}

//FIXME: use __has_feature(objc_instancetype) guard - see https://clang.llvm.org/docs/LanguageExtensions.html#related-result-types
#if !__has_feature(objc_protocol_qualifier_mangling)
-(id)initWithCoder:(NSCoder *)coder
#else
-(id<NSCoding>)initWithCoder:(NSCoder *)coder
#endif
{
	if([coder allowsKeyedCoding]){
		//TODO: verify logic here
		NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;
		self=[super init]; // NSObject init
		if (self != nil) {
			// TODO: consider moving this to an atomic helper
			if ([keyed containsValueForKey:@"NSMenu"]) {
				// FIXME: this seems backwards, as the super menu should encode our menu
				self->_supermenu=[keyed decodeObjectForKey:@"NSMenu"]; // kept as historical just-in-case
				// TODO: lock self->_supermenu (risk: read-while-write concurrency issues)
				int _self_back_ref_index = [self->_supermenu indexOfItemWithSubmenu:self];
				if (_self_back_ref_index<0) {
					id _self_supermenu_item = [self->_supermenu itemAtIndex:_self_back_ref_index];
					if ((_self_supermenu_item!=nil) && (![_self_supermenu_item hasSubmenu])) {
						// rationale, ok we need to add ourself to our super menu
						//TODO: add '[_self_supermenu_item performSelector:@selector(setSubmenu:) withObject:self];'
					}; // else noop
				} // else noop
				// TODO: unlock self->_supermenu
			} else {
				if (self->_supermenu!=nil) {
					// TODO: cleanup/release
					// TODO: lock self->_supermenu (risk: read-while-write concurrency issues)
					int _self_back_ref_index = [self->_supermenu indexOfItemWithSubmenu:self];
					if (_self_back_ref_index>=0) {
						id _self_supermenu_item = [self->_supermenu itemAtIndex:_self_back_ref_index];
						if ((_self_supermenu_item!=nil) && ([_self_supermenu_item hasSubmenu])) {
							// rationale, ok we need to remove ourself from our super menu
							[_self_supermenu_item performSelector:@selector(setSubmenu:) withObject:nil];
						}; // else noop
					} // else noop
					// TODO: unlock self->_supermenu
				} // else noop
				// CHECKPOINT - menu and supermenu should be insync
			}
			// end atomic supermenu helper

			if ([keyed containsValueForKey:@"NSTitle"]) {
				self->_title=[(NSString *)[keyed decodeObjectForKey:@"NSTitle"] copy];
			} else {
				self->_title=nil; // FIXME: should probably name this a localized "untitled menu" or similar
			}
			// CHECKPOINT - menu and menu title should be insync
#if defined(__RAVYNOS__)
			if ([keyed containsValueForKey:@"NSName"]) {
				self->_name=[(NSString *)[keyed decodeObjectForKey:@"NSName"] copy];
			} else {
				self->_name=@""; // FIXME: should probably name this a localized "untitled menu" or similar see getter/setter
			}
			// CHECKPOINT - menu and object name should be insync
#endif
			if ([keyed containsValueForKey:@"NSMenuItems"]) {
				//assuming this is recursive and works
				self->_itemArray=[[NSMutableArray alloc] initWithArray:[keyed decodeObjectForKey:@"NSMenuItems"]];
				//TODO: '[self->_itemArray makeObjectsPerformSelector:@selector(_setMenu:) withObject:self];'
			} else {
				// TODO: consider moving this to an atomic helper
				// TODO: lock self->_itemArray (risk: read-while-write concurrency issues)
				if (self->_itemArray!=nil){
					//TODO: '[self->_itemArray autorelease];' // trigger MCU release-cascade from array to contents
					self->_itemArray=nil;
				} // else noop
				self->_itemArray=[[NSMutableArray alloc] init]; // same logic as [self init]
				// TODO: unlock self->_itemArray
			}
			// assume that NSNoAutoenable is always encoded (see nsmenu encodewithcoder logic)
			self->_autoenablesItems=![keyed decodeBoolForKey:@"NSNoAutoenable"];
			return self; // only safe time to return self
		} else {
#if defined(NSException)
			[NSException raise:NSInvalidArgumentException format:@"Can not initWithCoder:%@",
			 [coder class]];
#else
#warning "no NSException support"
#endif
		}
	}
	else {
		// FIXME: unless overloaded ISA should probably be [NSMenu class]
		// rationale: because subclasses can overload this to change it anyway
#if defined(NSException)
		[NSException raise:NSInvalidArgumentException format:@"%@ can not initWithCoder:%@",isa,[coder class]];
#else
#warning "no NSException support"
		//TODO: perhaps try abort() ?
#endif
	}
	// only reached on error
	return nil;
}

//FIXME: use __has_feature(objc_instancetype) guard - see https://clang.llvm.org/docs/LanguageExtensions.html#related-result-types
-(id)initWithTitle:(NSString *)title {
	self=([super init]); // NSObject init
	if (self != nil) {
		if (title==nil) { self->_title=[@"" copy]; } else { self->_title=[title copy]; }
		self->_itemArray=[[NSMutableArray alloc] init]; //TODO: should consider retain to keep from auto-release
		self->_autoenablesItems=YES;
		// defaults
		self->_supermenu=nil;
		return self;
	} else {
		return nil; // can't init NSObject ... check runtime
	}
}

//FIXME: use __has_feature(objc_instancetype) guard - see https://clang.llvm.org/docs/LanguageExtensions.html#related-result-types
-(id)init {
	return [self initWithTitle:@""];
}

//FIXME: use __has_feature(objc_instancetype) guard - see https://clang.llvm.org/docs/LanguageExtensions.html#related-result-types
// deprecated and old root cause of https://github.com/ravynsoft/ravynos/issues/288
-(id)initApplicationMenu:(NSString*)appName {
	// FIXME: handle nil appName
	// test expected auto-release or [[NSMenu new] retain] and [self autorelease]
	self = [NSMenu newMenuAsApplicationMenu:appName];
	return self;
}

-(void)dealloc {
#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
	//FIXME: add locking here - thread-unsafe otherwise
	if (self->_delegate!=nil) {
		[self->_delegate release]; // cleanup/release
	} // else noop
	[self->_title release];
#if defined(__RAVYNOS__)
	[self->_name release];
#endif
	[self->_itemArray makeObjectsPerformSelector:@selector(_setMenu:) withObject:nil];
	[self->_itemArray removeAllObjects];
	[self->_itemArray release];
	// TODO: unlock here
	// TODO: dealloc internal locks
	[super dealloc];
#else
	[self->_title release];
#if defined(__RAVYNOS__)
	[self->_name release];
#endif
	[self->_itemArray makeObjectsPerformSelector:@selector(_setMenu:) withObject:nil];
	[self->_itemArray release];
	//TODO: add guard to prevent issues with new form
	[super dealloc];
#endif
}

//FIXME: use __has_feature(objc_instancetype) guard - see https://clang.llvm.org/docs/LanguageExtensions.html#related-result-types
-(id)copyWithZone:(NSZone *)zone {
	NSMenu *copy=NSCopyObject(self, 0, zone);

	copy->_title=[_title copyWithZone:zone];
#if defined(__RAVYNOS__)
	copy->_name=[_name copyWithZone:zone];
#endif
	copy->_itemArray = [[NSMutableArray allocWithZone:zone] init];
	for (NSMenuItem *item in _itemArray) {
		[copy addItem: [[item copyWithZone:zone] autorelease]];
	}

	return copy;
}

#if !__has_feature(nullability)
-(NSMenu *)supermenu
#else
-(NSMenu * __nullable)supermenu
#endif
{
#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
	id _self_supermenu = nil;
	//FIXME: add locking here - thread-unsafe otherwise
	if (self->_supermenu!=nil){
		_self_supermenu = self->_supermenu;
	}
	// TODO: unlock here
	return _self_supermenu;
#else
	return self->_supermenu;
#endif
}

#if !__has_feature(nullability)
-(NSString *)title
#else
-(NSString * __nullable)title
#endif
{
#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
	id _self_title = nil;
	//FIXME: add locking here - thread-unsafe otherwise
	if (self->_title!=nil){
		_self_title = self->_title;
	}
	// TODO: unlock here
	return _self_title;
#else
	return self->_title;
#endif
}

-(int)numberOfItems {
#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
	return [[self itemArray] count]; // safer
#else
	return [self->_itemArray count]; // faster
#endif
}

#if !__has_feature(nullability)
-(NSArray *)itemArray
#else
-(NSArray * __nonnull)itemArray
#endif
{
#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
	id _self_itemArray;
	//FIXME: add locking here - thread-unsafe otherwise
	if (self->_itemArray!=nil){
		//FIXME: mutable arrays are unsafe to pass to others (prefer API to change via Key-Value protocols)
		_self_itemArray = self->_itemArray; // TODO: '[NSArray arrayFromArray:... ]' return immutable array here instead
	} else {
		_self_itemArray = [NSArray array]; // empty array - safer to make member calls on
	}
	// TODO: unlock here
	return _self_itemArray;
#else
	//FIXME: mutable arrays are unsafe to pass to others (prefer API to change via Key-Value protocols)
	return self->_itemArray; // TODO: '[NSArray arrayFromArray:... ]' return immutable array here instead
#endif
}

#if !__has_feature(nullability)
-(void)setItemArray:(NSMutableArray *)newItemArray
#else
-(void)setItemArray:(__kindof NSArray * __nonnull)newItemArray
#endif
{
#if !__has_feature(nullability)
	if (newItemArray==nil){
#if !__has_feature(objc_arc)
		if(self->_itemArray!=nil)
		{
			[self->_itemArray release];
		}
#endif
		self->_itemArray=nil; // ensure it is nil
		return; // return here on nil
	}
#endif
	// CHECKPOINT - safe to assume input is not nil by this point
#if !__has_feature(objc_arc)
	NSMutableArray *_safe_new_ItemArray = (NSMutableArray *)[newItemArray retain]; // prevent common GC race-conditions by retaining now
#else
	NSMutableArray *_safe_new_ItemArray = (NSMutableArray *)[newItemArray copy]; // prevent common GC race-conditions by shallow coping now
#endif
#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
	//FIXME: add locking here - thread-unsafe otherwise
#if !__has_feature(objc_arc)
	if(self->_itemArray!=_safe_new_ItemArray)
	{
		[self->_itemArray release];
	}
#if __has_feature(nullability)
	else {
		[newItemArray release]; // re-release double retained input here
	}
#endif
#endif
	self->_itemArray=_safe_new_ItemArray;
	// TODO: unlock here
#else
	if(_itemArray!=_safe_new_ItemArray)
	{
		[_itemArray release]; //FIXME: not arc compatible
		self->_itemArray=_safe_new_ItemArray;
	}
#endif
}

-(BOOL)autoenablesItems {
#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
	BOOL _self_autoenablesItem = YES;
	//FIXME: add locking here - thread-unsafe otherwise
	if !(self->_autoenablesItems) {
		_self_autoenablesItem = NO;
	}
	// TODO: unlock here
	return _self_autoenablesItem;
#else
	return self->_autoenablesItems;
#endif
}

#if !__has_feature(nullability)
-(NSMenuItem *)itemAtIndex:(int)index
#else
-(NSMenuItem * __nullable)itemAtIndex:(int)index
#endif
{
	// depending on implementation may return NSNotFound/NSIntegerMax/undefined
	// assume some NSInteger #if defined()
#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
	return [[self itemArray] objectAtIndex:index]; // safer
#else
	return [self->_itemArray objectAtIndex:index]; // faster - less maintainable
#endif
}

#if !__has_feature(nullability)
-(NSMenuItem *)itemWithTag:(int)tag
#else
-(NSMenuItem * __nullable)itemWithTag:(int)tag
#endif
{
#if defined(__RAVYNOS__)
	if (tag<0)
#else
		// defined
#if defined(NSINTEGER_DEFINED) && NSINTEGER_DEFINED
#if defined(NSNotFound) || defined(__APPLE__)
	/* const from Foundation */
	if (tag==NSNotFound)
#else
	if (tag<0 || tag==NSIntegerMax)
#endif
#else
#if __LP64__ || NS_BUILD_32_LIKE_64
	if (tag<0 || tag==LONG_MAX)
#else
	if (tag<0 || tag==INT_MAX)
#endif
#endif
#endif
		return nil; // return faster on negative tag - for now (consider negative reserved?)
	// CHECKPOINT - tag should be a positive non-overflow value
#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
	int i,count=[self numberOfItems]; // safer
	NSArray *_self_itemArray = [self itemArray]; // thread safe
#else
	int i,count=[self->_itemArray count]; // faster
	NSArray *_self_itemArray = self->_itemArray; // still use immutable array copy while looping
#endif

	for(i=0;i<count;i++){
		NSMenuItem *item=[_self_itemArray objectAtIndex:i];

		if ([item tag] == tag)
			return item;
	}

	return nil;
}

#if !__has_feature(nullability)
-(NSMenuItem *)itemWithTitle:(NSString *)title
#else
-(NSMenuItem * __nullable)itemWithTitle:(NSString * __nullable)title
#endif
{
	if (title==nil) return nil; // return faster on bad input (or did we want nil title matching?)
#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
	int i,count=[self numberOfItems]; // safer
	NSArray *_self_itemArray = [self itemArray]; // thread safe
#else
	int i,count=[self->_itemArray count]; // faster
	NSArray *_self_itemArray = self->_itemArray; // still use immutable array copy while looping
#endif

	for(i=0;i<count;i++){
		NSMenuItem *item=[_self_itemArray objectAtIndex:i];

		if([title isEqualToString:[item title]])
			return item;
	}

	return nil;
}

#if !__has_feature(nullability)
-(int)indexOfItem:(NSMenuItem *)item
#else
-(int)indexOfItem:(NSMenuItem * __nullable)item
#endif
{
	// return faster for bad input
#if defined(__RAVYNOS__)
	if (item==nil) return -1; // return faster on bad index - for now (consider negative reserved?)
#else
	// defined
#if defined(NSINTEGER_DEFINED) && NSINTEGER_DEFINED
#if defined(NSNotFound) || defined(__APPLE__)
	/* const from Foundation */
	if (item==nil) return NSNotFound;
#else
	if (item==nil) return NSIntegerMax;
#endif
#else
#if __LP64__ || NS_BUILD_32_LIKE_64
	if (item==nil) return LONG_MAX
#else
		if (item==nil) return INT_MAX;
#endif
#endif
#endif
#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
	return [[self itemArray] indexOfObjectIdenticalTo:item]; // safer
#else
	return [self->_itemArray indexOfObjectIdenticalTo:item]; // faster - less maintainable
#endif
}

-(int)indexOfItemWithTag:(int)tag {
	// return faster on negative tag
#if defined(__RAVYNOS__)
	if (tag<0) return -1; // return faster on negative tag - for now (consider negative reserved?)
#else
	// defined
#if defined(NSINTEGER_DEFINED) && NSINTEGER_DEFINED
#if defined(NSNotFound) || defined(__APPLE__)
	/* const from Foundation */
	if (tag<0) return NSNotFound;
#else
	if (tag<0) return NSIntegerMax;
#endif
#else
#if __LP64__ || NS_BUILD_32_LIKE_64
	if (tag<0) return LONG_MAX
#else
		if (tag<0) return INT_MAX;
#endif
#endif
#endif
	/**
	 Historically it is possible that every item has the default tag of 0 but in practice that is a flaw.
	 Tags are expected to be unique (Apple Store auto-review would reject the nib).
	 Note: -1 is typically the first responder tag (NSResponder)
	 -2 is typically the nib File's Owner (NSApplication / __kindof NSBundle)
	 -3 is typically the application (__kindof NSObject (NOT NSApplication))
	 for menus -2 will usually be 'super' and -3 will be 'self'
	 */
#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
	// TODO: probably too slow though is thread-safe (consider adding locking to faster implementation)
	// FIXME: may be same as [[self itemArray] indexOfObjectIdenticalTo:nil] in some cases
	return [[self itemArray] indexOfObjectIdenticalTo:[[self itemArray] itemWithTag:tag]];
#else
	NSArray *_self_itemArray = self->_itemArray; // still use immutable array copy while looping
	int i,count=[_self_itemArray count];

	for (i=0; i<count; i++)
		if ([[_self_itemArray objectAtIndex:i] tag] == tag)
			return i;

	return [self indexOfItemWithTag:-1]; // leverage fast-return above
#endif
}

#if !__has_feature(nullability)
-(int)indexOfItemWithTitle:(NSString *)title
#else
-(int)indexOfItemWithTitle:(NSString * __nullable)title
#endif
{
	// return faster on negative tag
#if defined(__RAVYNOS__)
	if (title==nil) return -1; // return faster on negative tag - for now (consider negative reserved?)
#else
	// defined
#if defined(NSINTEGER_DEFINED) && NSINTEGER_DEFINED
#if defined(NSNotFound) || defined(__APPLE__)
	/* const from Foundation */
	if (title==nil) return NSNotFound;
#else
	if (title==nil) return NSIntegerMax;
#endif
#else
#if __LP64__ || NS_BUILD_32_LIKE_64
	if (title==nil) return LONG_MAX
#else
		if (title==nil) return INT_MAX;
#endif
#endif
#endif
#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
	// TODO: probably too slow though is thread-safe (consider adding locking to faster implementation)
	// FIXME: may be same as [[self itemArray] indexOfObjectIdenticalTo:nil] in some cases
	return [[self itemArray] indexOfObjectIdenticalTo:[[self itemArray] itemWithTitle:item]]; // safer
#else
	NSArray *_self_itemArray = self->_itemArray; // still use immutable array copy while looping
	int i,count=[_self_itemArray count];

	for (i=0; i<count; i++)
		if ([title isEqualToString:[[_self_itemArray objectAtIndex:i] title]])  // title in is never nil here, but item's title can be nil
			return i;

	return [self indexOfItemWithTitle:nil]; // leverage fast-return above
#endif
}

#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
#if !__has_feature(nullability)
-(NSMenuItem *)itemWithRepresentedObject:(id)object
#else
-(NSMenuItem * __nullable)itemWithRepresentedObject:(id __nullable)object
#endif
{
	if (object==nil) return nil; // return faster on negative tag - for now (consider negative reserved?)

	int i,count=[self numberOfItems]; // safer
	NSArray *_self_itemArray = [self itemArray]; // thread safe

	for(i=0;i<count;i++){
		NSMenuItem *item=[_self_itemArray objectAtIndex:i];
		if ([item representedObject]!=nil) && ([[item representedObject] isEqual:object])
			return item;
	}

	return nil;
}
#endif


#if !__has_feature(nullability)
-(int)indexOfItemWithRepresentedObject:(id)object
#else
-(int)indexOfItemWithRepresentedObject:(id __nullable)object
#endif
{
	// return faster on negative tag
#if defined(__RAVYNOS__)
	if (object==nil) return -1; // return faster on negative tag - for now (consider negative reserved?)
#else
	// defined
#if defined(NSINTEGER_DEFINED) && NSINTEGER_DEFINED
#if defined(NSNotFound) || defined(__APPLE__)
	/* const from Foundation */
	if (object==nil) return NSNotFound;
#else
	if (object==nil) return NSIntegerMax;
#endif
#else
#if __LP64__ || NS_BUILD_32_LIKE_64
	if (object==nil) return LONG_MAX
#else
		if (object==nil) return INT_MAX;
#endif
#endif
#endif
	// FIXME: UNLIKE the rest of the implementations for indexes there are still unhandled edge-cases here for nils
#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
	// TODO: probably too slow though is thread-safe (consider adding locking to faster implementation)
	// FIXME: may be same as [[self itemArray] indexOfObjectIdenticalTo:nil] in some cases
	return [[self itemArray] indexOfObjectIdenticalTo:[[self itemArray] itemWithRepresentedObject:object]]; // safer
#else
	NSArray *_self_itemArray = self->_itemArray; // still use immutable array copy while looping
	int i,count=[_self_itemArray count];

	for (i=0; i<count; i++) {
		NSMenuItem *item=[_self_itemArray objectAtIndex:i];
		if ([[item representedObject] isEqual:object])  // FIXME: object is never nil here, but item's rep-object can be nil
			return i;
	}
	return [self indexOfItemWithRepresentedObject:nil]; // leverage fast-return above
#endif
}

#if !__has_feature(nullability)
// needed this for NSApplication windowsMenu stuff, so i did 'em all..
-(int)indexOfItemWithTarget:(id)target andAction:(SEL)action
#else
-(int)indexOfItemWithTarget:(id __nullable)target andAction:(SEL __nullable)action
#endif
{
	// return faster on no target (or do we allow fuzzy match on action only? e.g., first responder)
#if defined(__RAVYNOS__)
	if (target==nil) return -1; // return faster on nil (but undefined)
#else
	// defined
#if defined(NSINTEGER_DEFINED) && NSINTEGER_DEFINED
#if defined(NSNotFound) || defined(__APPLE__)
	/* const from Foundation */
	if (target==nil) return NSNotFound;
#else
	if (target==nil) return NSIntegerMax;
#endif
#else
#if __LP64__ || NS_BUILD_32_LIKE_64
	if (target==nil) return LONG_MAX
#else
		if (target==nil) return INT_MAX;
#endif
#endif
#endif
#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
	int i,count=[self numberOfItems]; // safer
	NSArray *_self_itemArray = [self itemArray]; // thread safe
#else
	int i,count=[self->_itemArray count]; // faster
	NSArray *_self_itemArray = self->_itemArray; // still use immutable array copy while looping
#endif

	for (i=0; i<count; ++i) {
		NSMenuItem *item = [_self_itemArray objectAtIndex:i];
		//FIXME: has target check?
		if ([item target] == target) {
			//FIXME: has action check

			if (action == nil)
				return i;
			/* else just fall through */
			if ((SEL)[item action] == action)
				return i;
		}
	}

	return [self indexOfItemWithTarget:nil andAction:nil]; // leverage fast-return above
}

#if !__has_feature(nullability)
-(int)indexOfItemWithSubmenu:(NSMenu *)submenu
#else
-(int)indexOfItemWithSubmenu:(NSMenu * __nullable)submenu
#endif
{
	// return faster on no target (or do we allow fuzzy match on action only? e.g., first responder)
#if defined(__RAVYNOS__)
	if (submenu==nil) return -1; // return faster on nil (but undefined)
#else
	// defined
#if defined(NSINTEGER_DEFINED) && NSINTEGER_DEFINED
#if defined(NSNotFound) || defined(__APPLE__)
	/* const from Foundation */
	if (submenu==nil) return NSNotFound;
#else
	if (submenu==nil) return NSIntegerMax;
#endif
#else
#if __LP64__ || NS_BUILD_32_LIKE_64
	if (submenu==nil) return LONG_MAX
#else
	if (submenu==nil) return INT_MAX;
#endif
#endif
#endif
#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
	int i,count=[self numberOfItems]; // safer
	NSArray *_self_itemArray = [self itemArray]; // thread safe
#else
	int i,count=[self->_itemArray count]; // faster
	NSArray *_self_itemArray = self->_itemArray; // still use immutable array copy while looping
#endif
	for (i=0; i<count; i++) {
		NSMenuItem *item=[_self_itemArray objectAtIndex:i];
		if ([[item submenu] isEqual:submenu])  // FIXME: submenu is never nil here, but item's submenu can be nil
			return i;
	}

	return -1;
}

#if !__has_feature(nullability)
-(void)setSupermenu:(NSMenu *)value
#else
-(void)setSupermenu:(NSMenu * __nullable)value
#endif
{
#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
	//FIXME: add locking here - thread-unsafe otherwise
	if (value != self->_supermenu)
	{
		__typeof(value) tmpValue = value;
		if (self->_supermenu!=nil) {
			// TODO: cleanup/release
			int _self_back_ref_index = [self->_supermenu indexOfItemWithSubmenu:self];
			if (_self_back_ref_index>=0) {
#if __has_feature(objc_protocol_qualifier_mangling)
				id<NSMenu __nullable *> _self_supermenu_item = [self->_supermenu itemAtIndex:_self_back_ref_index];
#else
				id _self_supermenu_item = [self->_supermenu itemAtIndex:_self_back_ref_index];
#endif
				if (_self_supermenu_item!=nil) && ([_self_supermenu_item hasSubmenu]) {
					// rationale, ok we need to remove ourself from our old super menu
					//TODO: [_self_supermenu_item performSelectorInBackground:@selector(setSubmenu:) withObject:nil]; // or deadlock safe equivilant
				}; // else noop
				// TODO: _self_supermenu_item = nil; // now safe to be autoreleased
			} // else noop
		} // else noop
		self->_supermenu = tmpValue; // now thread-safe while locked
	};
	// TODO: unlock here
#else
	self->_supermenu = value; // faster
#endif
}

#if !__has_feature(nullability)
-(void)setTitle:(NSString *)title
#else
-(void)setTitle:(NSString * __nullable)title
#endif
{
#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
	//FIXME: add locking here - thread-unsafe otherwise
	if (title != self->_title)
	{
		NSString *tmpValue = nil;
		if (title != nil) tmpValue = [title copy]; // don't copy nil but assume copy returns retained
		if (self->_title!=nil) {
			[self->_title release]; // cleanup/release
		} // else noop
		self->_title = tmpValue; // now thread-safe while locked
	};
	// TODO: unlock here
#else
	title=[title copy]; // unsafe shadow clone
	[self->_title release];
	self->_title=title; // faster
#endif
}

-(void)setAutoenablesItems:(BOOL)flag {
#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
	//FIXME: add locking here - thread-unsafe otherwise
	if (flag != self->_autoenablesItems)
		self->_autoenablesItems = !(self->_autoenablesItems); // safe shadow clone (inversion)
	// TODO: unlock here
#else
	self->_autoenablesItems=flag; // faster
#endif
}

#if !__has_feature(nullability)
-(void)addItem:(NSMenuItem *)item
#else
-(void)addItem:(NSMenuItem * __nullable)item
#endif
{
	if (item==nil) return; // do nothing really fast
	// else
	[item performSelector:@selector(_setMenu:) withObject:self]; // or in-background to be deadlock safe
#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
	//FIXME: add locking here - thread-unsafe otherwise
	[self->_itemArray addObject:item];
	// TODO: unlock here
#else
	[self->_itemArray addObject:item]; // faster
#endif
}

#if !__has_feature(nullability)
-(NSMenuItem *)addItemWithTitle:(NSString *)title action:(SEL)action keyEquivalent:(NSString *)keyEquivalent
#else
-(NSMenuItem * __null_unspecified)addItemWithTitle:(NSString * __null_unspecified)title action:(SEL __null_unspecified)action keyEquivalent:(NSString * __nullable)keyEquivalent
#endif
{
	NSMenuItem *item=[[[NSMenuItem alloc] initWithTitle:title action:action keyEquivalent:keyEquivalent] autorelease];
	[self addItem:item];
	return item;
}

-(void)removeAllItems {
#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
	//FIXME: add locking here - thread-unsafe otherwise
	// see required API https://github.com/ravynsoft/ravynos/blob/a1330219b84d2d3498bbadeb8c9322a0861e041d/Frameworks/Foundation/NSArray/NSMutableArray.h#L20C9-L20C25
	[self->_itemArray makeObjectsPerformSelector:@selector(setMenu:) withObject:nil]; // thread safe way to forget this menus
	[self->_itemArray performSelector:@selector(removeAllObjects)]; // or even release and realloc
	// TODO: unlock here
#else
	while([_itemArray count]>0)
		[self removeItem:[_itemArray lastObject]];
#endif
}

-(void)removeItem:(NSMenuItem *)item {
	if (item==nil) return; // do nothing really fast
#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
	//FIXME: add locking here - thread-unsafe otherwise
	[item performSelector:@selector(setMenu:) withObject:nil];
	[self->_itemArray removeObjectIdenticalTo:item];
	// TODO: unlock here
#else
	[item performSelector:@selector(setMenu:) withObject:nil];
	[self->_itemArray removeObjectIdenticalTo:item];
#endif
}

-(void)removeItemAtIndex:(int)index {
	[self removeItem:[self itemAtIndex:index]];
}

-(void)insertItem:(NSMenuItem *)item atIndex:(int)index {
	if (item==nil) return; // do nothing really fast
	[item performSelector:@selector(setMenu:) withObject:self];
#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
	//FIXME: add locking here - thread-unsafe otherwise
	[self->_itemArray insertObject:item atIndex:index];
	// TODO: unlock here
#else
	[self->_itemArray insertObject:item atIndex:index];
#endif
}

#if !__has_feature(nullability)
-(NSMenuItem *)insertItemWithTitle:(NSString *)title action:(SEL)action keyEquivalent:(NSString *)keyEquivalent atIndex:(int)index
#else
-(NSMenuItem * __null_unspecified)insertItemWithTitle:(NSString * __null_unspecified)title action:(SEL __null_unspecified)action keyEquivalent:(NSString * __nullable)keyEquivalent atIndex:(int)index
#endif
{
	NSMenuItem *item=[[[NSMenuItem alloc] initWithTitle:title action:action keyEquivalent:keyEquivalent] autorelease];
	[self insertItem:item atIndex:index];
	return item;
}

#if !__has_feature(nullability)
-(void)setSubmenu:(NSMenu *)submenu forItem:(NSMenuItem *)item
#else
-(void)setSubmenu:(NSMenu * __null_unspecified)submenu forItem:(NSMenuItem * __null_unspecified)item
#endif
{
	//TODO: this should probably check stuff like nil
	[item setSubmenu:submenu];
}

#if !__has_feature(nullability)
-(BOOL)itemIsEnabled:(NSMenuItem *)item
#else
-(BOOL)itemIsEnabled:(NSMenuItem * __nullable)item
#endif
{
if (item==nil) return NO; //do nothing really fast
BOOL enabled=NO;

if([item action]!=nil){
	id target=[item target];

	target=[[NSApplication sharedApplication] targetForAction:[item action] to:[item target] from:nil];

	if ((target == nil) || ![target respondsToSelector:[item action]]) {
		enabled = NO;
	} else if ([target respondsToSelector:@selector(validateMenuItem:)]) {
		enabled = [target validateMenuItem:item];
	} else if ([target respondsToSelector:@selector(validateUserInterfaceItem:)]) { // New validation scheme
		enabled = (BOOL)[target validateUserInterfaceItem:item];
	} else {
		enabled = YES;
	}
}

return enabled;
}

// FIXME: ensure thread-safe is race-condition free - even with BUILD_NSMENU_WITH_LOCKING it is not yet
-(void)update {
	// developers expect this to do nothing if autoenable items is disabled
	if (![self autoenablesItems]) return; // so do nothing really fast when not autoenabling items

#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
	int i,count=[self numberOfItems]; // safer
	NSArray *_self_itemArray = [self itemArray]; // simple but technically thread safe, yet NOT atomic
#else
	if ([_delegate respondsToSelector:@selector(menuNeedsUpdate:)]) {
		[_delegate menuNeedsUpdate:self];
	}
	int i,count=[self->_itemArray count]; // faster
	NSArray *_self_itemArray = self->_itemArray; // still use immutable array copy while looping
#endif
	BOOL _just_assume_enabled = [self autoenablesItems]; // check once
	for(i=0;i<count;i++){
		NSMenuItem *item=[_self_itemArray objectAtIndex:i];
		BOOL _can_enable = ![item isSeparatorItem]; // check only once per item
		if(![item isHidden] && _can_enable){
			BOOL currentlyEnabled = [item isEnabled] ? YES : NO; // check only once per item
			BOOL enabled = (_just_assume_enabled || currentlyEnabled) ? YES : NO; // check only once per item

#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
			if(enabled!=currentlyEnabled){
				//FIXME: add locking here - thread-unsafe otherwise
				[[self->_itemArray objectAtIndex:i] setEnabled:enabled]; // simple but technically thread safe, yet NOT atomic
				// TODO: unlock here
			}
#else

#if defined(__RAVYNOS__)
			if(enabled!=currentlyEnabled && ![item _binderForBinding:@"enabled" create:NO])
#else
				if(enabled!=currentlyEnabled)
#endif
			{
				[item setEnabled:enabled];
				[self itemChanged:item]; // FIXME: this will recursively re-check - needlessly by default
			}
#endif
		} else { if (_can_enable && [item hasSubmenu]) {
#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
			//FIXME: add locking here - thread-unsafe otherwise
			[[self->_itemArray objectAtIndex:i] update]; // simple but technically thread safe, yet NOT atomic
			// TODO: unlock here
#else
			[[item submenu] update];  // faster - but might not work for copy mmc
#endif
		} /* else probably does not really matter (or do we want to handle hidden too?) */
		}
	} /* end for loop */
}

#if !__has_feature(nullability)
-(void)itemChanged:(NSMenuItem *)item
#else
-(void)itemChanged:(NSMenuItem * __nullable)item
#endif
{
#if defined(__RAVYNOS__)
//FIXME: this should probably call [self update] on a background thread instead of during [self performKeyEquivalent]
return; //ravynOS menu does not implement this
#else
#warning "NSMenu itemChanged: is not implemented"
#endif
// COMPATIBILITY: Developers expect this to raise NSMenuDidChangeItemNotification at the end
//TODO: figure out what userInfo is expected for true compatibility with apple's NSMenu
// FIXME: add something like [NSNotificationCenter.defaultCenter postNotificationName:NSMenuDidChangeItemNotification object:self];
}

#if !__has_feature(nullability)
-(BOOL)performKeyEquivalent:(NSEvent *)event
#else
-(BOOL)performKeyEquivalent:(NSEvent * __nullable)event
#endif
{
	//FIXME: handling events need to be very efficient or user will notice UI lag
	if (event==nil) return NO; // return fast for nothing
	// assume we are the first responder here
	// FIXME: skip impossible inputs like gamepads and tablets with 'if [theEvent type]' checks
	NSString *characters=[event charactersIgnoringModifiers];
#if defined(NSEventModifierFlags) || defined(__APPLE__)
	NSEventModifierFlags modifiers=[event modifierFlags];
#else
	unsigned int modifiers=[event modifierFlags];
#endif

#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
	int i,count=[self numberOfItems]; // safer
	NSArray *_self_itemArray = [self itemArray]; // thread safe
	//FIXME: avoid anything like re-computing here E.G., calls to update
#else
	int i,count=[self->_itemArray count]; // faster
	NSArray *_self_itemArray = self->_itemArray; // still use immutable array copy while looping
	if (self->_autoenablesItems) // FIXME: this is a bug
		[self update];
#endif
	BOOL _just_assume_enabled = [self autoenablesItems]; // check once
	for(i=0;i<count;i++){
		NSMenuItem *item=[_self_itemArray objectAtIndex:i];
		// is the item enabled? (ignoring how)
		// FIXME: skip dividers as they should not be allowed to do things with events
		// TODO: ensure this is compiled as a lazy OR for performance; otherwise refactor
		if (_just_assume_enabled || [item isEnabled]) {
			// TODO: check hidden and allow when hidden logic (or put that over in NSMenuItem?)
#if defined(NSEventModifierFlags) || defined(__APPLE__)
			NSEventModifierFlags itemModifiersMask = [item keyEquivalentModifierMask];
#else
			unsigned int itemModifiers=[item keyEquivalentModifierMask];
#endif
			// Does the event contain *all* modifiers required by the menu item?
			if ((modifiers & itemModifiersMask) == itemModifiersMask) {

				// Optional: also ensure no extra modifiers are present
				// BOOL exactMatch = (modifiers & itemModifiersMask) == modifiers && (modifiers & itemModifiersMask) == itemModifiersMask;
				NSString *key=[item keyEquivalent];
				if([key isEqualToString:characters]){
					/* This *must* accurately reflect menu validation when ignoring or processing
					 key equivalents. Relying on update to keep isEnabled in the proper state is
					 unfortunately too tenuous.
					 */
					// TODO: implement & use helper if (itemIsEnabled(item)) for __RAVYNOS__
#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
					if ([NSApplication sharedApplication] != nil) {
						// TODO: check the to-from logic here
						return [[NSApplication sharedApplication] sendAction:[item action] to:[item target] from:item];
					} else { /* No application to act */ return NO; }
#else
					return [[NSApplication sharedApplication] sendAction:[item action] to:[item target] from:item];
#endif
				}
			}
			if([item hasSubmenu])
				if ([[item submenu] performKeyEquivalent:event]) return YES; // else fall-through NO
		} /* else not enabled */
	} /* end loop */
#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
	//FIXME: avoid same thread for anything like re-computing, but DO trigger notifications
	// TODO: consider some kind of NeedsUpdated notification trigger
	if ([self delegate]!=nil && [[self delegate] respondsToSelector:@selector(menuNeedsUpdate:)]) {
		[[self delegate] menuNeedsUpdate:self];
	} //FIXME: follow supermenu chain up to nearest delegate
#endif
	return NO; // FIXME: either document this as the top class or call super
}

#if !__has_feature(nullability)
-(BOOL)performClickEquivalent:(NSMenuItem *)item
#else
-(BOOL)performClickEquivalent:(NSMenuItem * __nullable)item
#endif
{
	//FIXME: handling clicks needs to be very efficient or user will notice UI lag
	if (item==nil) return NO; // return fast for nothing
	// assume we are the first responder here
	//FIXME: do we need to handle open-close submenu here? e.g. (self == item) case?
#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
	// optional check for if ([self indexOfItem:item]<1) return NO; // only care about our own items
	//FIXME: avoid anything like re-computing here E.G., calls to update
#else
	if (self->_autoenablesItems) // FIXME: this is a bug
		[self update];
#endif
	BOOL _just_assume_enabled = [self autoenablesItems]; // check once
	// is the item enabled? (ignoring how)
	// FIXME: skip dividers as they should not be allowed to do things with clicks
	// TODO: ensure this is compiled as a lazy OR for performance; otherwise refactor
	if ([self autoenablesItems] || [item isEnabled]) {
		// TODO: check hidden and allow when hidden logic (or put that over in NSMenuItem?)
		NSString *key=[item keyEquivalent];
		if([item action]!=nil){
			/* This *must* accurately reflect menu validation when ignoring or processing
			 key equivalents. Relying on update to keep isEnabled in the proper state is
			 unfortunately too tenuous.
			 */
			// TODO: implement & use helper if (itemIsEnabled(item)) for __RAVYNOS__
#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
			if ([NSApplication sharedApplication] != nil) {
				// TODO: check the to-from logic here
				BOOL result = [[NSApplication sharedApplication] sendAction:[item action] to:[item target] from:item];
				if (result) [self itemChanged:item]; // TODO: make async via notification?
				return result; // always return
			} else { /* No application to act */ return NO; }
#else
			return [[NSApplication sharedApplication] sendAction:[item action] to:[item target] from:item];
#endif
		} else {
			/* check for submenu when no action (e.g. to allow submenu customization by app devs) */
			if([item hasSubmenu])
				if ([[item submenu] performClickEquivalent:item]) return YES; // else fall-through NO
		}
	} /* else not enabled */
#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
	//FIXME: avoid same thread for anything like re-computing, but DO trigger notifications
	// TODO: consider some kind of NeedsUpdated notification trigger
	if ([self delegate]!=nil && [[self delegate] respondsToSelector:@selector(menuNeedsUpdate:)]) {
		[[self delegate] menuNeedsUpdate:self];
	} //FIXME: follow supermenu chain up to nearest delegate
#endif
	return NO; // FIXME: either document this as the top class or call super
}

#if !__has_feature(objc_protocol_qualifier_mangling)
#if !__has_feature(nullability)
- (void)setDelegate:(id)object
#else
- (void)setDelegate:(id __nullable)object
#endif /* end __has_feature(nullability) */
#else
#if !__has_feature(nullability)
- (void)setDelegate:(id<NSMenuDelegate>)object
#else
- (void)setDelegate:(id<NSMenuDelegate> __nullable)object
#endif /* end __has_feature(nullability) */
#endif /* end __has_feature(objc_protocol_qualifier_mangling) */
{
#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
	//FIXME: add locking here - thread-unsafe otherwise
	if (object != self->_delegate)
	{
		__typeof(object) *tmpValue = nil; // weak property can be nil
		if (object != nil) tmpValue = object; // assign (weak) and don't retain nil
		self->_delegate = tmpValue; // now thread-safe while locked
	};
	// TODO: unlock here
#else
	self->_delegate = object; // faster
#endif
}

#if !__has_feature(objc_protocol_qualifier_mangling)
#if !__has_feature(nullability)
-(id)delegate
#else
-(id __nullable)delegate
#endif /* end __has_feature(nullability) */
#else
#if !__has_feature(nullability)
-(id<NSMenuDelegate>)delegate
#else
-(id<NSMenuDelegate> __nullable)delegate
#endif /* end __has_feature(nullability) */
#endif /* end __has_feature(objc_protocol_qualifier_mangling) */
{
#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
#if __has_feature(objc_protocol_qualifier_mangling)
	id<NSMenuDelegate> _self_delegate = nil; // weak property can be nil or broken pointer (prefer nil)
#else
	id _self_delegate;
#endif
	//FIXME: add locking here - thread-unsafe otherwise
	if (self->_delegate!=nil){
#if !__has_feature(nullability)
		_self_delegate = (NSMenuDelegate *)(self->_delegate);
#else
		_self_delegate = (NSMenuDelegate * __nullable)(self->_delegate);
#endif
	}
	// TODO: unlock here
	return _self_delegate; // safer
#else
	return self->_delegate; // faster
#endif
}


#if defined(__RAVYNOS__)

/* RavynOS Extras */

#if !__has_feature(nullability)
-(NSString *)_name
#else
-(NSString * __nonnull)_name
#endif
{
#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
	NSString *_self_name = nil;
	//FIXME: add locking here - thread-unsafe otherwise
	if (self->_name!=nil){
		_self_name = self->_name;
		// TODO: unlock here
	} else {
		// TODO: unlock here
		// use a hopefully unique untitled placeholder
		_self_name = [NSString stringWithFormat:@"UntitledMenu %i", [self DBusItemID], nil];
	}
	return _self_name; // safer
#else
	return self->_name; // faster
#endif
}

#if !__has_feature(nullability)
-(NSMenu *)_menuWithName:(NSString *)name
#else
-(NSMenu * __nullable)_menuWithName:(NSString *__nullable)name
#endif
{
#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
	NSString *_self_name = [self name]; // safer
#else
	NSString *_self_name = self->_name; // faster
#endif
	if(_self_name!=nil && [_self_name isEqual:name])
		return self;
	else {
#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
		int i,count=[self numberOfItems]; // safer
		NSArray *_self_itemArray = [self itemArray]; // use immutable array copy while looping
#else
		int i,count=[self->_itemArray count]; // faster
		NSArray *_self_itemArray = self->_itemArray; // still use immutable array copy while looping
#endif
		for(i=0;i<count;i++){
			NSMenuItem *item = [_self_itemArray objectAtIndex:i];
			if ([item hasSubmenu]) {
				NSMenu *check=[[item submenu] _menuWithName:name];
				if(check!=nil)
					return check;
			}
			/* otherwise fall-through nil */
		}
	} /* and fall-through to nil */
	return nil;
}

/* see NSUserInterfaceItemIdentification.h for compatibility could be unified ID */

#if !defined(DBusInstanceID)
-(uint64_t)DBusItemID
#else
-(DBusInstanceID)DBusItemID
#endif
{
#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
	// e.g., __typeof(self->_DBusItemID) but thread-safe
#if !defined(DBusInstanceID)
	uint64_t _self_DBusItemID = ULLONG_MAX; // or some other invalid DBusItemID
#else
	DBusInstanceID _self_DBusItemID = ULLONG_MAX;
#endif
	//FIXME: add locking here - thread-unsafe otherwise
	if (self->_DBusItemID!=ULLONG_MAX){
#if !defined(DBusInstanceID)
		_self_DBusItemID = (uint64_t)(self->_DBusItemID);
#else
		_self_DBusItemID = (DBusInstanceID)(self->_DBusItemID);
#endif
	}
	// TODO: unlock here
	return _self_DBusItemID; // safer
#else
	return self->_DBusItemID; // faster
#endif
}

#if !defined(DBusInstanceID)
-(void)setDBusItemID:(uint64_t)itemID
#else
-(void)setDBusItemID:(DBusInstanceID)itemID
#endif
{
#if defined(BUILD_NSMENU_WITH_LOCKING) && BUILD_NSMENU_WITH_LOCKING
		//FIXME: add locking here - thread-unsafe otherwise
		if (itemID != self->_DBusItemID)
		{
			self->_DBusItemID = itemID; // now thread-safe while locked
		};
		// TODO: unlock here
#else
	self->_DBusItemID=itemID; // faster
#endif
}

#if __has_feature(objc_categories)
@end

@implementation NSMenu (MenuHelpers)
#endif
/*
 Circa 2025
 Modification provided by Mr. Walls under MIT
*/

#if !__has_feature(nullability)
+(NSMenu *)newMenuAsApplicationMenu:(NSString*)appName
#else
+(NSMenu * _Nonnull)newMenuAsApplicationMenu:(NSString*)appName
#endif
{
	//TODO: add localization logic
	NSMenu *aMenu = [NSMenu new];
	// legacy code:
	//[aMenu setValue:@"NSAppleMenu" forKey:@"name"]; // assuming KvP
	//FIXME: generate valid DBus
#if defined(DBusInstanceID)
	//FIXME: generate valid DBusItemID for this menu and assign it here
#endif
	[aMenu setTitle:appName];

	NSMenuItem *aboutItem = [[NSMenuItem alloc] initWithTitle:[NSString stringWithFormat:NSMENU_RAVYN_OS_PATTERN_APPMENU_ABOUT_TITLE, appName]
												action:@selector(orderFrontStandardAboutPanel:)
												keyEquivalent:@""];
	[aboutItem setTarget:[NSApplication sharedApplication]]; // FIXME: should be set to first-responder
	[aMenu addItem:aboutItem];

	[aMenu addItem:[NSMenuItem separatorItem]];

	// this stuff must be customized by the App Developer
	NSMenuItem *prefItem = [[NSMenuItem alloc] initWithTitle:NSMENU_RAVYN_OS_PATTERN_APPMENU_PREFERENCES_TITLE
												action:NULL
												keyEquivalent:@","];
	[prefItem setKeyEquivalentModifierMask:NSEventModifierFlagCommand]; // now it is command+comma
	[prefItem setTarget:nil]; // FIXME: should be set to custom target
	// consider default of [prefItem setEnabled:NO];
	[aMenu addItem:prefItem];

	[aMenu addItem:[NSMenuItem separatorItem]];

#if defined(NSMENU_RAVYN_OS_SUPPORTS_APP_SERVICES) && NSMENU_RAVYN_OS_SUPPORTS_APP_SERVICES
	//FIXME: handle services menu stuff
	NSMenuItem *servicesItem = [[NSMenuItem alloc] initWithTitle:@"Services"
													action:NULL
													keyEquivalent:@""];
	[servicesItem setTarget:nil]; // FIXME: should be set to system
	[servicesItem setSubmenu:[NSMenu new]]; // FIXME: this is an empty placeholder for system services menu
	[aMenu addItem:servicesItem];

	[aMenu addItem:[NSMenuItem separatorItem]];
#endif

	NSMenuItem *hideSelfItem = [[NSMenuItem alloc] initWithTitle:[NSString stringWithFormat:NSMENU_RAVYN_OS_PATTERN_APPMENU_HIDE_SELF_TITLE, appName]
													action:@selector(hide:)
													keyEquivalent:@"h"];
	[hideSelfItem setKeyEquivalentModifierMask:NSEventModifierFlagCommand]; // now it is command+h
	[hideSelfItem setTarget:[NSApplication sharedApplication]]; // FIXME: should be set to first-responder
	[aMenu addItem:hideSelfItem];

	NSMenuItem *hideOthersItem = [[NSMenuItem alloc] initWithTitle:[NSString stringWithFormat:NSMENU_RAVYN_OS_PATTERN_APPMENU_HIDE_OTHERS_TITLE, appName]
													action:@selector(hideOtherApplications:)
													keyEquivalent:@"h"];
	[hideOthersItem setKeyEquivalentModifierMask:(NSEventModifierFlagCommand|NSEventModifierFlagOption)]; // now it is command+option/alt+h
	[hideOthersItem setTarget:[NSApplication sharedApplication]]; // FIXME: should be set to first-responder
	[aMenu addItem:hideOthersItem];

	NSMenuItem *showAllItem = [[NSMenuItem alloc] initWithTitle:[NSString stringWithFormat:NSMENU_RAVYN_OS_PATTERN_APPMENU_SHOW_ALL_TITLE, appName]
													action:@selector(unhideAllApplications:)
													keyEquivalent:@""];
	[showAllItem setTarget:[NSApplication sharedApplication]]; // FIXME: should be set to first-responder
	[aMenu addItem:showAllItem];

	[aMenu addItem:[NSMenuItem separatorItem]];

	// see GHI https://github.com/ravynsoft/ravynos/issues/288
	NSMenuItem *quitItem = [[NSMenuItem alloc] initWithTitle:[NSString stringWithFormat:NSMENU_RAVYN_OS_PATTERN_APPMENU_QUIT_TITLE, appName] action:@selector(terminate:) keyEquivalent:@"q"];
	[quitItem setKeyEquivalentModifierMask:NSEventModifierFlagCommand]; // now it is command+q
	[quitItem setTarget:[NSApplication sharedApplication]];
	[aMenu addItem:quitItem];

	return aMenu;
}

#endif /* !__RAVYNOS__ */

@end
