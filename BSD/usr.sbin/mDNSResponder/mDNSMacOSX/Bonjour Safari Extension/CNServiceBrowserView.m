/*
 * Copyright (c) 2017-2019 Apple Inc. All rights reserved.
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

#import "CNServiceBrowserView.h"
#import "CNDomainBrowserPathUtils.h"
#include <dns_sd.h>

#import <SafariServices/SafariServices.h>

#define SHOW_SERVICETYPE_IF_SEARCH_COUNT	0

const NSString *    _CNInstanceKey_fullName             = @"fullName";
const NSString *    _CNInstanceKey_name                 = @"name";
const NSString *    _CNInstanceKey_serviceType          = @"serviceType";
const NSString *    _CNInstanceKey_domainPath           = @"domainPath";
const NSString *    _CNInstanceKey_resolveUrl           = @"resolveUrl";
const NSString *    _CNInstanceKey_resolveInstance      = @"resolveInstance";

@interface _DNSServiceRefWrapper : NSObject
{
    DNSServiceRef    _ref;
}

- (instancetype)initWithRef:(DNSServiceRef)ref;
@end

@implementation _DNSServiceRefWrapper

- (instancetype)initWithRef:(DNSServiceRef)ref
{
    if( self = [super init] )
    {
        _ref = ref;
    }
    return( self );
}

- (void)dealloc
{
    if( _ref ) DNSServiceRefDeallocate( _ref );
}

@end

@implementation NSArray( CaseInsensitiveStringArrayCompare )

- (BOOL)caseInsensitiveStringMatch:(NSArray *)inArray
{
    BOOL match = YES;
    
    if( self.count != [inArray count] )    match = NO;    //    Nil zero len ok
    else
    {
        NSInteger    i = 0;
        for( NSString * next in self )
        {
            NSString * inNext = inArray[i++];
            if( ![inNext isKindOfClass: [NSString class]] || ![next isKindOfClass: [NSString class]] )
            {
                match = NO;
                break;
            }
            else if( [next caseInsensitiveCompare: inNext] != NSOrderedSame )
            {
                match = NO;
                break;
            }
        }
    }
    
    return( match );
}

@end

@protocol CNServiceTypeLocalizerDelegate <NSObject>
@property (strong) NSDictionary * localizedServiceTypesDictionary;
@end

@interface CNServiceTypeLocalizer : NSValueTransformer
{
	id<CNServiceTypeLocalizerDelegate>	_delegate;
}
- (instancetype)initWithDelegate:(id<CNServiceTypeLocalizerDelegate>)delegate;

@end

@implementation CNServiceTypeLocalizer

- (instancetype)initWithDelegate:(id<CNServiceTypeLocalizerDelegate>)delegate
{
	if( self = [super init] )
	{
		_delegate = delegate;
	}
	return( self );
}

+ (Class)transformedValueClass
{
	return [NSString class];
}

+ (BOOL)allowsReverseTransformation
{
	return NO;
}

- (nullable id)transformedValue:(nullable id)value
{
	id	result = value;
	
	if( value && _delegate && [_delegate respondsToSelector: @selector(localizedServiceTypesDictionary)] )
	{
		NSString *	localizedValue = [_delegate.localizedServiceTypesDictionary objectForKey: value];
		if( localizedValue ) result = localizedValue;
	}
	
	return( result );
}

@end

@implementation NSBrowser( PathArray )

- (NSArray *)pathArrayToColumn:(NSInteger)column includeSelectedRow:(BOOL)includeSelection
{
	NSMutableArray * pathArray = [NSMutableArray array];
	if( !includeSelection ) column--;
	for( NSInteger c = 0 ; c <= column ; c++ )
	{
		NSBrowserCell *cell = [self selectedCellInColumn: c];
		if( cell ) [pathArray addObject: [cell stringValue]];
	}
	
	return( pathArray );
}

@end

static void resolveReply( DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char *fullname, const char *hosttarget, uint16_t port, uint16_t txtLen, const unsigned char *txtRecord, void *context );
static void browseReply( DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char *serviceName, const char *regtype, const char *replyDomain, void *context );

@interface CNServiceBrowserView ()

@property (strong) NSTableView	*			instanceTable;
@property (strong) NSArrayController *		instanceC;
@property (strong) NSTableColumn *			instanceNameColumn;
@property (strong) NSTableColumn *			instanceServiceTypeColumn;
@property (strong) NSTableColumn *			instancePathPopupColumn;

@property (strong) CNServiceTypeLocalizer * serviceTypeLocalizer;

@property (strong) NSArray *                currentDomainPath;
@property (strong) NSMutableArray *         instanceRs;
@property (strong) NSMutableDictionary *    instanceD;
@property (strong) NSMutableArray *         instanceA;

@property (strong) dispatch_queue_t         instanceBrowseQ;

@end

@implementation CNServiceBrowserView

@synthesize serviceTypes = _serviceTypes;

- (instancetype)initWithFrame:(NSRect)frameRect
{
	if( self = [super initWithFrame: frameRect] )
	{
		[self commonInit];
	}
	return( self );
}

- (nullable instancetype)initWithCoder:(NSCoder *)coder
{
	if( self = [super initWithCoder: coder] )
	{
		[self commonInit];
	}
	return( self );
}


- (void)contentViewsInit
{
	NSRect	frame = self.frame;
	self.instanceC = [[NSArrayController alloc] init];
	self.serviceTypeLocalizer = [[CNServiceTypeLocalizer alloc] initWithDelegate: (id<CNServiceTypeLocalizerDelegate>)self];
	
	//	My table view
	NSTableView * tableView = [[NSTableView alloc] initWithFrame: frame];
	tableView.columnAutoresizingStyle = NSTableViewFirstColumnOnlyAutoresizingStyle;
	tableView.allowsColumnReordering = NO;
	tableView.delegate = (id<NSTableViewDelegate>)self;
    tableView.doubleAction = @selector( doubleAction:);
	[tableView bind: NSContentBinding toObject: self.instanceC withKeyPath: @"arrangedObjects" options: nil];
	self.instanceTable = tableView;

	//	Scroll view for table
	NSScrollView * tableContainer = [[NSScrollView alloc] initWithFrame: frame];
	tableContainer.autoresizingMask = (NSViewHeightSizable | NSViewWidthSizable);
	[tableContainer setDocumentView: tableView];

	//	Name column
	NSTableColumn * column = [[NSTableColumn alloc] init];
	column.resizingMask = (NSTableColumnAutoresizingMask);
	column.width = frame.size.width / 3;
	column.minWidth = column.width / 2;
	NSTextFieldCell * cell = [[NSTextFieldCell alloc] init];
	cell.truncatesLastVisibleLine = YES;
	column.dataCell = cell;
	[column.headerCell setStringValue: NSLocalizedString( @"_dnsBrowser.instances.name", nil )];
	[column bind: NSValueBinding toObject: self.instanceC withKeyPath: @"arrangedObjects.name" options: nil];
	[tableView addTableColumn: column];
	self.instanceNameColumn = column;
	
	//	Service type column
	column = [[NSTableColumn alloc] init];
	column.resizingMask = (NSTableColumnNoResizing);
	column.width = frame.size.width / 3;
	column.dataCell = [[NSTextFieldCell alloc] init];
	[column.headerCell setStringValue: NSLocalizedString( @"_dnsBrowser.instances.type", nil )];
	[column bind: NSValueBinding toObject: self.instanceC withKeyPath: @"arrangedObjects.serviceType" options: @{ NSValueTransformerBindingOption: self.serviceTypeLocalizer }];
	[tableView addTableColumn: column];
	self.instanceServiceTypeColumn = column;
	
	//	Path popup column
	column = [[NSTableColumn alloc] init];
	column.resizingMask = (NSTableColumnNoResizing);
	column.width = frame.size.width / 3;
	NSPopUpButtonCell * popUpCell = [[NSPopUpButtonCell alloc] init];
	popUpCell.pullsDown = YES;
	popUpCell.arrowPosition = NSPopUpArrowAtBottom;
	popUpCell.autoenablesItems = YES;
	popUpCell.preferredEdge = NSRectEdgeMaxY;
    popUpCell.bezelStyle = NSBezelStyleTexturedSquare;
	popUpCell.font = [NSFont systemFontOfSize: [NSFont smallSystemFontSize]];
	column.dataCell = popUpCell;
	[column.headerCell setStringValue: NSLocalizedString( @"_dnsBrowser.instances.domain", nil )];
	[column bind: NSContentBinding toObject: self.instanceC withKeyPath: @"arrangedObjects.domainPath" options: nil];
	[tableView addTableColumn: column];
	self.instancePathPopupColumn = column;
    
	[self addSubview: tableContainer];
}

- (void)commonInit
{
    self.serviceTypes = @[@"_http._tcp"];
    self.instanceRs = [NSMutableArray array];
    self.instanceD = [NSMutableDictionary dictionary];
    self.instanceA = [NSMutableArray array];
    
    [self contentViewsInit];
}

- (void) setServiceTypes:(NSArray *)serviceTypes
{
	if( ![_serviceTypes isEqualTo: serviceTypes] )
	{
		_serviceTypes = serviceTypes;
	}
}

- (NSArray *) serviceTypes
{
	return( _serviceTypes );
}

- (BOOL)foundInstancesWithMoreThanOneServiceType
{
    BOOL result = NO;
    
#if SHOW_SERVICETYPE_IF_SEARCH_COUNT
    result = (_serviceTypes.count > 1);
#else
    if( _instanceD.count )
    {
        NSString * serviceType;
        for( NSDictionary *next in [_instanceD allValues] )
        {
            if( !serviceType )
            {
                serviceType = next[_CNInstanceKey_serviceType];
                continue;
            }
            else if( [next[_CNInstanceKey_serviceType] caseInsensitiveCompare: serviceType] != NSOrderedSame )
            {
                result = YES;
                break;
            }
        }
    }
#endif
    
    return( result );
}

- (BOOL)foundInstancesInMoreThanCurrentDomainPath
{
    BOOL result = NO;
    
    if( _instanceD.count )
    {
        NSArray * selectedPathArray = [[_currentDomainPath reverseObjectEnumerator] allObjects];
        if( !selectedPathArray.count ) selectedPathArray = [NSArray arrayWithObject: @"local"];
        for( NSDictionary *next in [_instanceD allValues] )
        {
            if( [next[_CNInstanceKey_domainPath] caseInsensitiveStringMatch: selectedPathArray] )    continue;
            else
            {
                result = YES;
                break;
            }
        }
    }
    
#if DEBUG_DOMAIN_POPUPS
    return( YES );
#else
    return( result );
#endif
}

#pragma mark - Notifications

- (void)tableViewSelectionDidChange:(NSNotification *)notification
{
	if( _delegate && [_delegate respondsToSelector: @selector(bonjourServiceSelected:type:atDomain:)] &&
	    notification.object == self.instanceTable )
	{
		NSTableView * table = (NSTableView *)notification.object;
		NSDictionary * record = nil;
		if( table.selectedRow >= 0 && table.selectedRow < (NSInteger)[self.instanceC.content count] ) record = (NSDictionary *)self.instanceC.content[table.selectedRow];
		
        [_delegate bonjourServiceSelected: record[_CNInstanceKey_name]
                                     type: record[_CNInstanceKey_serviceType]
                                 atDomain: record ? DomainPathToDNSDomain( [[record[_CNInstanceKey_domainPath] reverseObjectEnumerator] allObjects] ) : nil];
    }
}


#pragma mark - Delegates

- (void)tableView:(NSTableView *)tableView willDisplayCell:(id)cell forTableColumn:(nullable NSTableColumn *)tableColumn row:(NSInteger)row
{
    (void)tableColumn;    // Unused
    (void)row;            // Unused
    if( tableView == self.instanceTable )
	{
		if( [cell isKindOfClass: [NSPopUpButtonCell class]] )
		{
			NSPopUpButtonCell *	popCell = cell;
			if( popCell.numberOfItems > 1  )	popCell.arrowPosition = NSPopUpArrowAtBottom;
			else								popCell.arrowPosition = NSPopUpNoArrow;
		}
	}
}

#if 0
- (void)tableView:(NSTableView *)tableView didClickTableColumn:(NSTableColumn *)tableColumn
{
}
#endif

- (void) handleBrowseResults
{
    dispatch_async( dispatch_get_main_queue(), ^{
        [self bonjourBrowserServiceBrowseUpdate: self->_instanceA];
    });
}

- (void)bonjourBrowserServiceBrowseUpdate:(NSArray *)services
{
    self.instanceC.content = [services sortedArrayUsingComparator: ^( id obj1, id obj2 ) {
        return (NSComparisonResult)[ obj1[_CNInstanceKey_name] compare: obj2[_CNInstanceKey_name]];
    }];
    [self adjustInstancesColumnWidths];
}

- (void) adjustInstancesColumnWidths
{
    self.instanceServiceTypeColumn.hidden = ![self foundInstancesWithMoreThanOneServiceType];
    self.instancePathPopupColumn.hidden = ![self foundInstancesInMoreThanCurrentDomainPath];
    
    if( !self.instanceServiceTypeColumn.hidden || !self.instancePathPopupColumn.hidden )
    {
        BOOL        sizeChanged = NO;
        CGFloat        maxWidthType = 0;
        CGFloat        maxWidthDomain = 0;
        BOOL        needRoomForPopup = NO;
        NSDictionary * fontAttrType = @{ NSFontAttributeName: ((NSTextFieldCell *)self.instanceServiceTypeColumn.dataCell).font };
        NSDictionary * fontAttrDomain = @{ NSFontAttributeName: ((NSTextFieldCell *)self.instancePathPopupColumn.dataCell).font };
        
        for( NSDictionary * next in self.instanceC.content )
        {
            NSString * serviceType = [self.serviceTypeLocalizer transformedValue: next[_CNInstanceKey_serviceType]];
            NSSize        nextSize = [serviceType sizeWithAttributes: fontAttrType];
            maxWidthType = MAX( nextSize.width, maxWidthType );
            
            NSArray *    path = next[_CNInstanceKey_domainPath];
            nextSize = [path[0] sizeWithAttributes: fontAttrDomain];
            maxWidthDomain = MAX( nextSize.width, maxWidthDomain );
            if( path.count > 1 ) needRoomForPopup = YES;
        }
        
#define EDGE_GAP    5
#define POPUP_ARROW    22
        
        if( !self.instanceServiceTypeColumn.hidden )
        {
            maxWidthType += (EDGE_GAP * 2);
            if( self.instanceServiceTypeColumn.width != maxWidthType )
            {
                self.instanceServiceTypeColumn.width = self.instanceServiceTypeColumn.minWidth = self.instanceServiceTypeColumn.maxWidth = maxWidthType;
                sizeChanged = YES;
            }
        }
        
        if( !self.instancePathPopupColumn.hidden )
        {
            maxWidthDomain += (EDGE_GAP * 2) + needRoomForPopup ? POPUP_ARROW : 0;
            if( self.instancePathPopupColumn.width != maxWidthDomain )
            {
                self.instancePathPopupColumn.width = self.instancePathPopupColumn.minWidth = self.instancePathPopupColumn.maxWidth = maxWidthDomain;
                sizeChanged = YES;
            }
        }
        
        if( sizeChanged )
        {
            [self.instancePathPopupColumn.tableView sizeToFit];
        }
    }
}

#pragma mark - Dispatch

static void finalizer( void * context )
{
    CNServiceBrowserView *self = (__bridge CNServiceBrowserView *)context;
//    NSLog( @"finalizer: %@", self );
    (void)CFBridgingRelease( (__bridge void *)self );
}

#pragma mark - Commands

- (void)doubleAction:(id)sender
{
    if( _delegate && [_delegate respondsToSelector: @selector(doubleAction:)] &&
       sender == self.instanceTable )
    {
        NSTableView * table = (NSTableView *)sender;
        NSDictionary * record = nil;
        if( table.selectedRow >= 0 && table.selectedRow < (NSInteger)[self.instanceC.content count] ) record = (NSDictionary *)self.instanceC.content[table.selectedRow];
        [_delegate doubleAction: record[_CNInstanceKey_resolveUrl]];
    }
}

- (void)newServiceBrowse:(NSArray *)domainPath
{
	if( _serviceTypes.count)
	{
        self.instanceC.content = nil;
        [self browseForServiceTypes: _serviceTypes inDomainPath: domainPath];
	}
}

- (void)browseForServiceTypes:(NSArray *)serviceTypes inDomainPath:(NSArray *)domainPath
{
    if( serviceTypes.count /*&& domainPath.count*/ )
    {
        _serviceTypes = [serviceTypes copy];
        _currentDomainPath = [domainPath copy];
        
        NSString * domainStr = DomainPathToDNSDomain( _currentDomainPath );
        
        [_instanceRs removeAllObjects];
        if( !_instanceBrowseQ )
        {
            self.instanceBrowseQ = dispatch_queue_create( "DNSServiceBrowse", DISPATCH_QUEUE_PRIORITY_DEFAULT );
            dispatch_set_context( _instanceBrowseQ, (void *)CFBridgingRetain( self ) );
            dispatch_set_finalizer_f( _instanceBrowseQ, finalizer );
        }
        
        dispatch_sync( _instanceBrowseQ, ^{
            [self->_instanceD removeAllObjects];
            [self->_instanceA removeAllObjects];
        });
        
        DNSServiceErrorType error;
        DNSServiceRef mainRef;
        if( (error = DNSServiceCreateConnection( &mainRef )) != 0 )
            NSLog(@"DNSServiceCreateConnection failed error: %ld", error);
        else
        {
            for( NSString * nextService in _serviceTypes )
            {
                DNSServiceRef ref = mainRef;
                if( (error = DNSServiceBrowse( &ref, kDNSServiceFlagsShareConnection, 0, [nextService UTF8String], [domainStr UTF8String], browseReply, (__bridge void *)self )) != 0 )
                    NSLog(@"DNSServiceBrowse failed error: %ld", error);
                else
                {
                    [_instanceRs addObject: [[_DNSServiceRefWrapper alloc] initWithRef: ref]];
                }
            }
            [_instanceRs addObject: [[_DNSServiceRefWrapper alloc] initWithRef: mainRef]];
            if( !error )
            {
                error = DNSServiceSetDispatchQueue( mainRef, _instanceBrowseQ );
                if( error ) NSLog( @"DNSServiceSetDispatchQueue error: %d", error );
            }
        }
    }
}

- (void)resolveServiceInstance:(NSMutableDictionary *)record
{
    __weak NSDictionary *   weakRecord = record;
    DNSServiceRef           ref;
    DNSServiceErrorType     error;
    NSString *              domainPath = DomainPathToDNSDomain( record[_CNInstanceKey_domainPath] );
    
    if( (error = DNSServiceResolve( &ref, (DNSServiceFlags)0, kDNSServiceInterfaceIndexAny, [record[_CNInstanceKey_name] UTF8String], [record[_CNInstanceKey_serviceType] UTF8String], [domainPath UTF8String], resolveReply, (__bridge void *)weakRecord )) != 0 )
    {
        NSLog(@"DNSServiceResolve failed error: %ld", error);
    }
    else
    {
        record[_CNInstanceKey_resolveInstance] = [[_DNSServiceRefWrapper alloc] initWithRef: ref];
        error = DNSServiceSetDispatchQueue( ref, _instanceBrowseQ );
        if( error ) NSLog( @"resolve DNSServiceSetDispatchQueue error: %d", error );
    }
}

#pragma mark - Static Callbacks

static void resolveReply( DNSServiceRef sdRef,
                  DNSServiceFlags flags,
                  uint32_t interfaceIndex,
                  DNSServiceErrorType errorCode,
                  const char *fullname,
                  const char *hosttarget,
                  uint16_t port,                                   /* In network byte order */
                  uint16_t txtLen,
                  const unsigned char *txtRecord,
                  void *context )
{
    (void)sdRef;            //    Unused
    (void)flags;            //    Unused
    (void)interfaceIndex;   //    Unused
    (void)errorCode;        //    Unused
    (void)fullname;         //    Unused
    __weak NSMutableDictionary * record = (__bridge __weak NSMutableDictionary *)context;
    if( record && hosttarget )
    {
        NSURLComponents * urlComponents = [[NSURLComponents alloc] init];
        urlComponents.scheme = @"http";
        urlComponents.host = [NSString stringWithUTF8String: hosttarget];
        if( TXTRecordContainsKey( txtLen, txtRecord, "path" ) )
        {
            uint8_t         valueLen;
            const u_char *  valuePtr = TXTRecordGetValuePtr( txtLen, txtRecord, "path", &valueLen );
            urlComponents.path = (__bridge_transfer NSString *)CFStringCreateWithBytes( kCFAllocatorDefault, valuePtr, valueLen, kCFStringEncodingUTF8, false );
        }
        if( port ) urlComponents.port = [NSNumber numberWithUnsignedShort: ntohs( port )];
        record[_CNInstanceKey_resolveUrl] = urlComponents.URL;
    }
}

static void browseReply( DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char *serviceName, const char *regtype, const char *replyDomain, void *context )
{
    (void)sdRef;            //    Unused
    (void)interfaceIndex;   //    Unused
    (void)errorCode;        //    Unused
    CNServiceBrowserView *self = (__bridge CNServiceBrowserView *)context;
    char fullNameBuffer[kDNSServiceMaxDomainName];
    if( DNSServiceConstructFullName( fullNameBuffer, serviceName, regtype, replyDomain ) == kDNSServiceErr_NoError )
    {
        NSString *fullName = @(fullNameBuffer);
        NSString *name = [NSString stringWithUTF8String: serviceName];
        NSArray *pathArray = DNSDomainToDomainPath( [NSString stringWithUTF8String: replyDomain] );
        
        if( flags & kDNSServiceFlagsAdd )
        {
            BOOL    okToAdd = YES;
            NSString * newServiceType = [[NSString stringWithUTF8String: regtype] stringByTrimmingCharactersInSet: [NSCharacterSet characterSetWithCharactersInString: @"."]];
            NSString * oldServiceType = [self.instanceD objectForKey: name][_CNInstanceKey_serviceType];
            if( oldServiceType && ![newServiceType isEqualToString: oldServiceType] )
            {
                NSInteger newIndex = [self.serviceTypes indexOfObject: newServiceType];
                NSInteger oldIndex = [self.serviceTypes indexOfObject: oldServiceType];
                if( newIndex != NSNotFound && oldIndex != NSNotFound && oldIndex < newIndex ) okToAdd = NO;
            }
            if( okToAdd )
            {
                NSMutableDictionary * record = [NSMutableDictionary dictionary];
                record[_CNInstanceKey_fullName] = fullName;
                record[_CNInstanceKey_name] = name;
                record[_CNInstanceKey_serviceType] = newServiceType;
                record[_CNInstanceKey_domainPath] = [[pathArray reverseObjectEnumerator] allObjects];
                [self.instanceD setObject: record
                                   forKey: name];
                [self resolveServiceInstance: record];
            }
        }
        else
        {
            NSString * newServiceType = [[NSString stringWithUTF8String: regtype] stringByTrimmingCharactersInSet: [NSCharacterSet characterSetWithCharactersInString: @"."]];
            NSDictionary * oldRecord = [self.instanceD objectForKey: name];
            if( [oldRecord[_CNInstanceKey_serviceType] isEqualToString: newServiceType] )
            {
                [self.instanceD removeObjectForKey: name];
            }
        }
        
        if( !(flags & kDNSServiceFlagsMoreComing) )
        {
            dispatch_async( dispatch_get_main_queue(), ^{
                [self.instanceA setArray: [[self.instanceD allValues] sortedArrayUsingComparator: ^( id obj1, id obj2 ) {
                    return (NSComparisonResult)[obj1[_CNInstanceKey_name] compare: obj2[_CNInstanceKey_name] options: NSCaseInsensitiveSearch];
                }]];
                [self handleBrowseResults];
            });
        }
    }
}

@end
