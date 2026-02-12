/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2002-2003 Apple Computer, Inc. All rights reserved.
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

#import <Cocoa/Cocoa.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <unistd.h>
#include <dns_sd.h>

@class ServiceController;  // holds state corresponding to outstanding DNSServiceRef

@interface BrowserController : NSObject
{
    IBOutlet id nameField;
    IBOutlet id typeField;

    IBOutlet id serviceDisplayTable;
    IBOutlet id typeColumn;
    IBOutlet id nameColumn;
    IBOutlet id serviceTypeField;
    IBOutlet id serviceNameField;

    IBOutlet id hostField;
    IBOutlet id ipAddressField;
    IBOutlet id ip6AddressField;
    IBOutlet id portField;
    IBOutlet id interfaceField;
    IBOutlet id textField;
    
    NSMutableArray *_srvtypeKeys;
    NSMutableArray *_srvnameKeys;
    NSMutableArray *_sortedServices;
    NSMutableDictionary *_servicesDict;

	ServiceController *_serviceBrowser;
	ServiceController *_serviceResolver;
	ServiceController *_ipv4AddressResolver;
	ServiceController *_ipv6AddressResolver;
}

- (void)notifyTypeSelectionChange:(NSNotification*)note;
- (void)notifyNameSelectionChange:(NSNotification*)note;

- (IBAction)connect:(id)sender;

- (IBAction)handleTableClick:(id)sender;
- (IBAction)removeSelected:(id)sender;
- (IBAction)addNewService:(id)sender;

- (IBAction)update:(NSString *)Type;

- (void)updateBrowseWithName:(const char *)name type:(const char *)resulttype domain:(const char *)domain interface:(uint32_t)interface flags:(DNSServiceFlags)flags;
- (void)resolveClientWitHost:(NSString *)host port:(uint16_t)port interfaceIndex:(uint32_t)interface txtRecord:(const char*)txtRecord txtLen:(uint16_t)txtLen;
- (void)updateAddress:(uint16_t)rrtype addr:(const void *)buff addrLen:(uint16_t)addrLen host:(const char*)host interfaceIndex:(uint32_t)interface more:(boolean_t)moreToCome;

- (void)_cancelPendingResolve;
- (void)_clearResolvedInfo;

@end

// The ServiceController manages cleanup of DNSServiceRef & runloop info for an outstanding request
@interface ServiceController : NSObject
{
	DNSServiceRef       fServiceRef;
	CFSocketRef         fSocketRef;
	CFRunLoopSourceRef  fRunloopSrc;
}

- (id)initWithServiceRef:(DNSServiceRef)ref;
- (void)addToCurrentRunLoop;
- (DNSServiceRef)serviceRef;
- (void)dealloc;

@end // interface ServiceController


static void
ProcessSockData(CFSocketRef s, CFSocketCallBackType type, CFDataRef address, const void *data, void *info)
{
	DNSServiceRef serviceRef = (DNSServiceRef)info;
	DNSServiceErrorType err = DNSServiceProcessResult(serviceRef);
	if (err != kDNSServiceErr_NoError) {
		printf("DNSServiceProcessResult() returned an error! %d\n", err);
    }
}


static void
ServiceBrowseReply(DNSServiceRef sdRef, DNSServiceFlags servFlags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, 
    const char *serviceName, const char *regtype, const char *replyDomain, void *context)
{
	if (errorCode == kDNSServiceErr_NoError) {
		[(BrowserController*)context updateBrowseWithName:serviceName type:regtype domain:replyDomain interface:interfaceIndex flags:servFlags];
	} else {
		printf("ServiceBrowseReply got an error! %d\n", errorCode);
	}
}


static void
ServiceResolveReply(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode,
    const char *fullname, const char *hosttarget, uint16_t port, uint16_t txtLen, const char *txtRecord, void *context)
{
	if (errorCode == kDNSServiceErr_NoError) {
		[(BrowserController*)context resolveClientWitHost:[NSString stringWithUTF8String:hosttarget] port:port interfaceIndex:interfaceIndex txtRecord:txtRecord txtLen:txtLen];
	} else {
		printf("ServiceResolveReply got an error! %d\n", errorCode);
	}
}


static void
QueryRecordReply(DNSServiceRef DNSServiceRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode,
    const char *fullname, uint16_t rrtype, uint16_t rrclass,  uint16_t rdlen, const void *rdata, uint32_t ttl, void *context)
{
    if (errorCode == kDNSServiceErr_NoError) {
        [(BrowserController*)context updateAddress:rrtype addr:rdata addrLen:rdlen host:fullname interfaceIndex:interfaceIndex more:(flags & kDNSServiceFlagsMoreComing)];
    } else {
        printf("QueryRecordReply got an error! %d\n", errorCode);
    }
}


static void
InterfaceIndexToName(uint32_t interface, char *interfaceName)
{
    assert(interfaceName);
    
    if (interface == kDNSServiceInterfaceIndexAny) {
        // All active network interfaces.
        strlcpy(interfaceName, "all", IF_NAMESIZE);
    } else if (interface == kDNSServiceInterfaceIndexLocalOnly) {
        // Only available locally on this machine.
        strlcpy(interfaceName, "local", IF_NAMESIZE);
    } else if (interface == kDNSServiceInterfaceIndexP2P) {
        // Peer-to-peer.
        strlcpy(interfaceName, "p2p", IF_NAMESIZE);
    } else {
        // Converts interface index to interface name.
        if_indextoname(interface, interfaceName);
    }
}


@implementation BrowserController		//Begin implementation of BrowserController methods

- (void)registerDefaults
{
    NSMutableDictionary *regDict = [NSMutableDictionary dictionary];

    NSArray *typeArray = [NSArray arrayWithObjects:@"_afpovertcp._tcp",
                                                   @"_smb._tcp",
                                                   @"_rfb._tcp",
												   @"_ssh._tcp",
                                                   @"_ftp._tcp",
												   @"_http._tcp",
												   @"_printer._tcp",
                                                   @"_ipp._tcp",
                                                   @"_airport._tcp",
												   @"_presence._tcp",
												   @"_daap._tcp",
												   @"_dpap._tcp",
                                                   nil];
                                                   
    NSArray *nameArray = [NSArray arrayWithObjects:@"AppleShare Servers",
                                                   @"Windows Sharing",
                                                   @"Screen Sharing",
	                                               @"Secure Shell",
                                                   @"FTP Servers",
	                                               @"Web Servers",
	                                               @"LPR Printers",
                                                   @"IPP Printers",
                                                   @"AirPort Base Stations",
												   @"iChat Buddies",
												   @"iTunes Libraries",
												   @"iPhoto Libraries",
                                                   nil];

    [regDict setObject:typeArray forKey:@"SrvTypeKeys"];
    [regDict setObject:nameArray forKey:@"SrvNameKeys"];

    [[NSUserDefaults standardUserDefaults] registerDefaults:regDict];
}


- (id)init
{
    self = [super init];
    if (self) {
        _srvtypeKeys = nil;
        _srvnameKeys = nil;
        _serviceBrowser = nil;
        _serviceResolver = nil;
        _ipv4AddressResolver = nil;
        _ipv6AddressResolver = nil;
        _sortedServices = [[NSMutableArray alloc] init];
        _servicesDict = [[NSMutableDictionary alloc] init];
    }
    return self;
}


- (void)awakeFromNib
{
    [typeField sizeLastColumnToFit];
    [nameField sizeLastColumnToFit];
    [nameField setDoubleAction:@selector(connect:)];

    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(notifyTypeSelectionChange:) name:NSTableViewSelectionDidChangeNotification object:typeField];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(notifyNameSelectionChange:) name:NSTableViewSelectionDidChangeNotification object:nameField];
    
    _srvtypeKeys = [[[NSUserDefaults standardUserDefaults] arrayForKey:@"SrvTypeKeys"] mutableCopy];
    _srvnameKeys = [[[NSUserDefaults standardUserDefaults] arrayForKey:@"SrvNameKeys"] mutableCopy];

    if (!_srvtypeKeys || !_srvnameKeys) {
        [_srvtypeKeys release];
        [_srvnameKeys release];
        [self registerDefaults];
        _srvtypeKeys = [[[NSUserDefaults standardUserDefaults] arrayForKey:@"SrvTypeKeys"] mutableCopy];
        _srvnameKeys = [[[NSUserDefaults standardUserDefaults] arrayForKey:@"SrvNameKeys"] mutableCopy];
    }
    
    [typeField reloadData];
}


- (void)dealloc
{
    [_srvtypeKeys release];
    [_srvnameKeys release];
    [_servicesDict release];
    [_sortedServices release];
    [super dealloc];
}


-(void)tableView:(NSTableView *)theTableView setObjectValue:(id)object forTableColumn:(NSTableColumn *)tableColumn row:(int)row
{
    if (row < 0) return;
}


- (int)numberOfRowsInTableView:(NSTableView *)theTableView	//Begin mandatory TableView methods
{
    if (theTableView == typeField) {
        return [_srvnameKeys count];
    }
    if (theTableView == nameField) {
        return [_servicesDict count];
    }
    if (theTableView == serviceDisplayTable) {
        return [_srvnameKeys count];
    }
    return 0;
}


- (id)tableView:(NSTableView *)theTableView objectValueForTableColumn:(NSTableColumn *)theColumn row:(int)rowIndex
{
    if (theTableView == typeField) {
        return [_srvnameKeys objectAtIndex:rowIndex];
    }
    if (theTableView == nameField) {
        return [[_servicesDict objectForKey:[_sortedServices objectAtIndex:rowIndex]] name];
    }
    if (theTableView == serviceDisplayTable) {
        if (theColumn == typeColumn) {
            return [_srvtypeKeys objectAtIndex:rowIndex];
        }
        if (theColumn == nameColumn) {
            return [_srvnameKeys objectAtIndex:rowIndex];
        }
        return nil;
    }
    
    return nil;
}


- (void)notifyTypeSelectionChange:(NSNotification*)note
{
    [self _cancelPendingResolve];

    int index = [[note object] selectedRow];
    if (index != -1) {
        [self update:[_srvtypeKeys objectAtIndex:index]];
    } else {
        [self update:nil];
    }
}


- (void)notifyNameSelectionChange:(NSNotification*)note
{
    [self _cancelPendingResolve];
    
    int index = [[note object] selectedRow];
    if (index == -1) {
		return;
	}
    
    // Get the currently selected service
    NSNetService *service = [_servicesDict objectForKey:[_sortedServices objectAtIndex:index]];
	
    DNSServiceRef serviceRef;
	DNSServiceErrorType err = DNSServiceResolve(&serviceRef,
                                         (DNSServiceFlags)0,
                               kDNSServiceInterfaceIndexAny,
                  (const char *)[[service name] UTF8String],
                 (const char *)[[service type]  UTF8String],
                (const char *)[[service domain] UTF8String],
                (DNSServiceResolveReply)ServiceResolveReply,
                                                      self);
        
	if (kDNSServiceErr_NoError == err) {
		_serviceResolver = [[ServiceController alloc] initWithServiceRef:serviceRef];
		[_serviceResolver addToCurrentRunLoop];
	}
}


- (IBAction)update:(NSString *)theType
{
    [_servicesDict removeAllObjects];
    [_sortedServices removeAllObjects];
    [nameField reloadData];

    // get rid of the previous browser if one exists
    if (_serviceBrowser != nil) {
		[_serviceBrowser release];
        _serviceBrowser = nil;
    }
    
    if (theType) {
        DNSServiceRef serviceRef;
        DNSServiceErrorType err = DNSServiceBrowse(&serviceRef, (DNSServiceFlags)0, 0, [theType UTF8String], NULL, ServiceBrowseReply, self);
        if (kDNSServiceErr_NoError == err) {
            _serviceBrowser = [[ServiceController alloc] initWithServiceRef:serviceRef];
            [_serviceBrowser addToCurrentRunLoop];
        }
    }
}


- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication
{
    return YES;
}


- (void)updateBrowseWithName:(const char *)name type:(const char *)type domain:(const char *)domain interface:(uint32_t)interface flags:(DNSServiceFlags)flags
{
    NSString *key = [NSString stringWithFormat:@"%s.%s%s%d", name, type, domain, interface];
    NSNetService *service = [[NSNetService alloc] initWithDomain:[NSString stringWithUTF8String:domain] type:[NSString stringWithUTF8String:type] name:[NSString stringWithUTF8String:name]];
    
    if (flags & kDNSServiceFlagsAdd) {
        [_servicesDict setObject:service forKey:key];
    } else {
        [_servicesDict removeObjectForKey:key];
    }

    // If not expecting any more data, then reload (redraw) TableView with newly found data
    if (!(flags & kDNSServiceFlagsMoreComing)) {
    
        // Save the current TableView selection
        int index = [nameField selectedRow];
        NSString *selected = (index != -1) ? [[_sortedServices objectAtIndex:index] copy] : nil;
        
        [_sortedServices release];
        _sortedServices = [[_servicesDict allKeys] mutableCopy];        
        [_sortedServices sortUsingSelector:@selector(caseInsensitiveCompare:)];
        [nameField reloadData];
        
        // Restore the previous TableView selection
        index = selected ? [_sortedServices indexOfObject:selected] : NSNotFound;
        if (index != NSNotFound) {
            [nameField selectRowIndexes:[NSIndexSet indexSetWithIndex:index] byExtendingSelection:NO];
            [nameField scrollRowToVisible:index];
        }
        
        [selected release];
    }

    [service release];

    return;
}


- (void)resolveClientWitHost:(NSString *)host port:(uint16_t)port interfaceIndex:(uint32_t)interface txtRecord:(const char*)txtRecord txtLen:(uint16_t)txtLen
{
	DNSServiceRef serviceRef;

	if (_ipv4AddressResolver) {
		[_ipv4AddressResolver release];
		_ipv4AddressResolver = nil;
	}
    
    if (_ipv6AddressResolver) {
		[_ipv6AddressResolver release];
		_ipv6AddressResolver = nil;
	}

	// Start an async lookup for IPv4 addresses
	DNSServiceErrorType err = DNSServiceQueryRecord(&serviceRef, (DNSServiceFlags)0, interface, [host UTF8String], kDNSServiceType_A, kDNSServiceClass_IN, QueryRecordReply, self);
	if (err == kDNSServiceErr_NoError) {
		_ipv4AddressResolver = [[ServiceController alloc] initWithServiceRef:serviceRef];
		[_ipv4AddressResolver addToCurrentRunLoop];
	}

	// Start an async lookup for IPv6 addresses
    err = DNSServiceQueryRecord(&serviceRef, (DNSServiceFlags)0, interface, [host UTF8String], kDNSServiceType_AAAA, kDNSServiceClass_IN, QueryRecordReply, self);
    if (err == kDNSServiceErr_NoError) {
        _ipv6AddressResolver = [[ServiceController alloc] initWithServiceRef:serviceRef];
        [_ipv6AddressResolver addToCurrentRunLoop];
    }

    char interfaceName[IF_NAMESIZE];
    InterfaceIndexToName(interface, interfaceName);

    [hostField setStringValue:host];
    [interfaceField setStringValue:[NSString stringWithUTF8String:interfaceName]];
    [portField setIntValue:ntohs(port)];

	// kind of a hack: munge txtRecord so it's human-readable
	if (txtLen > 0) {
		char *readableText = (char*) malloc(txtLen);
		if (readableText != nil) {
			ByteCount index, subStrLen;
			memcpy(readableText, txtRecord, txtLen);
			for (index=0; index < txtLen - 1; index += subStrLen + 1) {
				subStrLen = readableText[index];
				readableText[index] = ' ';
			}
			[textField setStringValue:[NSString stringWithCString:&readableText[1] length:txtLen - 1]];
			free(readableText);
		}
	}
}


- (void)updateAddress:(uint16_t)rrtype  addr:(const void *)buff addrLen:(uint16_t)addrLen host:(const char*) host interfaceIndex:(uint32_t)interface more:(boolean_t)moreToCome
{
    char addrBuff[256];

	if (rrtype == kDNSServiceType_A) {
		inet_ntop(AF_INET, buff, addrBuff, sizeof(addrBuff));
        if ([[ipAddressField stringValue] length] > 0) {
            [ipAddressField setStringValue:[NSString stringWithFormat:@"%@, ", [ipAddressField stringValue]]];
        }
		[ipAddressField setStringValue:[NSString stringWithFormat:@"%@%s", [ipAddressField stringValue], addrBuff]];

		if (!moreToCome) {
			[_ipv4AddressResolver release];
			_ipv4AddressResolver = nil;
		}
	} else if (rrtype == kDNSServiceType_AAAA) {
		inet_ntop(AF_INET6, buff, addrBuff, sizeof(addrBuff));
        if ([[ip6AddressField stringValue] length] > 0) {
            [ip6AddressField setStringValue:[NSString stringWithFormat:@"%@, ", [ip6AddressField stringValue]]];
        }
		[ip6AddressField setStringValue:[NSString stringWithFormat:@"%@%s", [ip6AddressField stringValue], addrBuff]];

		if (!moreToCome) {
			[_ipv6AddressResolver release];
			_ipv6AddressResolver = nil;
		}
	}
}


- (void)connect:(id)sender
{
    NSString *host = [hostField stringValue];
    NSString *txtRecord = [textField stringValue];
    int port = [portField intValue];
        
    int index = [nameField selectedRow];
    NSString *selected = (index >= 0) ? [_sortedServices objectAtIndex:index] : nil;
    NSString *type = [[_servicesDict objectForKey:selected] type];
    
    if ([type isEqual:@"_http._tcp."]) {
        NSString *pathDelim = @"path=";
		NSRange where;

        // If the TXT record specifies a path, extract it.
		where = [txtRecord rangeOfString:pathDelim options:NSCaseInsensitiveSearch];
        if (where.length) {
			NSRange	targetRange = { where.location + where.length, [txtRecord length] - where.location - where.length };
			NSRange	endDelim = [txtRecord rangeOfString:@"\n" options:kNilOptions range:targetRange];
			
			if (endDelim.length)   // if a delimiter was found, truncate the target range
				targetRange.length = endDelim.location - targetRange.location;

            NSString    *path = [txtRecord substringWithRange:targetRange];
            [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:[NSString stringWithFormat:@"http://%@:%d%@", host, port, path]]];
        } else {
            [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:[NSString stringWithFormat:@"http://%@:%d", host, port]]];
        }
    }
    else if ([type isEqual:@"_ftp._tcp."])        [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:[NSString stringWithFormat:@"ftp://%@:%d/", host, port]]];
    else if ([type isEqual:@"_ssh._tcp."])        [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:[NSString stringWithFormat:@"ssh://%@:%d/", host, port]]];
    else if ([type isEqual:@"_afpovertcp._tcp."]) [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:[NSString stringWithFormat:@"afp://%@:%d/", host, port]]];
    else if ([type isEqual:@"_smb._tcp."])        [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:[NSString stringWithFormat:@"smb://%@:%d/", host, port]]];
    else if ([type isEqual:@"_rfb._tcp."])        [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:[NSString stringWithFormat:@"vnc://%@:%d/", host, port]]];

    return;
}


- (IBAction)handleTableClick:(id)sender
{
    //populate the text fields
}


- (IBAction)removeSelected:(id)sender
{
    // remove the selected row and force a refresh

    int selectedRow = [serviceDisplayTable selectedRow];

    if (selectedRow) {

        [_srvtypeKeys removeObjectAtIndex:selectedRow];
        [_srvnameKeys removeObjectAtIndex:selectedRow];

        [[NSUserDefaults standardUserDefaults] setObject:_srvtypeKeys forKey:@"SrvTypeKeys"];
        [[NSUserDefaults standardUserDefaults] setObject:_srvnameKeys forKey:@"SrvNameKeys"];

        [typeField reloadData];
        [serviceDisplayTable reloadData];
    }
}


- (IBAction)addNewService:(id)sender
{
    // add new entries from the edit fields to the arrays for the defaults
    NSString *newType = [serviceTypeField stringValue];
    NSString *newName = [serviceNameField stringValue];

    // 3282283: trim trailing '.' from service type field
    if ([newType length] && [newType hasSuffix:@"."])
        newType = [newType substringToIndex:[newType length] - 1];

    if ([newType length] && [newName length]) {
        [_srvtypeKeys addObject:newType];
        [_srvnameKeys addObject:newName];

        [[NSUserDefaults standardUserDefaults] setObject:_srvtypeKeys forKey:@"SrvTypeKeys"];
        [[NSUserDefaults standardUserDefaults] setObject:_srvnameKeys forKey:@"SrvNameKeys"];

        [typeField reloadData];
        [serviceDisplayTable reloadData];
    }
}


- (void)_cancelPendingResolve
{
    [_ipv4AddressResolver release];
    _ipv4AddressResolver = nil;

    [_ipv6AddressResolver release];
    _ipv6AddressResolver = nil;

    [_serviceResolver release];
    _serviceResolver = nil;

	[self _clearResolvedInfo];
}


- (void)_clearResolvedInfo
{
	[hostField setStringValue:@""];
	[ipAddressField setStringValue:@""];
	[ip6AddressField setStringValue:@""];
	[portField setStringValue:@""];
    [interfaceField setStringValue:@""];
	[textField setStringValue:@""];
}

@end // implementation BrowserController


@implementation ServiceController : NSObject
{
	DNSServiceRef        fServiceRef;
	CFSocketRef          fSocketRef;
	CFRunLoopSourceRef   fRunloopSrc;
}


- (id)initWithServiceRef:(DNSServiceRef)ref
{
	self = [super init];
    if (self) {
        fServiceRef = ref;
        fSocketRef = NULL;
        fRunloopSrc = NULL;
    }
	return self;
}


- (void)addToCurrentRunLoop
{
	CFSocketContext	context = { 0, (void*)fServiceRef, NULL, NULL, NULL };

	fSocketRef = CFSocketCreateWithNative(kCFAllocatorDefault, DNSServiceRefSockFD(fServiceRef), kCFSocketReadCallBack, ProcessSockData, &context);
	if (fSocketRef) {
        // Prevent CFSocketInvalidate from closing DNSServiceRef's socket.
        CFOptionFlags sockFlags = CFSocketGetSocketFlags(fSocketRef);
        CFSocketSetSocketFlags(fSocketRef, sockFlags & (~kCFSocketCloseOnInvalidate));
		fRunloopSrc = CFSocketCreateRunLoopSource(kCFAllocatorDefault, fSocketRef, 0);
    }
	if (fRunloopSrc) {
		CFRunLoopAddSource(CFRunLoopGetCurrent(), fRunloopSrc, kCFRunLoopDefaultMode);
    } else {
		printf("Could not listen to runloop socket\n");
    }
}


- (DNSServiceRef)serviceRef
{
	return fServiceRef;
}


- (void)dealloc
{
	if (fSocketRef) {
		CFSocketInvalidate(fSocketRef);		// Note: Also closes the underlying socket
		CFRelease(fSocketRef);
        
        // Workaround that gives time to CFSocket's select thread so it can remove the socket from its
        // FD set before we close the socket by calling DNSServiceRefDeallocate. <rdar://problem/3585273>
        usleep(1000);
	}

	if (fRunloopSrc) {
		CFRunLoopRemoveSource(CFRunLoopGetCurrent(), fRunloopSrc, kCFRunLoopDefaultMode);
		CFRelease(fRunloopSrc);
	}

	DNSServiceRefDeallocate(fServiceRef);

	[super dealloc];
}


@end // implementation ServiceController

int main(int argc, const char *argv[])
{
    return NSApplicationMain(argc, argv);
}
