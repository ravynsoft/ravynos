/* Copyright (c) 2006-2009 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSToolbar.h>
#import <AppKit/NSToolbarItem.h>
#import <AppKit/NSToolbarView.h>
#import <AppKit/NSToolbarCustomizationPalette.h>
#import <AppKit/NSView.h>
#import <AppKit/NSWindow-Private.h>
#import <AppKit/NSThemeFrame.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSParagraphStyle.h>
#import <AppKit/NSAttributedString.h>
#import <AppKit/NSFont.h>
#import <AppKit/NSRaise.h>

NSSize _NSToolbarSizeRegular = { 40, 40 };
NSSize _NSToolbarSizeSmall = { 32, 32 };
NSSize _NSToolbarIconSizeRegular = { 32, 32 };
NSSize _NSToolbarIconSizeSmall = { 24, 24 };

NSString * const NSToolbarWillAddItemNotification = @"NSToolbarWillAddItemNotification";
NSString * const NSToolbarDidRemoveItemNotification = @"NSToolbarDidRemoveItemNotification";

// private inter-toolbar notification
NSString * const NSToolbarChangeAppearanceNotification = @"__NSToolbarChangeAppearanceNotification";

@interface NSWindow(NSToolbarPrivate)
-(void)_toolbarSizeDidChangeFromOldHeight:(CGFloat)oldHeight;
@end

@interface NSToolbarItem(private)
-(void)_setToolbar:(NSToolbar *)toolbar;
@end

@implementation NSToolbar

-(NSDictionary *)_labelAttributesForSizeMode:(NSToolbarSizeMode)sizeMode {
   NSMutableParagraphStyle *style = [[[NSParagraphStyle defaultParagraphStyle] mutableCopy] autorelease];
    
   [style setLineBreakMode:NSLineBreakByClipping];
   [style setAlignment:NSCenterTextAlignment];
 
   switch (sizeMode) {
    case NSToolbarSizeModeSmall:
     return [NSDictionary dictionaryWithObjectsAndKeys:
                [NSFont systemFontOfSize:10.0], NSFontAttributeName,
                style, NSParagraphStyleAttributeName,
                nil];
            
    case NSToolbarSizeModeRegular:
    case NSToolbarSizeModeDefault:
    default:
     return [NSDictionary dictionaryWithObjectsAndKeys:
                [NSFont systemFontOfSize:11.0], NSFontAttributeName,
                style, NSParagraphStyleAttributeName,
                nil];
   }        
}

-(NSDictionary *)_labelAttributes {
   return [self _labelAttributesForSizeMode:_sizeMode];
}

-initWithIdentifier:(NSString *)identifier {
   _identifier=[identifier copy];
   _delegate=nil;
   _items=[[NSMutableArray alloc] init];
   _selectedItemIdentifier=nil;
   _allowedItems=[[NSMutableArray alloc] init];
   _defaultItems=[[NSMutableArray alloc] init];
   _selectableItems=[[NSMutableArray alloc] init];
   _identifiedItems=[[NSMutableDictionary alloc] init];
   _window=nil;   
   _view=[[NSToolbarView alloc] init];
   [_view setToolbar:self];
   _palette=nil;
   _sizeMode=NSToolbarSizeModeDefault;
   _displayMode=NSToolbarDisplayModeDefault;
   _autosavesConfiguration=NO;
   _visible=YES;
   _allowsUserCustomization=YES;
   _isLoadingConfiguration=NO;
   _loadDefaultItems=YES;
   
   [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(toolbarChangedAppearance:) name:NSToolbarChangeAppearanceNotification object:nil];
   // these cause notification loops
//   [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(toolbarWillAddItem:) name:NSToolbarWillAddItemNotification object:nil];
//   [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(toolbarDidRemoveItem:) name:NSToolbarDidRemoveItemNotification object:nil];
   
   return self;
}

-(void)dealloc {
   [[NSNotificationCenter defaultCenter] removeObserver:self];
    
   [_identifier release];
   [_items release];
   [_allowedItems release];
   [_defaultItems release];
   [_selectableItems release];
   [_identifiedItems release];
   _window=nil;
   [_view release];
   _palette=nil;

   [super dealloc];
}

-(NSToolbarView *)_view {
   return _view;
}

-(CGFloat)visibleHeight {
   if(!_visible)
    return 0;
   
   return [_view frame].size.height;
}

-(NSString *)_configurationKey {
   return [NSString stringWithFormat:@"NSToolbar identifier=%@", _identifier];
}

-(BOOL)loadConfiguration {
   NSDictionary *dictionary=[[NSUserDefaults standardUserDefaults] objectForKey:[self _configurationKey]];
    
   if(dictionary!=nil){
    _isLoadingConfiguration=YES;
    [self setConfigurationFromDictionary:dictionary];        
    _isLoadingConfiguration=NO;
    return YES;
   }
    
   return NO;
}

-(void)saveConfiguration {    
   if(_isLoadingConfiguration==NO){
    [[NSUserDefaults standardUserDefaults] setObject:[self configurationDictionary] forKey:[self _configurationKey]];
   }
}

-(NSString *)identifier {
   return _identifier;
}

-delegate {
   return _delegate;
}

-(BOOL)isVisible {
   return _visible;
}

-(NSToolbarSizeMode)sizeMode {
   return _sizeMode;
}

-(NSToolbarDisplayMode)displayMode {
   return _displayMode;
}

-(BOOL)showsBaselineSeparator {
   return _showsBaselineSeparator;
}

-(NSArray *)items {
   return _items;
} 

-(NSArray *)visibleItems {
   return [_view visibleItems];
}

-(BOOL)autosavesConfiguration {
   return _autosavesConfiguration;
}

-(BOOL)allowsUserCustomization {
    return _allowsUserCustomization;
}

-(NSString *)selectedItemIdentifier {
   return _selectedItemIdentifier;
}

-(NSArray *)itemIdentifiers {
   return [_items valueForKey:@"itemIdentifier"];
}

-(NSDictionary *)configurationDictionary {
    NSDictionary *dictionary = [NSDictionary dictionaryWithObjectsAndKeys:
        [NSNumber numberWithInt:_displayMode], @"displayMode",
        [NSNumber numberWithInt:_sizeMode], @"sizeMode",
        [NSNumber numberWithBool:_visible], @"isVisible",
        [NSNumber numberWithBool:_autosavesConfiguration], @"autosavesConfiguration",
        [self itemIdentifiers], @"itemIdentifiers",
        nil];    
        
    return dictionary;
}

static BOOL isStandardItemIdentifier(NSString *identifier){
   if([identifier isEqualToString:NSToolbarCustomizeToolbarItemIdentifier])
    return YES;
   if([identifier isEqualToString:NSToolbarFlexibleSpaceItemIdentifier])
    return YES;
   if([identifier isEqualToString:NSToolbarPrintItemIdentifier])
    return YES;
   if([identifier isEqualToString:NSToolbarSeparatorItemIdentifier])
    return YES;
   if([identifier isEqualToString:NSToolbarShowColorsItemIdentifier])
    return YES;
   if([identifier isEqualToString:NSToolbarShowFontsItemIdentifier])
    return YES;
   if([identifier isEqualToString:NSToolbarSpaceItemIdentifier])
    return YES;

   return NO;
}

-(NSToolbarItem *)_itemForItemIdentifier:(NSString *)identifier willBeInsertedIntoToolbar:(BOOL)intoToolbar {
   NSToolbarItem *item=[_identifiedItems objectForKey:identifier];
   
   if(item==nil){
    // The delegate does not get toolbar:itemForItemIdentifier:willBeInsertedIntoToolbar: for the standard items
    BOOL standardItem=isStandardItemIdentifier(identifier);
    
    if(_delegate==nil || standardItem)
     item=[[[[self toolbarItemClass] alloc] initWithItemIdentifier:identifier] autorelease];
    else {
     item=[_delegate toolbar:self itemForItemIdentifier:identifier willBeInsertedIntoToolbar:intoToolbar];
    }
    
    if(item!=nil){
     [_identifiedItems setObject:item forKey:identifier];
     [item _setToolbar:self];
    }
   }
   
   return item;
}

-(NSArray *)_itemsWithIdentifiers:(NSArray*)identifiers {   
   NSMutableArray *result=[NSMutableArray array];
   
   for (NSString *identifier in identifiers) {
    NSToolbarItem *item=[self _itemForItemIdentifier:identifier willBeInsertedIntoToolbar:NO];
    
    if(item!=nil)
     [result addObject:item];
   }
   
   return result;
}

-(void)_insertItem:(NSToolbarItem *)item atIndex:(NSInteger)index {
   [[NSNotificationCenter defaultCenter] postNotificationName:NSToolbarWillAddItemNotification object:self userInfo:[NSDictionary dictionaryWithObjectsAndKeys:item, @"item", [NSNumber numberWithInteger:index],@"index",nil]];

   [_items insertObject:item atIndex:index];
   [_view _insertItem:item atIndex:index];
}

-(void)_removeItemAtIndex:(NSInteger)index {
   NSToolbarItem *item=[[[_items objectAtIndex:index] retain] autorelease];

   [_items removeObjectAtIndex:index];
   [_view _removeItemAtIndex:index];

   [[NSNotificationCenter defaultCenter] postNotificationName:NSToolbarDidRemoveItemNotification object:self userInfo:[NSDictionary dictionaryWithObjectsAndKeys:item, @"item", nil]];
}

-(void)toolbarWillAddItem:(NSNotification *)note {
   NSToolbar *toolbar=[note object];
   
   if(toolbar==self)
    return;
   if(![[toolbar identifier] isEqualToString:_identifier])
    return;

   NSToolbarItem *item=[[note userInfo] objectForKey:@"item"];

   if([_items containsObject:item])
    return;
    
   NSInteger      index=[[[note userInfo] objectForKey:@"index"] integerValue];
   
   [self _insertItem:item atIndex:index];
}

-(void)toolbarDidRemoveItem:(NSNotification *)note {
   NSToolbar *toolbar=[note object];
   
   if(toolbar==self)
    return;
   if(![[toolbar identifier] isEqualToString:_identifier])
    return;
    
   NSToolbarItem *item=[[note userInfo] objectForKey:@"item"];
   NSInteger      index=[[[note userInfo] objectForKey:@"index"] integerValue];

   [self _removeItemAtIndex:index];
}

-(void)insertItemWithItemIdentifier:(NSString *)identifier atIndex:(int)index {
   NSToolbarItem *item=[self _itemForItemIdentifier:identifier willBeInsertedIntoToolbar:YES];
    
   [self _insertItem:item atIndex:index];
   
   if([self autosavesConfiguration])
    [self saveConfiguration];
}

-(void)removeItemAtIndex:(int)index {
   [self _removeItemAtIndex:index];

   if([self autosavesConfiguration])
    [self saveConfiguration];
}

// called from drag&drop
-(void)_setItemsWithIdentifiersFromArray:(NSArray *)identifiers {
   [_items removeAllObjects];
   [_items addObjectsFromArray:[self _itemsWithIdentifiers:identifiers]];

   if([self autosavesConfiguration])
    [self saveConfiguration];
}

-(NSArray *)_defaultToolbarItems {
   NSArray *result=nil;

   if([_delegate respondsToSelector:@selector(toolbarDefaultItemIdentifiers:)])
    result=[self _itemsWithIdentifiers:[_delegate toolbarDefaultItemIdentifiers:self]];

   if(result==nil)
    result=_defaultItems;
    
   return result;
}

-(void)loadDefaultItemsIfNeeded {
   if(_loadDefaultItems){
    _loadDefaultItems=NO;
    
    NSArray *items=[self _defaultToolbarItems];
    
    while([_items count])
     [self _removeItemAtIndex:0];
 
    for(NSToolbarItem *item in items)
     [self _insertItem:item atIndex:[_items count]];
   }
}

-(void)toolbarChangedAppearance:(NSNotification *)note {
   NSToolbar *toolbar=[note object];

   if(toolbar==self)
    return;
    
   if(![[toolbar identifier] isEqualToString:_identifier])
    return;
   
   _sizeMode=[toolbar sizeMode];
   _displayMode=[toolbar displayMode];

   [_window _toolbarSizeDidChangeFromOldHeight:[self visibleHeight]];
}

-(void)layoutFrameSizeWithWidth:(CGFloat)width {
   [self loadDefaultItemsIfNeeded];
   [_view layoutViewsWithWidth:width setFrame:YES];
}

-(void)itemSizeDidChange {
   [_window _toolbarSizeDidChangeFromOldHeight:[self visibleHeight]];
}

-(void)setDelegate:delegate {
   struct {
    NSString *name;
    SEL       selector;
   } notes[] = {
    { NSToolbarWillAddItemNotification, @selector(toolbarWillAddItem:) },
    { NSToolbarDidRemoveItemNotification, @selector(toolbarDidRemoveItem:) },
    { nil, NULL }
   };
   NSInteger i;
    
   if(_delegate!=nil)
    for (i = 0; notes[i].name != nil; i++)
     [[NSNotificationCenter defaultCenter] removeObserver:_delegate name:notes[i].name object:self];
    
   BOOL isNew = (delegate != nil && delegate != _delegate);
    
   _delegate = delegate;
    
   for (i = 0; notes[i].name != nil; i++)
    if ([_delegate respondsToSelector:notes[i].selector])
     [[NSNotificationCenter defaultCenter] addObserver:_delegate selector:notes[i].selector name:notes[i].name object:self];
     
   if (isNew) {  // allow the new delegate to reset the layout
    _loadDefaultItems=YES;
    [self loadDefaultItemsIfNeeded];
    [_window _toolbarSizeDidChangeFromOldHeight:[self visibleHeight]];
   }
 }

-(void)didChangeAppearanceGlobal:(BOOL)global {
   if([self autosavesConfiguration])
    [self saveConfiguration];

   [_window _toolbarSizeDidChangeFromOldHeight:[self visibleHeight]];
   if(global)
    [[NSNotificationCenter defaultCenter] postNotificationName:NSToolbarChangeAppearanceNotification object:self];
}

-(void)setVisible:(BOOL)flag {
   _visible=flag;
   [self didChangeAppearanceGlobal:NO];
}

-(void)setSizeMode:(NSToolbarSizeMode)mode {
   _sizeMode=mode;
   [self didChangeAppearanceGlobal:YES];
}

-(void)setDisplayMode:(NSToolbarDisplayMode)mode {
    _displayMode = mode;
   [self didChangeAppearanceGlobal:YES];
}

-(void)setShowsBaselineSeparator:(BOOL)value {
   _showsBaselineSeparator=value;
   [self didChangeAppearanceGlobal:YES];
}

-(void)setConfigurationFromDictionary:(NSDictionary *)dictionary {
   NSToolbarDisplayMode displayMode = [[dictionary objectForKey:@"displayMode"] intValue];
   NSToolbarSizeMode    sizeMode = [[dictionary objectForKey:@"sizeMode"] intValue];
   BOOL                 visible = [[dictionary objectForKey:@"isVisible"] boolValue];
   BOOL                 autosavesConfiguration = [[dictionary objectForKey:@"autosavesConfiguration"] boolValue];
   NSArray             *identifiers = [dictionary objectForKey:@"itemIdentifiers"];
    

   [self setDisplayMode:displayMode];
   [self setSizeMode:sizeMode];
   
   _items=[[self _itemsWithIdentifiers:identifiers] mutableCopy];

   [self setVisible:visible];
   [self setAutosavesConfiguration:autosavesConfiguration];
}

-(void)setAutosavesConfiguration:(BOOL)flag {
    _autosavesConfiguration=flag;
}

-(void)setAllowsUserCustomization:(BOOL)flag {
   _allowsUserCustomization=flag;
}

-(void)setSelectedItemIdentifier:(NSString *)identifier {
   identifier=[identifier copy];
   [_selectedItemIdentifier release];
   _selectedItemIdentifier=identifier;
   [_view setNeedsDisplay:YES];
}

-(void)validateVisibleItems {
   NSArray *visibleItems=[self visibleItems];
   NSInteger i, count = [visibleItems count];
    
   for (i = 0; i < count;i++)
    [[visibleItems objectAtIndex:i] validate];
}

- (void)didSelectToolbarItem:(NSString*)identifier
{
    // Cocoa says the selectedItemIdentifier is updated only if the item is in the selectableItems
    if([_delegate respondsToSelector:@selector(toolbarSelectableItemIdentifiers:)]) {
        NSArray *selectableIdentifiers = [_delegate toolbarSelectableItemIdentifiers: self];
        if ([selectableIdentifiers containsObject: identifier]) {
            [self setSelectedItemIdentifier: identifier];
        }
    }
}

-(BOOL)customizationPaletteIsRunning {
   return _palette != nil;
}
 
-(void)sheetDidEnd:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo {
    [_palette orderOut:nil];
    [_palette release];
    _palette = nil;
}

-(void)runCustomizationPalette:sender {
   if (_palette != nil || _allowsUserCustomization == NO || _visible == NO)
    return;
    
   _palette = [[NSToolbarCustomizationPalette toolbarCustomizationPalette] retain];
   [_palette setToolbar:self];
    
   [NSApp beginSheet:_palette modalForWindow:_window modalDelegate:self didEndSelector:@selector(sheetDidEnd:returnCode:contextInfo:) contextInfo:NULL];
}

-(void)_setWindow:(NSWindow *)window {    
   _window=window;
}

-(NSArray *)_allowedToolbarItems {
   NSArray *result=nil;
   
   if([_delegate respondsToSelector:@selector(toolbarAllowedItemIdentifiers:)])
    result=[self _itemsWithIdentifiers:[_delegate toolbarAllowedItemIdentifiers:self]];

   if(result==nil)
    result=_allowedItems;
       
   return result;
}

-initWithCoder:(NSCoder *)coder {
   if(![coder allowsKeyedCoding])
      NSUnimplementedMethod();
   else {
      _identifier=[[coder decodeObjectForKey:@"NSToolbarIdentifier"] retain];
      [self setDelegate:[coder decodeObjectForKey:@"NSToolbarDelegate"]];
      _items=[[coder decodeObjectForKey:@"NSToolbarIBDefaultItems"] mutableCopy];
      _selectedItemIdentifier=nil;
      _allowedItems=[[coder decodeObjectForKey:@"NSToolbarIBAllowedItems"] retain];
      _defaultItems=[[coder decodeObjectForKey:@"NSToolbarIBDefaultItems"] retain];
      _selectableItems=[[coder decodeObjectForKey:@"NSToolbarIBSelectableItems"] retain];
      _identifiedItems=[[coder decodeObjectForKey:@"NSToolbarIBIdentifiedItems"] mutableCopy];
      [[_identifiedItems allValues] makeObjectsPerformSelector:@selector(_setToolbar:) withObject:self];
      _window=nil;
      _view = [[NSToolbarView alloc] init];
      [_view  setToolbar:self];
      _palette=nil;
      _sizeMode=[coder decodeIntForKey:@"NSToolbarSizeMode"];
      _displayMode=[coder decodeIntForKey:@"NSToolbarDisplayMode"];
      _autosavesConfiguration=[coder decodeBoolForKey:@"NSToolbarAutosavesConfiguration"];
      _visible=YES;
      _allowsUserCustomization=[coder decodeBoolForKey:@"NSToolbarAllowsUserCustomization"];
      _isLoadingConfiguration=NO;
      _loadDefaultItems=NO;
      [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(toolbarChangedAppearance:) name:NSToolbarChangeAppearanceNotification object:nil];
      /*
       NSToolbarPrefersToBeShown = 1;
       NSToolbarShowsBaselineSeparator = 1;
       */
   }
   return self;
}

-(void)postAwakeFromNib {
// this causes animation, display, frame changes, do post in initWithCoder:
   [[NSNotificationCenter defaultCenter] postNotificationName:NSToolbarChangeAppearanceNotification object:self];
}

@end

@implementation NSToolbar (NSToolbarCustomization)

- (Class)toolbarItemClass
{
	return [NSToolbarItem class];
}

@end

