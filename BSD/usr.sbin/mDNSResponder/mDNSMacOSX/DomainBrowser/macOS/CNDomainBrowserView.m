/*
 *
 * Copyright (c) 2016 Apple Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#import "CNDomainBrowserView.h"
#import "_CNDomainBrowser.h"
#import "CNDomainBrowserPathUtils.h"

#define BROWSER_CELL_SPACING                4
#define INITIAL_LEGACYBROWSE                1

@implementation NSBrowser(PathArray)

- (NSArray *)pathArrayToColumn:(NSInteger)column includeSelectedRow:(BOOL)includeSelection
{
	NSMutableArray * pathArray = [NSMutableArray array];
	if (!includeSelection) column--;
	for (NSInteger c = 0 ; c <= column ; c++)
	{
		NSBrowserCell *cell = [self selectedCellInColumn: c];
		if (cell) [pathArray addObject: [cell stringValue]];
	}
	
	return(pathArray);
}

@end

@interface CNDomainBrowserView ()

@property (strong) _CNDomainBrowser *      bonjour;

@property (strong) NSTableView	*			instanceTable;
@property (strong) NSArrayController *		instanceC;
@property (strong) NSTableColumn *			instanceNameColumn;
@property (strong) NSTableColumn *			instanceServiceTypeColumn;
@property (strong) NSTableColumn *			instancePathPopupColumn;

@property (strong) NSBrowser *				browser;
#if INITIAL_LEGACYBROWSE
@property (assign) BOOL                     initialPathSet;
#endif

@end

@implementation CNDomainBrowserView

- (instancetype)initWithFrame:(NSRect)frameRect
{
	if (self = [super initWithFrame: frameRect])
	{
		[self commonInit];
	}
	return(self);
}

- (nullable instancetype)initWithCoder:(NSCoder *)coder
{
	if (self = [super initWithCoder: coder])
	{
		[self commonInit];
	}
	return(self);
}

- (void)awakeFromNib
{
    [super awakeFromNib];
    
    self.bonjour = [[_CNDomainBrowser alloc] initWithDelegate:(id<_CNDomainBrowserDelegate>)self];
    _bonjour.browseRegistration = _browseRegistration;
    _bonjour.ignoreLocal = _ignoreLocal;
    _bonjour.ignoreBTMM = _ignoreBTMM;
}


- (void)contentViewsInit
{
	NSRect	frame = self.frame;
	self.instanceC = [[NSArrayController alloc] init];
    
	//	Bottom browser
    frame.origin.x = frame.origin.y = 0;
	NSBrowser * browserView = [[NSBrowser alloc] initWithFrame: frame];
	browserView.delegate = (id<NSBrowserDelegate>)self;
	browserView.action = @selector(clickAction:);
	browserView.titled = NO;
	browserView.separatesColumns = NO;
	browserView.allowsEmptySelection = YES;
	browserView.allowsMultipleSelection = NO;
	browserView.takesTitleFromPreviousColumn = NO;
	browserView.hasHorizontalScroller = YES;
	browserView.columnResizingType = NSBrowserNoColumnResizing;
	browserView.minColumnWidth = 50;
    browserView.translatesAutoresizingMaskIntoConstraints = NO;
	self.browser = browserView;

    [self addSubview: browserView];
    
    [self addConstraint:
     [NSLayoutConstraint constraintWithItem:_browser
                                  attribute:NSLayoutAttributeLeft
                                  relatedBy:NSLayoutRelationEqual
                                     toItem:self
                                  attribute:NSLayoutAttributeLeft
                                 multiplier:1
                                   constant:0]];
    [self addConstraint:
     [NSLayoutConstraint constraintWithItem:_browser
                                  attribute:NSLayoutAttributeRight
                                  relatedBy:NSLayoutRelationEqual
                                     toItem:self
                                  attribute:NSLayoutAttributeRight
                                 multiplier:1
                                   constant:0]];
    [self addConstraint:
     [NSLayoutConstraint constraintWithItem:_browser
                                  attribute:NSLayoutAttributeBottom
                                  relatedBy:NSLayoutRelationEqual
                                     toItem:self
                                  attribute:NSLayoutAttributeBottom
                                 multiplier:1
                                   constant:0]];
    [self addConstraint:
     [NSLayoutConstraint constraintWithItem:_browser
                                  attribute:NSLayoutAttributeTop
                                  relatedBy:NSLayoutRelationEqual
                                     toItem:self
                                  attribute:NSLayoutAttributeTop
                                 multiplier:1
                                   constant:0]];
}

- (void)commonInit
{
	[self contentViewsInit];
}

- (void)viewWillMoveToSuperview:(NSView *)newSuperview
{
    [super viewWillMoveToSuperview: newSuperview];
    if (newSuperview && !_bonjour)
    {
        [self awakeFromNib];
    }
}

- (void)setDomainSelectionToPathArray:(NSArray *)pathArray
{
	NSInteger column = 0;
	for (NSString * nextPathComponent in pathArray)
	{
		NSArray * subPath = [self.browser pathArrayToColumn: column includeSelectedRow: NO];
		NSArray * rowArray = [[self.bonjour subDomainsAtDomainPath: subPath] sortedArrayUsingComparator: ^(id obj1, id obj2) {
			return (NSComparisonResult)[ obj1[_CNSubDomainKey_subPath] compare: obj2[_CNSubDomainKey_subPath]];
		}];
		NSInteger nextRow = [rowArray indexOfObjectPassingTest: ^BOOL(id obj, NSUInteger index, BOOL *stop) {
			(void)index;
			(void)stop;
			return [obj[_CNSubDomainKey_subPath] isEqualToString: nextPathComponent];
		}];
		[self.browser selectRow: nextRow inColumn: column++];
	}
}

- (NSInteger)maxNumberOfVisibleSubDomainRows
{
	NSInteger  result = 0;
	
	for (NSInteger i = self.browser.firstVisibleColumn ; i <= self.browser.lastVisibleColumn ; i++)
	{
		NSInteger rows = [self browser: self.browser numberOfRowsInColumn: i];
		result = MAX(rows, result);
	}
	
	return(result);
}

#pragma mark - Public Methods

- (void)setIgnoreLocal:(BOOL)ignoreLocal
{
    _ignoreLocal = ignoreLocal;
    self.bonjour.ignoreLocal = _ignoreLocal;
}

- (void)setBrowseRegistration:(BOOL)browseRegistration
{
    _browseRegistration = browseRegistration;
    self.bonjour.browseRegistration = _browseRegistration;
}

- (NSString *)selectedDNSDomain
{
    NSArray * pathArray = [self.browser pathArrayToColumn: self.browser.selectedColumn includeSelectedRow: YES];
    return(DomainPathToDNSDomain(pathArray));
}

- (NSString *)defaultDNSDomain
{
    return(DomainPathToDNSDomain(self.bonjour.defaultDomainPath));
}

- (NSArray *)flattenedDNSDomains
{
    return(self.bonjour.flattenedDNSDomains);
}


- (void)startBrowse
{
    [self.bonjour startBrowser];
}

- (void)stopBrowse
{
    [self.bonjour stopBrowser];
    _initialPathSet = NO;
}

- (BOOL)isBrowsing
{
    return(self.bonjour.isBrowsing);
}

- (CGFloat)minimumHeight
{
    return self.selectedDNSDomain.length ? [self.browser frameOfRow: [self.browser selectedRowInColumn: self.browser.lastVisibleColumn] inColumn: self.browser.lastVisibleColumn].size.height : 0.0;
}

- (void)showSelectedRow
{
    for( NSInteger i = self.browser.firstVisibleColumn ; i <= self.browser.lastVisibleColumn ; i++ )
    {
        NSInteger selRow = [self.browser selectedRowInColumn: i];
        if( selRow != NSNotFound ) [self.browser scrollRowToVisible: selRow inColumn: i];
    }
}

- (BOOL)foundInstanceInMoreThanLocalDomain
{
    return( [_bonjour foundInstanceInMoreThanLocalDomain] );
}


#pragma mark - Notifications

- (void)browser:(NSBrowser *)sender selectionDidChange:(NSArray *)pathArray
{
    if (_delegate && [_delegate respondsToSelector: @selector(domainBrowserDomainSelected:)] &&
        sender == self.browser)
    {
        [_delegate domainBrowserDomainSelected: pathArray ? DomainPathToDNSDomain(pathArray) : nil];
    }
}

#pragma mark - NSBrowserDelegate

- (NSInteger)browser:(NSBrowser *)sender numberOfRowsInColumn:(NSInteger)column
{
	return ([self.bonjour subDomainsAtDomainPath: [sender pathArrayToColumn: column includeSelectedRow: NO]].count);
}

- (void)browser:(NSBrowser *)sender willDisplayCell:(id)cell atRow:(NSUInteger)row column:(NSInteger)column
{
	//	Get the name
	NSMutableArray * pathArray = [NSMutableArray arrayWithArray: [sender pathArrayToColumn: column includeSelectedRow: NO]];
	NSArray * rowArray = [[self.bonjour subDomainsAtDomainPath: pathArray] sortedArrayUsingComparator: ^(id obj1, id obj2) {
		return (NSComparisonResult)[ obj1[_CNSubDomainKey_subPath] compare: obj2[_CNSubDomainKey_subPath]];
	}];
	if (row < rowArray.count)
	{
		NSDictionary *	item = [rowArray objectAtIndex: row];
		NSString *val = item[_CNSubDomainKey_subPath];
		[cell setStringValue: val];
		
		//	See if it's a leaf
		[pathArray addObject: val];
		((NSBrowserCell*)cell).leaf = (![self.bonjour subDomainsAtDomainPath: pathArray].count);
		
		//	Make Default domain bold
		if ([item[_CNSubDomainKey_defaultFlag] boolValue])	((NSBrowserCell*)cell).font = [NSFont boldSystemFontOfSize: [NSFont systemFontSizeForControlSize: sender.controlSize]];
		else                                                ((NSBrowserCell*)cell).font = [NSFont controlContentFontOfSize: [NSFont systemFontSizeForControlSize: sender.controlSize]];
	}
}

- (CGFloat)browser:(NSBrowser *)sender shouldSizeColumn:(NSInteger)column forUserResize:(BOOL)forUserResize toWidth:(CGFloat)suggestedWidth
{
	(void)forUserResize;
	CGFloat newSize = 0;
	
	NSArray * pathArray = [NSArray arrayWithArray: [sender pathArrayToColumn: column includeSelectedRow: NO]];
	NSArray * rowArray = [[self.bonjour subDomainsAtDomainPath: pathArray] sortedArrayUsingComparator: ^(id obj1, id obj2) {
		return (NSComparisonResult)[ obj1[_CNSubDomainKey_subPath] compare: obj2[_CNSubDomainKey_subPath]];
	}];
    
	for (NSDictionary * next in rowArray)
	{
		NSFont * font = [next[_CNSubDomainKey_defaultFlag] boolValue] ?
						[NSFont boldSystemFontOfSize: [NSFont systemFontSizeForControlSize: sender.controlSize]]:
						[NSFont controlContentFontOfSize: [NSFont systemFontSizeForControlSize: sender.controlSize]];
        NSArray * itemArray = [pathArray arrayByAddingObjectsFromArray: [NSArray arrayWithObject: next[_CNSubDomainKey_subPath]]];
        NSBrowserCell * cell = [[NSBrowserCell alloc] initTextCell: next[_CNSubDomainKey_subPath]];
        cell.font = font;
        cell.leaf = ([self.bonjour subDomainsAtDomainPath: itemArray].count == 0);
		newSize = MAX(newSize, cell.cellSize.width + BROWSER_CELL_SPACING);
	}
	
	if (!newSize) newSize = suggestedWidth;
	newSize = (NSInteger)(newSize + 0.5);
	
	return(newSize);
}

#pragma mark - _CNDomainBrowser Delegates

- (void)bonjourBrowserDomainUpdate:(NSArray *)defaultDomainPath
{
	(void)defaultDomainPath;
    [self.browser loadColumnZero];
#if INITIAL_LEGACYBROWSE
    if( !_initialPathSet )
    {
        _initialPathSet = YES;
        [_delegate domainBrowserDomainUpdate: [NSString string]];
    }
    else
#endif
    {
        [self setDomainSelectionToPathArray: self.bonjour.defaultDomainPath];
        if (_delegate && [_delegate respondsToSelector: @selector(domainBrowserDomainUpdate:)])
        {
            [_delegate domainBrowserDomainUpdate: defaultDomainPath ? DomainPathToDNSDomain(defaultDomainPath) : [NSString string]];
        }
    }
}

#pragma mark - Commands

- (IBAction)clickAction:(id)sender
{
	(void)sender;
	NSArray * pathArray = [self.browser pathArrayToColumn: self.browser.selectedColumn includeSelectedRow: YES];
    if (!pathArray.count && (([NSEvent modifierFlags] & NSEventModifierFlagOption ) != NSEventModifierFlagOption)) pathArray = self.bonjour.defaultDomainPath;
    [self setDomainSelectionToPathArray: pathArray];
    [self browser: self.browser selectionDidChange: pathArray];
}

@end

@interface CNBonjourDomainCell()

@property(strong)   NSMutableArray *   browserCells;

@end

@implementation CNBonjourDomainCell

- (instancetype)init
{
	self = [super init];
	if (self)
	{
		self.browserCells = [NSMutableArray array];
	}
	return self;
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    self = [super initWithCoder:coder];
    if (self)
    {
        self.browserCells = [NSMutableArray array];
    }
    return self;
}

- (id)copyWithZone:(NSZone *)zone
{
    CNBonjourDomainCell *cell = [super copyWithZone: zone];
    if (cell)
    {
        cell.browserCells = [NSMutableArray arrayWithArray: self.browserCells];
    }
    return cell;
}

- (void) setObjectValue:(id)objectValue
{
    [super setObjectValue: objectValue];
    
    [self.browserCells removeAllObjects];
    if ([objectValue isKindOfClass: [NSString class]])
    {
        NSUInteger  count = 0;
        NSArray * subPaths = DNSDomainToDomainPath(objectValue);
        for (NSString * nextPath in subPaths)
        {
            NSBrowserCell * nextCell = [[NSBrowserCell alloc] initTextCell: nextPath];
            nextCell.leaf = (++count == subPaths.count);
            [self.browserCells addObject: nextCell];
        }
    }
}

- (void)drawInteriorWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
    CGFloat    usedWidth = BROWSER_CELL_SPACING / 2;
    for (NSBrowserCell * nextCell in self.browserCells)
    {
        NSRect nextRect = cellFrame;
        nextRect.size.width = cellFrame.size.width - usedWidth;
        nextRect.origin.x += usedWidth;

        NSSize  cellSize = [nextCell cellSizeForBounds: nextRect];
		CGFloat yOffset = (nextRect.size.height - cellSize.height) / 2;
        nextRect.size.width = cellSize.width;
        nextRect.size.height = cellSize.height;
		nextRect.origin.y += yOffset;

        [nextCell drawInteriorWithFrame: nextRect
                                 inView: controlView];
        usedWidth += nextRect.size.width + BROWSER_CELL_SPACING;
    }
}

@end

@interface CNBonjourDomainView()

@property(strong)   CNBonjourDomainCell *	cell;

@end

@implementation CNBonjourDomainView

- (instancetype)initWithFrame:(NSRect)frameRect
{
	self = [super initWithFrame:frameRect];
	if (self)
	{
		self.cell = [[CNBonjourDomainCell alloc] init];
	}
	return self;
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
	self = [super initWithCoder:coder];
	if (self)
	{
		self.cell = [[CNBonjourDomainCell alloc] init];
	}
	return self;
}

- (void) setDomain:(NSString *)domain
{
    if (![domain isEqualToString: self.cell.stringValue])
    {
        self.cell.stringValue = domain;
        self.needsDisplay = YES;
    }
}

- (NSString *)domain
{
	return self.cell.stringValue;
}

- (void) drawRect:(NSRect)dirtyRect
{
	(void)dirtyRect;	// Unused
	[self.cell drawInteriorWithFrame: self.bounds inView: self];
}

@end
