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

#include "dns_sd.h"

@interface RegistrationController : NSObject
{
    IBOutlet NSTableColumn 	*typeColumn;
    IBOutlet NSTableColumn 	*nameColumn;
    IBOutlet NSTableColumn 	*portColumn;
    IBOutlet NSTableColumn 	*domainColumn;
    IBOutlet NSTableColumn 	*textColumn;

    IBOutlet NSTableView	*serviceDisplayTable;

    IBOutlet NSTextField	*serviceTypeField;
    IBOutlet NSTextField	*serviceNameField;
    IBOutlet NSTextField	*servicePortField;
    IBOutlet NSTextField	*serviceDomainField;
    IBOutlet NSTextField	*serviceTextField;
    
    NSMutableArray		*srvtypeKeys;
    NSMutableArray		*srvnameKeys;
    NSMutableArray		*srvportKeys;
    NSMutableArray		*srvdomainKeys;
    NSMutableArray		*srvtextKeys;

    NSMutableDictionary		*registeredDict;
}

- (IBAction)registerService:(id)sender;
- (IBAction)unregisterService:(id)sender;

- (IBAction)addNewService:(id)sender;
- (IBAction)removeSelected:(id)sender;

@end

void reg_reply
    (
    DNSServiceRef                       sdRef,
    DNSServiceFlags                     flags,
    DNSServiceErrorType                 errorCode,
    const char                          *name,
    const char                          *regtype,
    const char                          *domain,
    void                                *context
    )
{
    // registration reply
    printf("Got a reply from the server with error %d\n", errorCode);
    return;
}

static void myCFSocketCallBack(CFSocketRef cfs, CFSocketCallBackType CallBackType, CFDataRef address, const void *data, void *context)
	{
	DNSServiceProcessResult((DNSServiceRef)context);
	}

static void addDNSServiceRefToRunLoop(DNSServiceRef ref)
	{
	int s = DNSServiceRefSockFD(ref);
	CFSocketContext myCFSocketContext = { 0, ref, NULL, NULL, NULL };
	CFSocketRef c = CFSocketCreateWithNative(kCFAllocatorDefault, s, kCFSocketReadCallBack, myCFSocketCallBack, &myCFSocketContext);
	CFRunLoopSourceRef rls = CFSocketCreateRunLoopSource(kCFAllocatorDefault, c, 0);
	CFRunLoopAddSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode);
	CFRelease(rls);
	}


@implementation RegistrationController

- (void)registerDefaults
{
    NSMutableDictionary *regDict = [NSMutableDictionary dictionary];

    NSArray *typeArray   = [NSArray arrayWithObjects:@"_ftp._tcp.",    @"_ssh._tcp.",  @"_tftp._tcp.",        @"_http._tcp.",      @"_printer._tcp.",  @"_afpovertcp._tcp.",         nil];
    NSArray *nameArray   = [NSArray arrayWithObjects:@"My ftp Server", @"My Computer", @"Testing Boot Image", @"A Web Server",     @"Steve's Printer", @"Company AppleShare Server", nil];
    NSArray *portArray   = [NSArray arrayWithObjects:@"21",            @"22",          @"69",                 @"80",               @"515",             @"548",                       nil];
    NSArray *domainArray = [NSArray arrayWithObjects:@"",              @"",            @"",                   @"",                 @"",                @"",                          nil];
    NSArray *textArray   = [NSArray arrayWithObjects:@"",              @"",            @"image=mybootimage",  @"path=/index.html", @"rn=lpt1",         @"Vol=Public",                nil];

    [regDict setObject:typeArray forKey:@"SrvTypeKeys"];
    [regDict setObject:nameArray forKey:@"SrvNameKeys"];
    [regDict setObject:portArray forKey:@"SrvPortKeys"];
    [regDict setObject:domainArray forKey:@"SrvDomainKeys"];
    [regDict setObject:textArray forKey:@"SrvTextKeys"];

    [[NSUserDefaults standardUserDefaults] registerDefaults:regDict];
}

- (id)init
{
    srvtypeKeys = [[NSMutableArray array] retain];	//Define arrays for Type, Domain, and Name
    srvnameKeys = [[NSMutableArray array] retain];
    srvportKeys = [[NSMutableArray array] retain];
    srvdomainKeys = [[NSMutableArray array] retain];
    srvtextKeys = [[NSMutableArray array] retain];

    registeredDict = [[NSMutableDictionary alloc] init];
    
    [self registerDefaults];
    return [super init];
}

- (void)awakeFromNib				//BrowserController startup procedure
{
    [srvtypeKeys addObjectsFromArray:[[NSUserDefaults standardUserDefaults] arrayForKey:@"SrvTypeKeys"]];
    [srvnameKeys addObjectsFromArray:[[NSUserDefaults standardUserDefaults] arrayForKey:@"SrvNameKeys"]];
    [srvportKeys addObjectsFromArray:[[NSUserDefaults standardUserDefaults] arrayForKey:@"SrvPortKeys"]];
    [srvdomainKeys addObjectsFromArray:[[NSUserDefaults standardUserDefaults] arrayForKey:@"SrvDomainKeys"]];
    [srvtextKeys addObjectsFromArray:[[NSUserDefaults standardUserDefaults] arrayForKey:@"SrvTextKeys"]];

    [serviceDisplayTable reloadData];				//Reload (redraw) data in fields

}



 - (IBAction)registerService:(id)sender
{
    int selectedRow = [serviceDisplayTable selectedRow];
    CFMachPortContext       context;
    DNSServiceRef 	        dns_client;

    if (selectedRow < 0) {
        return;
    }

	NSString *key = [srvtypeKeys objectAtIndex:selectedRow];
	if ([registeredDict objectForKey:key]) { printf("Already registered\n"); return; }

    context.version                 = 1;
    context.info                    = 0;
    context.retain                  = NULL;
    context.release                 = NULL;
    context.copyDescription 	    = NULL;
    unsigned char txtbuffer[300];
	strncpy(txtbuffer+1, [[srvtextKeys objectAtIndex:selectedRow] UTF8String], sizeof(txtbuffer)-1);
	txtbuffer[0] = strlen(txtbuffer+1);

    DNSServiceErrorType err = DNSServiceRegister
        (
        	&dns_client, 0, 0,
            [[srvnameKeys objectAtIndex:selectedRow] UTF8String],
            [key UTF8String],
            [[srvdomainKeys objectAtIndex:selectedRow] UTF8String],
            NULL, htons([[srvportKeys objectAtIndex:selectedRow] intValue]),
            txtbuffer[0]+1, txtbuffer,
            reg_reply,
            nil
            );
	if (err)
		printf("DNSServiceRegister failed %d\n", err);
	else
	{
		addDNSServiceRefToRunLoop(dns_client);
		[registeredDict setObject:[NSNumber numberWithUnsignedInt:(unsigned int)dns_client] forKey:key];
	}
}

- (IBAction)unregisterService:(id)sender
{
    int selectedRow = [serviceDisplayTable selectedRow];
    NSString *key = [srvtypeKeys objectAtIndex:selectedRow];

    NSNumber *refPtr = [registeredDict objectForKey:key];
    DNSServiceRef ref = (DNSServiceRef)[refPtr unsignedIntValue];

    if (ref) {
        DNSServiceRefDeallocate(ref);
        [registeredDict removeObjectForKey:key];
    }
}

-(void)tableView:(NSTableView *)theTableView setObjectValue:(id)object forTableColumn:(NSTableColumn *)tableColumn row:(int)row
{
    if (row<0) return;
}

- (int)numberOfRowsInTableView:(NSTableView *)theTableView	//Begin mandatory TableView methods
{
    return [srvtypeKeys count];
}

- (id)tableView:(NSTableView *)theTableView objectValueForTableColumn:(NSTableColumn *)theColumn row:(int)rowIndex
{
    if (theColumn == typeColumn) {
        return [srvtypeKeys objectAtIndex:rowIndex];
    }
    if (theColumn == nameColumn) {
        return [srvnameKeys objectAtIndex:rowIndex];
    }
    if (theColumn == portColumn) {
        return [srvportKeys objectAtIndex:rowIndex];
    }
    if (theColumn == domainColumn) {
        return [srvdomainKeys objectAtIndex:rowIndex];
    }
    if (theColumn == textColumn) {
        return [srvtextKeys objectAtIndex:rowIndex];
    }
    
    return(0);
}						//End of mandatory TableView methods

- (IBAction)removeSelected:(id)sender
{
    // remove the selected row and force a refresh

    int selectedRow = [serviceDisplayTable selectedRow];

    if (selectedRow) {

        [srvtypeKeys removeObjectAtIndex:selectedRow];
        [srvnameKeys removeObjectAtIndex:selectedRow];
        [srvportKeys removeObjectAtIndex:selectedRow];
        [srvdomainKeys removeObjectAtIndex:selectedRow];
        [srvtextKeys removeObjectAtIndex:selectedRow];

        [[NSUserDefaults standardUserDefaults] setObject:srvtypeKeys forKey:@"SrvTypeKeys"];
        [[NSUserDefaults standardUserDefaults] setObject:srvnameKeys forKey:@"SrvNameKeys"];
        [[NSUserDefaults standardUserDefaults] setObject:srvportKeys forKey:@"SrvPortKeys"];
        [[NSUserDefaults standardUserDefaults] setObject:srvdomainKeys forKey:@"SrvDomainKeys"];
        [[NSUserDefaults standardUserDefaults] setObject:srvtextKeys forKey:@"SrvTextKeys"];
        
        [serviceDisplayTable reloadData];
    }
}

- (IBAction)addNewService:(id)sender
{
    // add new entries from the edit fields to the arrays for the defaults

    if ([[serviceTypeField stringValue] length] && [[serviceNameField stringValue] length] && [[serviceDomainField stringValue] length]&& [[servicePortField stringValue] length]) {
        [srvtypeKeys addObject:[serviceTypeField stringValue]];
        [srvnameKeys addObject:[serviceNameField stringValue]];
        [srvportKeys addObject:[servicePortField stringValue]];
        [srvdomainKeys addObject:[serviceDomainField stringValue]];
        [srvtextKeys addObject:[serviceTextField stringValue]];

        [[NSUserDefaults standardUserDefaults] setObject:srvtypeKeys forKey:@"SrvTypeKeys"];
        [[NSUserDefaults standardUserDefaults] setObject:srvnameKeys forKey:@"SrvNameKeys"];
        [[NSUserDefaults standardUserDefaults] setObject:srvportKeys forKey:@"SrvPortKeys"];
        [[NSUserDefaults standardUserDefaults] setObject:srvdomainKeys forKey:@"SrvDomainKeys"];
        [[NSUserDefaults standardUserDefaults] setObject:srvtextKeys forKey:@"SrvTextKeys"];

        [serviceDisplayTable reloadData];
    } else {
        NSBeep();
    }

}

@end

int main(int argc, const char *argv[])
{
    return NSApplicationMain(argc, argv);
}
