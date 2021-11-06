/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSPopUpButton.h>
#import <AppKit/NSPopUpButtonCell.h>
#import <AppKit/NSMenu.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSObject+BindingSupport.h>

NSString * const NSPopUpButtonWillPopUpNotification=@"NSPopUpButtonWillPopUpNotification";

static NSString * const NSPopUpButtonBindingObservationContext=@"NSPopUpButtonBindingObservationContext";

@implementation NSPopUpButton

+(Class)cellClass {
   return [NSPopUpButtonCell class];
}

-initWithFrame:(NSRect)frame pullsDown:(BOOL)pullsDown {
   [super initWithFrame:frame];
   [self setPullsDown:pullsDown];
   return self;
}

-(void)dealloc {
   NS_DURING
    [self removeObserver:self forKeyPath:@"cell.selectedItem"];
    [self removeObserver:self forKeyPath:@"cell.menu.itemArray"];
   NS_HANDLER
   NS_ENDHANDLER
	
   [super dealloc];
}

-(BOOL)pullsDown {
   return [_cell pullsDown];
}

-(NSMenu *)menu {
   return [_cell menu];
}

-(BOOL)autoenablesItems {
   return [_cell autoenablesItems];
}

-(NSRectEdge)preferredEdge {
   return [_cell preferredEdge];
}

-(NSArray *)itemArray {
   return [_cell itemArray];
}

-(NSInteger)numberOfItems {
   return [_cell numberOfItems];
}

-(NSMenuItem *)itemAtIndex:(NSInteger)index {
   return [_cell itemAtIndex:index];
}

-(NSMenuItem *)itemWithTitle:(NSString *)title {
   return [_cell itemWithTitle:title];
}

-(NSMenuItem *)lastItem {
   return [_cell lastItem];
}

-(NSInteger)indexOfItem:(NSMenuItem *)item {
   return [_cell indexOfItem:item];
}

-(NSInteger)indexOfItemWithTitle:(NSString *)title {
   return [_cell indexOfItemWithTitle:title];
}

-(NSInteger)indexOfItemWithTag:(NSInteger)tag {
   return [_cell indexOfItemWithTag:tag];
}

-(NSInteger)indexOfItemWithRepresentedObject:object {
   return [_cell indexOfItemWithRepresentedObject:object];
}

-(NSInteger)indexOfItemWithTarget:target andAction:(SEL)action {
   return [_cell indexOfItemWithTarget:target andAction:action];
}

-(NSMenuItem *)selectedItem {
   return [_cell selectedItem];
}

-(NSString *)titleOfSelectedItem {
   return [_cell titleOfSelectedItem];
}

-(NSInteger)selectedTag {
   return [_cell tag];
}

-(NSInteger)indexOfSelectedItem {
   return [_cell indexOfSelectedItem];
}

-(void)setPullsDown:(BOOL)flag {
   [_cell setPullsDown:flag];
   [self setNeedsDisplay:YES];
}

-(void)setMenu:(NSMenu *)menu {
   [_cell setMenu:menu];
   [self setNeedsDisplay:YES];
}

-(void)setAutoenablesItems:(BOOL)value {
   [_cell setAutoenablesItems:value];
}

-(void)setPreferredEdge:(NSRectEdge)edge {
   [_cell setPreferredEdge:edge];
}

-(void)addItemWithTitle:(NSString *)title {
   [_cell addItemWithTitle:title];
   [self setNeedsDisplay:YES];
}

-(void)addItemsWithTitles:(NSArray *)titles {
   [_cell addItemsWithTitles:titles];
   [self setNeedsDisplay:YES];
}

-(void)removeAllItems {
   [_cell removeAllItems];
   [self setNeedsDisplay:YES];
}

-(void)removeItemAtIndex:(NSInteger)index {
   [_cell removeItemAtIndex:index];
   [self setNeedsDisplay:YES];
}

-(void)removeItemWithTitle:(NSString *)title {
   [_cell removeItemWithTitle:title];
   [self setNeedsDisplay:YES];
}

-(void)insertItemWithTitle:(NSString *)title atIndex:(NSInteger)index {
   [_cell insertItemWithTitle:title atIndex:index];
   [self setNeedsDisplay:YES];
}

-(void)selectItem:(NSMenuItem *)item {
   [_cell selectItem:item];
   [self setNeedsDisplay:YES];
}

-(void)selectItemAtIndex:(NSInteger)index {
   [_cell selectItemAtIndex:index];
   [self setNeedsDisplay:YES];
}

-(void)selectItemWithTitle:(NSString *)title {
   [_cell selectItemWithTitle:title];
   [self setNeedsDisplay:YES];
}

-(BOOL)selectItemWithTag:(NSInteger)tag {
   [self setNeedsDisplay:YES];
   return [_cell selectItemWithTag:tag];
}

-(NSString *)itemTitleAtIndex:(NSInteger)index {
   return [_cell itemTitleAtIndex:index];
}

-(NSArray *)itemTitles {
   return [_cell itemTitles];
}

- (void)setTitle:(NSString *)title
{
    if ([self pullsDown]) {
        // The title gets stored in the zero index item in the menu - it made sense to Apple at some point...
        [[_cell itemAtIndex: 0] setTitle: title];
        [self synchronizeTitleAndSelectedItem];
    } else {
        [super setTitle: title];
    }
}

-(void)synchronizeTitleAndSelectedItem {
   [_cell synchronizeTitleAndSelectedItem];
   [self setNeedsDisplay:YES];
}

-(void)performClick:sender {
   [self lockFocus];
   if([_cell trackMouse:[NSApp currentEvent] inRect:[self bounds] ofView:self untilMouseUp:YES]){
    NSMenuItem *item=[self selectedItem];
    SEL         action=[item action];
    id          target=[item target];

	   if (action != NULL) {
		   // The item has an explicit action - so it's going to be the sender
		   sender = item;
	   }
	   
    [_cell setState:![_cell state]];
    [self setNeedsDisplay:YES];

    if(action==NULL){
     action=[self action];
     target=[self target];
    }
    else if(target==nil){
     target=[self target];
    }

    [NSApp sendAction:action to:target from: sender];
   }

   [self unlockFocus];

}

-(void)mouseDown:(NSEvent *)event {
   if(![self isEnabled])
    return;

   [self performClick:self];
}

- (void)keyDown:(NSEvent *)event {
    [self interpretKeyEvents:[NSArray arrayWithObject:event]];
}

// this gets us arrow keys to select items in the menu w/o popping it up.
- (void)moveUp:(id)sender {
    [_cell moveUp:sender];
    [self setNeedsDisplay:YES];
}

- (void)moveDown:(id)sender {
    [_cell moveDown:sender];
    [self setNeedsDisplay:YES];
}

- (void)insertNewline:(id)sender {
    [self mouseDown:nil];
}

@end


@implementation NSPopUpButton (BindingSupport)

-(void)_setItemValues:(NSArray *)values forKey:(NSString *)key {

    [_cell removeAllItems];
    [_cell addItemsWithTitles:values];
    
	if ([self indexOfSelectedItem] >= values.count) {
		[self selectItem:nil];
	}
	[self synchronizeTitleAndSelectedItem];
}

-(id)_contentValues
{
	return [self valueForKeyPath:@"itemArray.title"];
}

// FIXME: is it contentValues or contentObjects, or both?
-(void)_setContentValues:(NSArray*)values
{
	[self _setItemValues:values forKey:@"title"];
}

-(id)_contentObjects {
	return [self valueForKeyPath:@"itemArray.title"];
}

-(void)_setContentObjects:(NSArray *)objects {
	[self _setItemValues:objects forKey:@"title"];
}

-(id)_content
{
	return [self valueForKeyPath:@"itemArray.representedObject"];
}

-(void)_setContent:(NSArray*)values
{
	[self _setItemValues:values forKey:@"representedObject"];
	if(![self _binderForBinding:@"contentValues"])
	{
		[self _setItemValues: [values valueForKey: @"description"] forKey:@"title"];
	}
}


-(NSInteger)_selectedTag {
   return [[_cell selectedItem] tag];
}

-(void)_setSelectedTag:(NSInteger)tag {
   int index = [_cell indexOfItemWithTag:tag];
   
   if (index >= 0)
    [_cell selectItemAtIndex:index];
}


-(NSUInteger)_selectedIndex
{
	return [self indexOfSelectedItem];
}

-(void)_setSelectedIndex:(NSUInteger)idx
{
	[self selectItemAtIndex:idx];
}

-(id)_selectedValue
{
	return [self titleOfSelectedItem];
}

-(void)_setSelectedValue:(id)value
{
	if (value && ![value isKindOfClass:[NSString class]]) {
		// Cocoa actually accepts non string values
		value = [NSString stringWithFormat:@"%@", value];
	}
	[self selectItemWithTitle:value];
}

- (void) bind:(NSString *)binding toObject:(id)observable withKeyPath:(NSString *)keyPath options:(NSDictionary *)options
{
	// No need to observe the same thing many times when we have several bindings
	if (!_observerAdded) {
		_observerAdded = YES;
		[self addObserver:self 
			   forKeyPath:@"cell.menu.itemArray" 
				  options:NSKeyValueObservingOptionPrior
				  context:NSPopUpButtonBindingObservationContext];
		
		[self addObserver:self 
			   forKeyPath:@"cell.selectedItem" 
				  options:NSKeyValueObservingOptionPrior
				  context:NSPopUpButtonBindingObservationContext];
	}
	[super bind:binding toObject:observable withKeyPath:keyPath options:options];
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if (context == NSPopUpButtonBindingObservationContext) {
		if([keyPath isEqualToString:@"cell.selectedItem"])
		{
			if([[change objectForKey:NSKeyValueChangeNotificationIsPriorKey] boolValue])
			{
				[self willChangeValueForKey:@"selectedIndex"];
            [self willChangeValueForKey:@"selectedValue"];
            [self willChangeValueForKey:@"selectedObject"];
				[self willChangeValueForKey:@"selectedTag"];

			}
			else
			{
            [self didChangeValueForKey:@"selectedObject"];
            [self didChangeValueForKey:@"selectedValue"];
				[self didChangeValueForKey:@"selectedIndex"];
				[self didChangeValueForKey:@"selectedTag"];
			}
		}
		else
		{
			if([[change objectForKey:NSKeyValueChangeNotificationIsPriorKey] boolValue])
			{
				[self willChangeValueForKey:@"contentValues"];
			}
			else
			{
				[self didChangeValueForKey:@"contentValues"];
			}
		}
	}
	else {
		[super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	}
}
@end
