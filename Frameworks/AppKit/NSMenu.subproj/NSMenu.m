/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSMenu.h>
#import <AppKit/NSMenuItem.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSEvent.h>
#import <AppKit/NSMenuWindow.h>
#import <AppKit/NSMenuView.h>
#import <Foundation/NSKeyedArchiver.h>

@implementation NSMenu

+(void)popUpContextMenu:(NSMenu *)menu withEvent:(NSEvent *)event forView:(NSView *)view {
   [menu update];
   if([[menu itemArray] count]>0){
    NSPoint       point=[event locationInWindow];
    NSWindow     *window=[event window];
    NSMenuWindow *menuWindow=[[NSMenuWindow alloc] initWithMenu:menu];
    NSMenuView   *menuView=[menuWindow menuView];
    NSMenuItem   *item;
    
    [menuWindow setReleasedWhenClosed:YES];
    [menuWindow setFrameTopLeftPoint:[window convertBaseToScreen:point]];
    [menuWindow orderFront:nil];

    item=[menuView trackForEvent:event];
 
    [menuWindow close];

    if(item!=nil)
     [NSApp sendAction:[item action] to:[item target] from:item];
   }
}

-(void)encodeWithCoder:(NSCoder *)coder {
   [coder encodeObject:_title forKey:@"NSTitle"];
   [coder encodeObject:_name forKey:@"NSName"];
   [coder encodeObject:_itemArray forKey:@"NSMenuItems"];
   [coder encodeBool:!_autoenablesItems forKey:@"NSNoAutoenable"];
}

-initWithCoder:(NSCoder *)coder {
   if([coder allowsKeyedCoding]){
    NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;
    
    _supermenu=[keyed decodeObjectForKey:@"NSMenu"];
    _title=[[keyed decodeObjectForKey:@"NSTitle"] copy];
    _name=[[keyed decodeObjectForKey:@"NSName"] copy];
    
    _itemArray=[[NSMutableArray alloc] initWithArray:[keyed decodeObjectForKey:@"NSMenuItems"]];
    _autoenablesItems=![keyed decodeBoolForKey:@"NSNoAutoenable"];    
   }
   else {
    [NSException raise:NSInvalidArgumentException format:@"%@ can not initWithCoder:%@",isa,[coder class]];
   }
   return self;
}

-initWithTitle:(NSString *)title {
   _title=[title copy];
   _itemArray=[NSMutableArray new];
   _autoenablesItems=YES;
   return self;
}

-init {
   return [self initWithTitle:@""];
}

-initApplicationMenu:(NSString*)appName {
   [self init];
   NSMenuItem *quitItem = [[[NSMenuItem alloc] initWithTitle:[@"Quit " stringByAppendingString:appName] action:@selector(terminate:) keyEquivalent:@"q"] autorelease];
   [self addItem: quitItem];
   [self setValue:@"NSAppleMenu" forKey:@"name"];
   return [self autorelease];
}

-(void)dealloc {
   [_title release];
   [_name release];
   [_itemArray makeObjectsPerformSelector:@selector(_setMenu:) withObject:nil];
   [_itemArray release];
   [super dealloc];
}

-copyWithZone:(NSZone *)zone {
	NSMenu *copy=NSCopyObject(self, 0, zone);
	
	copy->_title=[_title copyWithZone:zone];
	copy->_name=[_name copyWithZone:zone];
	copy->_itemArray = [[NSMutableArray alloc] init];
	for (NSMenuItem *item in _itemArray) {
		[copy addItem: [[item copyWithZone:zone] autorelease]];
    }
	
	return copy;
}

-(NSMenu *)supermenu {
   return _supermenu;
}

-(NSString *)title {
   return _title;
}

-(int)numberOfItems {
   return [_itemArray count];
}

-(NSArray *)itemArray {
   return _itemArray;
}

-(void)_setItemArray:itemArray {
	if(_itemArray!=itemArray)
	{
		[_itemArray release];
		_itemArray=[itemArray retain];
	}
}

-(BOOL)autoenablesItems {
   return _autoenablesItems;
}

-(NSMenuItem *)itemAtIndex:(int)index {
   return [_itemArray objectAtIndex:index];
}

-(NSMenuItem *)itemWithTag:(int)tag {
    int i,count=[_itemArray count];

    for(i=0;i<count;i++){
        NSMenuItem *item=[_itemArray objectAtIndex:i];

        if ([item tag] == tag)
            return item;
    }

    return nil;
}

-(NSMenuItem *)itemWithTitle:(NSString *)title {
   int i,count=[_itemArray count];

   for(i=0;i<count;i++){
    NSMenuItem *item=[_itemArray objectAtIndex:i];

    if([[item title] isEqualToString:title])
     return item;
   }

   return nil;
}

-(int)indexOfItem:(NSMenuItem *)item {
    return [_itemArray indexOfObjectIdenticalTo:item];
}

-(int)indexOfItemWithTag:(int)tag {
    int i,count=[_itemArray count];

    for (i=0; i<count; ++i)
        if ([[_itemArray objectAtIndex:i] tag] == tag)
            return i;

    return -1;
}

-(int)indexOfItemWithTitle:(NSString *)title {
    int i,count=[_itemArray count];

    for (i=0;i<count;i++)
        if ([[[_itemArray objectAtIndex:i] title] isEqualToString:title])
            return i;

    return -1;
}

-(int)indexOfItemWithRepresentedObject:object {
   int i,count=[_itemArray count];
   
   for(i=0;i<count;i++)
    if([[(NSMenuItem *)[_itemArray objectAtIndex:i] representedObject] isEqual:object])
     return i;

   return -1;
}

// needed this for NSApplication windowsMenu stuff, so i did 'em all..
-(int)indexOfItemWithTarget:(id)target andAction:(SEL)action {
    int i,count=[_itemArray count];

    for (i=0; i<count; ++i) {
        NSMenuItem *item = [_itemArray objectAtIndex:i];

        if ([item target] == target) {
            if (action == NULL)
                return i;
            else if ([item action] == action)
                return i;
        }
    }

    return -1;
}

-(int)indexOfItemWithSubmenu:(NSMenu *)submenu {
    int i, count=[_itemArray count];

    for (i = 0; i < count; ++i) 
        if ([[_itemArray objectAtIndex:i] submenu] == submenu)
            return i;

    return -1;
}

-(void)setSupermenu:(NSMenu *)value {
   _supermenu=value;
}

-(void)setTitle:(NSString *)title {
   title=[title copy];
   [_title release];
   _title=title;
}

-(void)setAutoenablesItems:(BOOL)flag {
   _autoenablesItems=flag;
}

-(void)addItem:(NSMenuItem *)item {
   [item performSelector:@selector(_setMenu:) withObject:self];
   [_itemArray addObject:item];
}

-(NSMenuItem *)addItemWithTitle:(NSString *)title action:(SEL)action keyEquivalent:(NSString *)keyEquivalent {
   NSMenuItem *item=[[[NSMenuItem alloc] initWithTitle:title action:action keyEquivalent:keyEquivalent] autorelease];

   [self addItem:item];

   return item;
}

-(void)removeAllItems {
   while([_itemArray count]>0)
    [self removeItem:[_itemArray lastObject]];
}

-(void)removeItem:(NSMenuItem *)item {
   [item performSelector:@selector(_setMenu:) withObject:nil];
   [_itemArray removeObjectIdenticalTo:item];
}

-(void)removeItemAtIndex:(int)index {
   [self removeItem:[_itemArray objectAtIndex:index]];
}

-(void)insertItem:(NSMenuItem *)item atIndex:(int)index {
   [item performSelector:@selector(_setMenu:) withObject:self];
   [_itemArray insertObject:item atIndex:index];
}

-(NSMenuItem *)insertItemWithTitle:(NSString *)title action:(SEL)action keyEquivalent:(NSString *)keyEquivalent atIndex:(int)index {
   NSMenuItem *item=[[[NSMenuItem alloc] initWithTitle:title action:action keyEquivalent:keyEquivalent] autorelease];

   [self insertItem:item atIndex:index];

   return item;
}

-(void)setSubmenu:(NSMenu *)submenu forItem:(NSMenuItem *)item {
   [item setSubmenu:submenu];
}

BOOL itemIsEnabled(NSMenuItem *item) {
    BOOL enabled=NO;
    
    if([item action]!=NULL){
        id target=[item target];
        
        target=[NSApp targetForAction:[item action] to:[item target] from:nil];
        
        if ((target == nil) || ![target respondsToSelector:[item action]]) {
            enabled = NO;
        } else if ([target respondsToSelector:@selector(validateMenuItem:)]) {
            enabled = [target validateMenuItem:item];
        } else if ([target respondsToSelector:@selector(validateUserInterfaceItem:)]) { // New validation scheme
            enabled = [target validateUserInterfaceItem:item];
        } else {
            enabled = YES;
        }
    } 

    return enabled;
}

-(void)update {
    if ([_delegate respondsToSelector:@selector(menuNeedsUpdate:)]) {
        [_delegate menuNeedsUpdate:self];
    }

    int i,count=[_itemArray count];
    
    for(i=0;i<count;i++){
        NSMenuItem *item=[_itemArray objectAtIndex:i];
        
        if(_autoenablesItems){
            BOOL enabled = itemIsEnabled(item) ? YES : NO;
            BOOL currentlyEnabled = [item isEnabled] ? YES : NO;
            
            if(enabled!=currentlyEnabled && ![item _binderForBinding:@"enabled" create:NO]){
                [item setEnabled:enabled];
                [self itemChanged:item];
            }
        }
        
        [[item submenu] update];
    }
}

-(void)itemChanged:(NSMenuItem *)item {
}

-(BOOL)performKeyEquivalent:(NSEvent *)event {
    int       i,count=[_itemArray count];
    NSString *characters=[event charactersIgnoringModifiers];
    unsigned  modifiers=[event modifierFlags];
    
    if (_autoenablesItems)
        [self update];
    
    for(i=0;i<count;i++){
        NSMenuItem *item=[_itemArray objectAtIndex:i];
        unsigned    itemModifiers=[item keyEquivalentModifierMask]&(NSCommandKeyMask|NSAlternateKeyMask);
        NSString *key=[item keyEquivalent];
        
        if((modifiers&(NSCommandKeyMask|NSAlternateKeyMask))==itemModifiers){
            
            if([key isEqualToString:characters]){
                /* This *must* accurately reflect menu validation when ignoring or processing
                    key equivalents. Relying on update to keep isEnabled in the proper state is
                    unfortunately too tenuous.
                 */
                if (itemIsEnabled(item))
                    return [NSApp sendAction:[item action] to:[item target] from:item];
            }
        }
        
        if([[item submenu] performKeyEquivalent:event])
            return YES;
    }
    
    return NO;
}

-(BOOL)performClickEquivalent:(NSMenuItem *)item {
    if(_autoenablesItems)
        [self update];

    if(itemIsEnabled(item))
        return [NSApp sendAction:[item action] to:[item target] from:item];

    return NO;
}


- (void)setDelegate:(id <NSMenuDelegate>)object
{
	_delegate = object;
}

- (id<NSMenuDelegate>)delegate
{
	return _delegate;
}

-(NSString *)_name {
   return _name;
}

-(NSMenu *)_menuWithName:(NSString *)name {
   if([_name isEqual:name])
    return self;
   else {
    int i,count=[_itemArray count];
    
    for(i=0;i<count;i++){
     NSMenu *check=[[[_itemArray objectAtIndex:i] submenu] _menuWithName:name];
     
     if(check!=nil)
      return check;
    }
   }   
   
   return nil;
}

-(int)DBusItemID {
    return _DBusItemID;
}

-(void)setDBusItemID:(int)itemID {
    _DBusItemID=itemID;
}


@end
