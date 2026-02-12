/*
    File: DNSServiceDiscoveryPref.m

    Abstract: System Preference Pane for Dynamic DNS and Wide-Area DNS Service Discovery

    Copyright: (c) Copyright 2005-2011 Apple Inc. All rights reserved.

    Disclaimer: IMPORTANT: This Apple software is supplied to you by Apple Inc.
    ("Apple") in consideration of your agreement to the following terms, and your
    use, installation, modification or redistribution of this Apple software
    constitutes acceptance of these terms.  If you do not agree with these terms,
    please do not use, install, modify or redistribute this Apple software.

    In consideration of your agreement to abide by the following terms, and subject
    to these terms, Apple grants you a personal, non-exclusive license, under Apple's
    copyrights in this original Apple software (the "Apple Software"), to use,
    reproduce, modify and redistribute the Apple Software, with or without
    modifications, in source and/or binary forms; provided that if you redistribute
    the Apple Software in its entirety and without modifications, you must retain
    this notice and the following text and disclaimers in all such redistributions of
    the Apple Software.  Neither the name, trademarks, service marks or logos of
    Apple Inc. may be used to endorse or promote products derived from the
    Apple Software without specific prior written permission from Apple.  Except as
    expressly stated in this notice, no other rights or licenses, express or implied,
    are granted by Apple herein, including but not limited to any patent rights that
    may be infringed by your derivative works or by other works in which the Apple
    Software may be incorporated.

    The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
    WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
    PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
    COMBINATION WITH YOUR PRODUCTS.

    IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
    GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
    OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
    (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
    ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "DNSServiceDiscoveryPref.h"
#import "CNDomainBrowserView.h"
#import "BonjourSCStore.h"
#import "BonjourPrefTool.h"
#import <Foundation/NSXPCConnection_Private.h>

#include "../../Clients/ClientCommon.h"

#pragma mark - BonjourPrefTool

static OSStatus
DNSPrefTool_SetKeychainEntry(NSDictionary * secretDictionary)
{
    __block OSStatus result;
    BonjourPrefTool * prefTool;
    
    NSXPCConnection * _connectionToTool = [[NSXPCConnection alloc] initWithServiceName:@"com.apple.preference.bonjour.tool"];
    _connectionToTool.remoteObjectInterface = [NSXPCInterface interfaceWithProtocol:@protocol(BonjourPrefToolProtocol)];
    [_connectionToTool resume];
    
#if 0
    prefTool = [_connectionToTool remoteObjectProxyWithErrorHandler:^(NSError * _Nonnull error) {
        NSLog( @"Cannot connect to BonjourPrefTool: %@.", error);
        result = error.code;
    }];
#else
    prefTool = [_connectionToTool synchronousRemoteObjectProxyWithErrorHandler:^(NSError * _Nonnull error) {
        NSLog( @"Cannot connect to BonjourPrefTool: %@.", error);
        result = error.code;
    }];
#endif
    [prefTool setKeychainEntry: secretDictionary withStatus: ^(OSStatus status){
        result = status;
    }];

    [_connectionToTool invalidate];

    return (result);
}

#pragma mark -

@implementation DNSServiceDiscoveryPref

static NSInteger
MyArrayCompareFunction(id val1, id val2, void *context)
{
	(void)context; // Unused
    return CFStringCompare((CFStringRef)val1, (CFStringRef)val2, kCFCompareCaseInsensitive);
}

static NSInteger
MyDomainArrayCompareFunction(id val1, id val2, void *context)
{
	(void)context; // Unused
	NSString *domain1 = [val1 objectForKey:(NSString *)SC_DYNDNS_DOMAIN_KEY];
	NSString *domain2 = [val2 objectForKey:(NSString *)SC_DYNDNS_DOMAIN_KEY];
    return CFStringCompare((CFStringRef)domain1, (CFStringRef)domain2, kCFCompareCaseInsensitive);
}


static void NetworkChanged(SCDynamicStoreRef store, CFArrayRef changedKeys, void *context)
{
	(void)store; // Unused
	(void)changedKeys; // Unused
    DNSServiceDiscoveryPref * me = (__bridge DNSServiceDiscoveryPref *)context;
    assert(me != NULL);
    
    [me setupInitialValues];
}


-(void)updateStatusImageView
{
    int value = [self statusForHostName:currentHostName];
    if      (value == 0) [statusImageView setImage:successImage];
    else if (value >  0) [statusImageView setImage:inprogressImage];
    else                 [statusImageView setImage:failureImage];
}


- (void)watchForPreferenceChanges
{
	SCDynamicStoreContext context = { 0, (__bridge void * _Nullable)(self), NULL, NULL, NULL };
	SCDynamicStoreRef     store   = SCDynamicStoreCreate(NULL, CFSTR("watchForPreferenceChanges"), NetworkChanged, &context);
	CFMutableArrayRef     keys    = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
    CFRunLoopSourceRef    rls;
	
	assert(store != NULL);
	assert(keys != NULL);
    
	CFArrayAppendValue(keys, SC_DYNDNS_STATE_KEY);
	CFArrayAppendValue(keys, SC_DYNDNS_SETUP_KEY);

	(void)SCDynamicStoreSetNotificationKeys(store, keys, NULL);

	rls = SCDynamicStoreCreateRunLoopSource(NULL, store, 0);
    assert(rls != NULL);
    
	CFRunLoopAddSource(CFRunLoopGetCurrent(), rls, kCFRunLoopCommonModes);
	CFRelease(rls);

    CFRelease(keys);
	CFRelease(store);
}


-(int)statusForHostName:(NSString * )domain
{
	SCDynamicStoreRef store       = SCDynamicStoreCreate(NULL, CFSTR("statusForHostName"), NULL, NULL);
    NSString     *lowercaseDomain = [domain lowercaseString];
    int status = 1;
    
    assert(store != NULL);
        
    NSDictionary *dynamicDNS = (NSDictionary *)CFBridgingRelease(SCDynamicStoreCopyValue(store, SC_DYNDNS_STATE_KEY));
    if (dynamicDNS) {
        NSDictionary *hostNames = [dynamicDNS objectForKey:(NSString *)SC_DYNDNS_HOSTNAMES_KEY];
        NSDictionary *infoDict  = [hostNames objectForKey:lowercaseDomain];
        if (infoDict) status = [[infoDict objectForKey:(NSString*)SC_DYNDNS_STATUS_KEY] intValue];
	}
    CFRelease(store);

    return status;
}


-(void)readPreferences
{
	NSDictionary *origDict;
    NSArray      *regDomainArray;
    NSArray      *hostArray;

    if (currentRegDomain)          currentRegDomain = nil;
    if (currentBrowseDomainsArray) currentBrowseDomainsArray = nil;
    if (currentHostName)           currentHostName = nil;

	SCDynamicStoreRef store = SCDynamicStoreCreate(NULL, CFSTR("com.apple.preference.bonjour"), NULL, NULL);
	origDict = (NSDictionary *)CFBridgingRelease(SCDynamicStoreCopyValue(store, SC_DYNDNS_SETUP_KEY));

	regDomainArray = [origDict objectForKey:(NSString *)SC_DYNDNS_REGDOMAINS_KEY];
	if (regDomainArray && [regDomainArray count] > 0) {
		currentRegDomain = [[[regDomainArray objectAtIndex:0] objectForKey:(NSString *)SC_DYNDNS_DOMAIN_KEY] copy];
		currentWideAreaState = [[[regDomainArray objectAtIndex:0] objectForKey:(NSString *)SC_DYNDNS_ENABLED_KEY] intValue];
    } else {
		currentRegDomain = @"";
		currentWideAreaState = NO;
	}

	currentBrowseDomainsArray = [origDict objectForKey:(NSString *)SC_DYNDNS_BROWSEDOMAINS_KEY];

    hostArray = [origDict objectForKey:(NSString *)SC_DYNDNS_HOSTNAMES_KEY];
	if (hostArray && [hostArray count] > 0) {
		currentHostName = [[[hostArray objectAtIndex:0] objectForKey:(NSString *)SC_DYNDNS_DOMAIN_KEY] copy];
	} else {
		currentHostName = @"";
    }

    if (store) CFRelease(store);
}


- (void)tableViewSelectionDidChange:(NSNotification *)notification
{
	[removeBrowseDomainButton setEnabled:[[notification object] numberOfSelectedRows]];
}


- (IBAction)addBrowseDomainClicked:(id)sender
{
    NSWindow *  window = (([NSEvent modifierFlags] & NSAlternateKeyMask) == NSAlternateKeyMask) ? addBrowseDomainManualWindow : addBrowseDomainWindow;
    [browseDomainTextField setStringValue: [NSString string]];

    [self disableControls];
	[NSApp beginSheet:window modalForWindow:mainWindow modalDelegate:self
		didEndSelector:@selector(addBrowseDomainSheetDidEnd:returnCode:contextInfo:) contextInfo:(__bridge void * _Null_unspecified)(sender)];

	[browseDomainList deselectAll:sender];
	[self updateApplyButtonState];
}


- (IBAction)removeBrowseDomainClicked:(id)sender
{
	(void)sender; // Unused
	int selectedBrowseDomain = [browseDomainList selectedRow];
	[browseDomainsArray removeObjectAtIndex:selectedBrowseDomain];
	[browseDomainList reloadData];
	[self updateApplyButtonState];
}


- (IBAction)enableBrowseDomainClicked:(id)sender
{
	NSTableView *tableView = sender;
    NSMutableDictionary *browseDomainDict;
	NSInteger value;
	
	browseDomainDict = [[browseDomainsArray objectAtIndex:[tableView clickedRow]] mutableCopy];
	value = [[browseDomainDict objectForKey:(NSString *)SC_DYNDNS_ENABLED_KEY] intValue];
	[browseDomainDict setObject:[NSNumber numberWithInt:(!value)] forKey:(NSString *)SC_DYNDNS_ENABLED_KEY];
	[browseDomainsArray replaceObjectAtIndex:[tableView clickedRow] withObject:browseDomainDict];
	[tableView reloadData];
	[self updateApplyButtonState];
}



- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
	(void)tableView; // Unused
	int numberOfRows = 0;
		
	if (browseDomainsArray) {
		numberOfRows = [browseDomainsArray count];
	}
	return numberOfRows;
}


- (void)tabView:(NSTabView *)xtabView didSelectTabViewItem:(NSTabViewItem *)tabViewItem
{
	(void)xtabView; // Unused
	(void)tabViewItem; // Unused
	[browseDomainList deselectAll:self];
	[mainWindow makeFirstResponder:nil];
}
 

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
	(void)tableView; // Unused
	NSDictionary *browseDomainDict;
	id           value = nil;
		
	if (browseDomainsArray) {
		browseDomainDict = [browseDomainsArray objectAtIndex:row];
		if (browseDomainDict) {
			if ([[tableColumn identifier] isEqualTo:(NSString *)SC_DYNDNS_ENABLED_KEY]) {
				value = [browseDomainDict objectForKey:(NSString *)SC_DYNDNS_ENABLED_KEY];
			} else if ([[tableColumn identifier] isEqualTo:(NSString *)SC_DYNDNS_DOMAIN_KEY]) {
                value = [browseDomainDict objectForKey:(NSString *)SC_DYNDNS_DOMAIN_KEY];
			}
		}
	}
	return value;
}


- (void)setupInitialValues
{    
    [self readPreferences];
    
    if (currentHostName) {
		[hostName setStringValue:currentHostName];
		[self updateStatusImageView];
	}
	
	if (browseDomainsArray) {
		browseDomainsArray = nil;
	}
	
	if (currentBrowseDomainsArray) {
		browseDomainsArray = [currentBrowseDomainsArray mutableCopy];
		if (browseDomainsArray) {
			[browseDomainsArray sortUsingFunction:MyDomainArrayCompareFunction context:nil];
			if ([browseDomainsArray isEqualToArray:currentBrowseDomainsArray] == NO) {
                [BonjourSCStore setObject: browseDomainsArray forKey: (NSString *)SC_DYNDNS_BROWSEDOMAINS_KEY];
				currentBrowseDomainsArray = [browseDomainsArray copy];
			}
		}
	} else {
		browseDomainsArray = nil;
	}
	[browseDomainList reloadData];
	
    if (currentRegDomain && ([currentRegDomain length] > 0)) {
        regDomainView.domain = currentRegDomain;
    }
    
    if (currentWideAreaState) {
        [self toggleWideAreaBonjour:YES];
    } else {
        [self toggleWideAreaBonjour:NO];
    }

    if (hostNameSharedSecretValue) {
        hostNameSharedSecretValue = nil;
    }
    
    if (regSharedSecretValue) {
        regSharedSecretValue = nil;
    }
    
    [self updateApplyButtonState];
    [mainWindow makeFirstResponder:nil];
	[browseDomainList deselectAll:self];
	[removeBrowseDomainButton setEnabled:NO];
}



- (void)awakeFromNib
{
    prefsNeedUpdating         = NO;
	browseDomainListEnabled   = NO;
	defaultRegDomain          = nil;
    currentRegDomain          = nil;
	currentBrowseDomainsArray = nil;
    currentHostName           = nil;
    hostNameSharedSecretValue = nil;
    regSharedSecretValue      = nil;
	browseDomainsArray        = nil;
    currentWideAreaState      = NO;
	NSString *successPath     = [[NSBundle bundleForClass:[self class]] pathForResource:@"success"    ofType:@"tiff"];
	NSString *inprogressPath  = [[NSBundle bundleForClass:[self class]] pathForResource:@"inprogress" ofType:@"tiff"];
	NSString *failurePath     = [[NSBundle bundleForClass:[self class]] pathForResource:@"failure"    ofType:@"tiff"];

    registrationDataSource    = [[NSMutableArray alloc] init];
	successImage              = [[NSImage alloc] initWithContentsOfFile:successPath];
	inprogressImage           = [[NSImage alloc] initWithContentsOfFile:inprogressPath];
	failureImage              = [[NSImage alloc] initWithContentsOfFile:failurePath];

    [tabView selectFirstTabViewItem:self];
    [self setupInitialValues];
    [self watchForPreferenceChanges];
    
}

- (void)willSelect
{
    [super willSelect];
    [bonjourBrowserView startBrowse];
    [registrationBrowserView startBrowse];
    
}

- (void)willUnselect
{
    [super willUnselect];
    [bonjourBrowserView stopBrowse];
    [registrationBrowserView stopBrowse];
}

- (IBAction)closeMyCustomSheet:(id)sender
{
    BOOL result = [sender tag];

    if (result) [NSApp endSheet:[sender window] returnCode:NSOKButton];
    else        [NSApp endSheet:[sender window] returnCode:NSCancelButton];
}


- (void)sharedSecretSheetDidEnd:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
    NSButton * button = (__bridge NSButton *)contextInfo;
    [sheet orderOut:self];
    [self enableControls];
    
    if (returnCode == NSOKButton) {
        if ([button isEqualTo:hostNameSharedSecretButton]) {
            hostNameSharedSecretName = [[NSString alloc] initWithString:[sharedSecretName stringValue]];
            hostNameSharedSecretValue = [[NSString alloc] initWithString:[sharedSecretValue stringValue]];
        } else {
            regSharedSecretName = [[NSString alloc] initWithString:[sharedSecretName stringValue]];
            regSharedSecretValue = [[NSString alloc] initWithString:[sharedSecretValue stringValue]];
        }
        [self updateApplyButtonState];
    }
    [sharedSecretValue setStringValue:@""];
}


- (BOOL)domainAlreadyInList:(NSString *)domainString
{
	if (browseDomainsArray) {
		NSDictionary *domainDict;
		NSString     *domainName;
		NSEnumerator *arrayEnumerator = [browseDomainsArray objectEnumerator];
		while ((domainDict = [arrayEnumerator nextObject]) != NULL) {
			domainName = [domainDict objectForKey:(NSString *)SC_DYNDNS_DOMAIN_KEY];
			if ([domainString caseInsensitiveCompare:domainName] == NSOrderedSame) return YES;
		}
	}
	return NO;
}


- (NSString *)trimCharactersFromDomain:(NSString *)domain
{
	NSMutableCharacterSet * trimSet = [NSMutableCharacterSet whitespaceCharacterSet];
	[trimSet formUnionWithCharacterSet:[NSCharacterSet punctuationCharacterSet]];
	return [domain stringByTrimmingCharactersInSet:trimSet];	
}


- (void)addBrowseDomainSheetDidEnd:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
	(void)contextInfo; // Unused
    [sheet orderOut:self];
    [self enableControls];
    
    if (returnCode == NSOKButton) {
        NSString * newBrowseDomainString;
        if(sheet == addBrowseDomainManualWindow)    newBrowseDomainString = [self trimCharactersFromDomain:[browseDomainTextField stringValue]];
        else                                        newBrowseDomainString = [self trimCharactersFromDomain:bonjourBrowserView.selectedDNSDomain];
		NSMutableDictionary *newBrowseDomainDict;
		if (browseDomainsArray == nil) browseDomainsArray = [[NSMutableArray alloc] initWithCapacity:0];
		if ([self domainAlreadyInList:newBrowseDomainString] == NO) {
			newBrowseDomainDict = [[NSMutableDictionary alloc] initWithCapacity:2];

			[newBrowseDomainDict setObject:newBrowseDomainString forKey:(NSString *)SC_DYNDNS_DOMAIN_KEY];
			[newBrowseDomainDict setObject:[NSNumber numberWithBool:YES] forKey:(NSString *)SC_DYNDNS_ENABLED_KEY];
			
			[browseDomainsArray addObject:newBrowseDomainDict];
			[browseDomainsArray sortUsingFunction:MyDomainArrayCompareFunction context:nil];
			[browseDomainList reloadData];
			[self updateApplyButtonState];
		}
    }
}

-(void)validateTextFields
{
    [hostName validateEditing];
    [browseDomainTextField validateEditing];
}


- (IBAction)changeButtonPressed:(id)sender
{
    NSString * keyName;
    
    [self disableControls];
    [self validateTextFields];
    [mainWindow makeFirstResponder:nil];
	[browseDomainList deselectAll:sender];

    if ([sender isEqualTo:hostNameSharedSecretButton]) {		
        if (hostNameSharedSecretValue) {
			[sharedSecretValue setStringValue:hostNameSharedSecretValue];
        } else if ((keyName = [self sharedSecretKeyName:[hostName stringValue]]) != NULL) {
			[sharedSecretName setStringValue:keyName];
            [sharedSecretValue setStringValue:@"****************"];
		} else {
			[sharedSecretName setStringValue:[hostName stringValue]];
            [sharedSecretValue setStringValue:@""];
        }

    } else {        
        if (regSharedSecretValue) {
			[sharedSecretValue setStringValue:regSharedSecretValue];
        } else if ((keyName = [self sharedSecretKeyName:regDomainView.domain]) != NULL) {
			[sharedSecretName setStringValue:keyName];
            [sharedSecretValue setStringValue:@"****************"];
		} else {
			[sharedSecretName setStringValue:regDomainView.domain];
            [sharedSecretValue setStringValue:@""];
        }
    }
    
    [sharedSecretWindow resignFirstResponder];

    if ([[sharedSecretName stringValue] length] > 0) [sharedSecretWindow makeFirstResponder:sharedSecretValue];
    else                                             [sharedSecretWindow makeFirstResponder:sharedSecretName];
    
    [NSApp beginSheet:sharedSecretWindow modalForWindow:mainWindow modalDelegate:self
            didEndSelector:@selector(sharedSecretSheetDidEnd:returnCode:contextInfo:) contextInfo:(__bridge void * _Null_unspecified)(sender)];
}


- (IBAction)selectWideAreaDomainButtonPressed:(id)sender
{
	NSWindow *  window = (([NSEvent modifierFlags] & NSAlternateKeyMask) == NSAlternateKeyMask) ? selectRegistrationDomainManualWindow : selectRegistrationDomainWindow;
	regDomainTextField.stringValue = regDomainView.domain;
	
    [self disableControls];
	[NSApp beginSheet:window modalForWindow:mainWindow modalDelegate:self
	   didEndSelector:@selector(selectWideAreaDomainSheetDidEnd:returnCode:contextInfo:) contextInfo:(__bridge void * _Null_unspecified)(sender)];
	
	[self updateApplyButtonState];
}

- (void)selectWideAreaDomainSheetDidEnd:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
	(void)contextInfo; // Unused
	[sheet orderOut:self];
	[self enableControls];
	
	if (returnCode == NSOKButton) {
		NSString * newRegDomainString;
		if(sheet == selectRegistrationDomainManualWindow) newRegDomainString = [self trimCharactersFromDomain:[regDomainTextField stringValue]];
		else                                              newRegDomainString = [self trimCharactersFromDomain:registrationBrowserView.selectedDNSDomain];
        regDomainView.domain = newRegDomainString;
        [self updateApplyButtonState];
	}
}

- (IBAction)wideAreaCheckBoxChanged:(id)sender
{    
    [self toggleWideAreaBonjour:[sender state]];
    [self updateApplyButtonState];
    [mainWindow makeFirstResponder:nil];
}



- (void)updateApplyButtonState
{
    NSString *hostNameString  = [hostName stringValue];
    NSString *regDomainString = regDomainView.domain;
    if ((currentHostName && ([hostNameString compare:currentHostName] != NSOrderedSame)) ||
        (currentRegDomain && ([regDomainString compare:currentRegDomain] != NSOrderedSame) && ([wideAreaCheckBox state])) ||
        (currentHostName == nil && ([hostNameString length]) > 0) ||
        (currentRegDomain == nil && ([regDomainString length]) > 0) ||
        (currentWideAreaState  != [wideAreaCheckBox state]) ||
        (hostNameSharedSecretValue != nil) ||
        (regSharedSecretValue != nil) ||
		(browseDomainsArray && [browseDomainsArray isEqualToArray:currentBrowseDomainsArray] == NO))
    {
        [self enableApplyButton];
    } else {
        [self disableApplyButton];
    }
}


- (void)controlTextDidChange:(NSNotification *)notification
{
	(void)notification; // Unused
    [self updateApplyButtonState];
}


- (NSMutableArray *)registrationDataSource
{
    return registrationDataSource;
}


- (NSString *)currentRegDomain
{
    return currentRegDomain;
}


- (NSArray *)currentBrowseDomainsArray
{
    return currentBrowseDomainsArray;
}


- (NSString *)currentHostName
{
    return currentHostName;
}


- (NSString *)defaultRegDomain
{
	return defaultRegDomain;
}


- (void)setDefaultRegDomain:(NSString *)domain
{
	defaultRegDomain = domain;
}


- (void)didSelect
{
    [super didSelect];
    mainWindow = [[self mainView] window];
}

- (void)mainViewDidLoad
{
    [comboAuthButton setString:"system.preferences"];
    [comboAuthButton setDelegate:self];
    [comboAuthButton setAutoupdate:YES];
    [super mainViewDidLoad];
}



- (IBAction)applyClicked:(id)sender
{
	(void)sender; // Unused
    [self applyCurrentState];
}


- (void)applyCurrentState
{
    [self validateTextFields];
    [self savePreferences];
    [self disableApplyButton];
    [mainWindow makeFirstResponder:nil];
}


- (void)enableApplyButton
{
    [applyButton setEnabled:YES];
    [revertButton setEnabled:YES];
    prefsNeedUpdating = YES;
}


- (void)disableApplyButton
{
    [applyButton setEnabled:NO];
    [revertButton setEnabled:NO];
    prefsNeedUpdating = NO;
}


- (void)toggleWideAreaBonjour:(BOOL)state
{
	[wideAreaCheckBox setState:state];
	[registrationSelectButton setEnabled:state];
	[registrationSharedSecretButton setEnabled:state];
}


- (IBAction)revertClicked:(id)sender
{
    [self restorePreferences];
	[browseDomainList deselectAll:sender];
    [mainWindow makeFirstResponder:nil];
}


- (void)restorePreferences
{
    [self setupInitialValues];
}


- (void)savePanelWillClose:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
	(void)sheet; // Unused
    DNSServiceDiscoveryPref * me = (__bridge DNSServiceDiscoveryPref *)contextInfo;
    
    if (returnCode == NSAlertDefaultReturn) {
        [me applyCurrentState];
    } else if (returnCode == NSAlertAlternateReturn ) {
        [me restorePreferences];
    }
    
    [me enableControls];
    [me replyToShouldUnselect:(returnCode != NSAlertOtherReturn)];
}


-(SecKeychainItemRef)copyKeychainItemforDomain:(NSString *)domain
{
    const char * serviceName = [domain UTF8String];
    UInt32 type              = 'ddns';
	UInt32 typeLength        = sizeof(type);

	SecKeychainAttribute attrs[] = { { kSecServiceItemAttr, strlen(serviceName),   (char *)serviceName },
                                     { kSecTypeItemAttr,             typeLength, (UInt32 *)&type       } };
    
	SecKeychainAttributeList attributes = { sizeof(attrs) / sizeof(attrs[0]), attrs };
    SecKeychainSearchRef searchRef;
    SecKeychainItemRef itemRef = NULL;
    OSStatus err;
    
    err = SecKeychainSearchCreateFromAttributes(NULL, kSecGenericPasswordItemClass, &attributes, &searchRef);
	if (err == noErr) {
		err = SecKeychainSearchCopyNext(searchRef, &itemRef);
		if (err != noErr) itemRef = NULL;
	}
	return itemRef;
}


-(NSString *)sharedSecretKeyName:(NSString * )domain
{
	SecKeychainItemRef itemRef = NULL;
	NSString *keyName = nil;
	OSStatus err;
	    
	err = SecKeychainSetPreferenceDomain(kSecPreferencesDomainSystem);
	assert(err == noErr);

	itemRef = [self copyKeychainItemforDomain:[domain lowercaseString]];
    if (itemRef) {
        UInt32 tags[1];
        SecKeychainAttributeInfo attrInfo;
        SecKeychainAttributeList *attrList = NULL;
        SecKeychainAttribute attribute;
		unsigned int i;
		
        tags[0] = kSecAccountItemAttr;
        attrInfo.count = 1;
        attrInfo.tag = tags;
        attrInfo.format = NULL;
					
        err = SecKeychainItemCopyAttributesAndData(itemRef,  &attrInfo, NULL, &attrList, NULL, NULL);
        if (err == noErr) {
            for (i = 0; i < attrList->count; i++) {
                attribute = attrList->attr[i];
                if (attribute.tag == kSecAccountItemAttr) {
                    keyName = [[NSString alloc] initWithBytes:attribute.data length:attribute.length encoding:NSUTF8StringEncoding];
                    break;
                }
            }
            if (attrList) (void)SecKeychainItemFreeAttributesAndData(attrList, NULL);
        }
		CFRelease(itemRef);
	}
    return keyName;
}


-(NSString *)domainForHostName:(NSString *)hostNameString
{
    NSString * domainName = nil;
    char text[64];
    char * ptr = NULL;
    
    ptr = (char *)[hostNameString UTF8String];
    if (ptr) {
        ptr = (char *)GetNextLabel(ptr, text);
        domainName = [[NSString alloc] initWithUTF8String:(const char *)ptr];             
    }
    return (domainName);
}


- (NSData *)dataForDomain:(NSString *)domainName isEnabled:(BOOL)enabled
{
	NSMutableArray      *domainsArray; 
	NSMutableDictionary *domainDict = nil;
	
	if (domainName && [domainName length] > 0) {
		domainDict= [NSMutableDictionary dictionaryWithCapacity:2];
		[domainDict setObject:domainName forKey:(NSString *)SC_DYNDNS_DOMAIN_KEY];
		[domainDict setObject:[NSNumber numberWithBool:enabled] forKey:(NSString *)SC_DYNDNS_ENABLED_KEY];
	}
	domainsArray = [NSMutableArray arrayWithCapacity:1];
	if (domainDict) [domainsArray addObject:domainDict];
	return [NSArchiver archivedDataWithRootObject:domainsArray];
}


- (NSData *)dataForDomainArray:(NSArray *)domainArray
{
	return [NSArchiver archivedDataWithRootObject:domainArray];
}


- (NSDictionary *)dictionaryForSharedSecret:(NSString *)secret domain:(NSString *)domainName key:(NSString *)keyName
{
	NSMutableDictionary *sharedSecretDict = [NSMutableDictionary dictionaryWithCapacity:3];
	[sharedSecretDict setObject:secret forKey:(NSString *)SC_DYNDNS_SECRET_KEY];
	[sharedSecretDict setObject:[domainName lowercaseString] forKey:(NSString *)SC_DYNDNS_DOMAIN_KEY];
	[sharedSecretDict setObject:keyName forKey:(NSString *)SC_DYNDNS_KEYNAME_KEY];
	return sharedSecretDict;
}


-(void)savePreferences
{
    NSString      *hostNameString               = [hostName stringValue];
    NSString      *regDomainString              = regDomainView.domain;
    NSString      *tempHostNameSharedSecretName = hostNameSharedSecretName;
    NSString      *tempRegSharedSecretName      = regSharedSecretName;
    BOOL          regSecretWasSet               = NO;
    BOOL          hostSecretWasSet              = NO;
    BOOL          updateHostname                = NO;

	hostNameString                = [self trimCharactersFromDomain:hostNameString];
	regDomainString               = [self trimCharactersFromDomain:regDomainString];
	tempHostNameSharedSecretName  = [self trimCharactersFromDomain:tempHostNameSharedSecretName];
	tempRegSharedSecretName       = [self trimCharactersFromDomain:tempRegSharedSecretName];
	
	[hostName setStringValue:hostNameString];
	regDomainView.domain = regDomainString;
    
    // Convert Shared Secret account names to lowercase.
    tempHostNameSharedSecretName = [tempHostNameSharedSecretName lowercaseString];
    tempRegSharedSecretName      = [tempRegSharedSecretName lowercaseString];
    
    // Save hostname shared secret.
    if ([hostNameSharedSecretName length] > 0 && ([hostNameSharedSecretValue length] > 0)) {
        DNSPrefTool_SetKeychainEntry([self dictionaryForSharedSecret:hostNameSharedSecretValue domain:hostNameString key:tempHostNameSharedSecretName]);
        hostNameSharedSecretValue = nil;
        hostSecretWasSet = YES;
    }
    
    // Save registration domain shared secret.
    if (([regSharedSecretName length] > 0) && ([regSharedSecretValue length] > 0)) {
        DNSPrefTool_SetKeychainEntry([self dictionaryForSharedSecret:regSharedSecretValue domain:regDomainString key:tempRegSharedSecretName]);
        regSharedSecretValue = nil;
        regSecretWasSet = YES;
    }

    // Save hostname.
    if ((currentHostName == NULL) || [currentHostName compare:hostNameString] != NSOrderedSame) {
        currentHostName = [hostNameString copy];
        updateHostname = YES;
    } else if (hostSecretWasSet) {
        currentHostName = @"";
        updateHostname = YES;
    }

    if (updateHostname) {
        [BonjourSCStore setObject: currentHostName.length ? @[@{
                                                                   (NSString *)SC_DYNDNS_DOMAIN_KEY  : currentHostName,
                                                                   (NSString *)SC_DYNDNS_ENABLED_KEY : @YES
                                                                   }] : nil
                           forKey: (NSString *)SC_DYNDNS_HOSTNAMES_KEY];
    }
    
    // Save browse domain.
	if (browseDomainsArray && [browseDomainsArray isEqualToArray:currentBrowseDomainsArray] == NO) {
        [BonjourSCStore setObject: browseDomainsArray forKey: (NSString *)SC_DYNDNS_BROWSEDOMAINS_KEY];
		currentBrowseDomainsArray = [browseDomainsArray copy];
    }
	
    // Save registration domain.
    if ((currentRegDomain == NULL) || ([currentRegDomain compare:regDomainString] != NSOrderedSame) || (currentWideAreaState != [wideAreaCheckBox state])) {
        [BonjourSCStore setObject: @[@{
                                         (NSString *)SC_DYNDNS_DOMAIN_KEY  : regDomainString,
                                         (NSString *)SC_DYNDNS_ENABLED_KEY : [wideAreaCheckBox state] ? @YES : @NO
                                         }]
                           forKey: (NSString *)SC_DYNDNS_REGDOMAINS_KEY];
        currentRegDomain = [regDomainString copy];

        if ([currentRegDomain length] > 0) {
			currentWideAreaState = [wideAreaCheckBox state];
            [registrationDataSource removeObject:regDomainString];
            [registrationDataSource addObject:currentRegDomain];
            [registrationDataSource sortUsingFunction:MyArrayCompareFunction context:nil];
 //           [regDomainsComboBox reloadData];
        } else {
			currentWideAreaState = NO;
			[self toggleWideAreaBonjour:NO];
            if (defaultRegDomain != nil) regDomainView.domain = defaultRegDomain;
		}
    } else if (regSecretWasSet) {
        [BonjourSCStore setObject: @[@{
                                         (NSString *)SC_DYNDNS_DOMAIN_KEY  : @"",
                                         (NSString *)SC_DYNDNS_ENABLED_KEY : @NO
                                         }]
                           forKey: (NSString *)SC_DYNDNS_REGDOMAINS_KEY];
        if ([currentRegDomain length] > 0) {
            [BonjourSCStore setObject: @[@{
                                             (NSString *)SC_DYNDNS_DOMAIN_KEY  : currentRegDomain,
                                             (NSString *)SC_DYNDNS_ENABLED_KEY : currentWideAreaState ? @YES : @NO
                                             }]
                               forKey: (NSString *)SC_DYNDNS_REGDOMAINS_KEY];
        }
    }
}   


- (NSPreferencePaneUnselectReply)shouldUnselect
{
#if 1
    if (prefsNeedUpdating == YES) {
    
        [self disableControls];
        
        NSBeginAlertSheet(
                    @"Apply Configuration Changes?",
                    @"Apply",
                    @"Don't Apply",
                    @"Cancel",
                    mainWindow,
                    self,
                    @selector( savePanelWillClose:returnCode:contextInfo: ),
                    NULL,
                    (__bridge void *) self, // sender,
                    @"" );
        return NSUnselectLater;
    }
#endif
    
    return NSUnselectNow;
}


-(void)disableControls
{
    [hostName setEnabled:NO];
    [hostNameSharedSecretButton setEnabled:NO];
    [applyButton setEnabled:NO];
    [revertButton setEnabled:NO];
    [wideAreaCheckBox setEnabled:NO];
	[registrationSelectButton setEnabled: NO];
    [registrationSharedSecretButton setEnabled:NO];
    [statusImageView setEnabled:NO];
	
	browseDomainListEnabled = NO;
	[browseDomainList deselectAll:self];
	[browseDomainList setEnabled:NO];
	
	[addBrowseDomainButton setEnabled:NO];
	[removeBrowseDomainButton setEnabled:NO];
}


- (BOOL)tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row
{
	(void)row; // Unused
	(void)tableView; // Unused
	return browseDomainListEnabled;
}


-(void)enableControls
{
    [hostName setEnabled:YES];
    [hostNameSharedSecretButton setEnabled:YES];
    [wideAreaCheckBox setEnabled:YES];
	[registrationSelectButton setEnabled: YES];
    [registrationSharedSecretButton setEnabled:YES];
    [self toggleWideAreaBonjour:[wideAreaCheckBox state]];
    [statusImageView setEnabled:YES];
	[addBrowseDomainButton setEnabled:YES];

	[browseDomainList setEnabled:YES];
	[browseDomainList deselectAll:self];
	browseDomainListEnabled = YES;

	[removeBrowseDomainButton setEnabled:[browseDomainList numberOfSelectedRows]];
	[applyButton setEnabled:prefsNeedUpdating];
	[revertButton setEnabled:prefsNeedUpdating];
}


- (void)authorizationViewDidAuthorize:(SFAuthorizationView *)view
{
    (void)view; //  unused
    [self enableControls];
}


- (void)authorizationViewDidDeauthorize:(SFAuthorizationView *)view
{    
    (void)view; //  unused
    [self disableControls];
}

@end


// Note: The C preprocessor stringify operator ('#') makes a string from its argument, without macro expansion
// e.g. If "version" is #define'd to be "4", then STRINGIFY_AWE(version) will return the string "version", not "4"
// To expand "version" to its value before making the string, use STRINGIFY(version) instead
#define STRINGIFY_ARGUMENT_WITHOUT_EXPANSION(s) #s
#define STRINGIFY(s) STRINGIFY_ARGUMENT_WITHOUT_EXPANSION(s)

// NOT static -- otherwise the compiler may optimize it out
// The "@(#) " pattern is a special prefix the "what" command looks for
const char VersionString_SCCS[] = "@(#) Bonjour Preference Pane " STRINGIFY(mDNSResponderVersion) " (" __DATE__ " " __TIME__ ")";

#if _BUILDING_XCODE_PROJECT_
// If the process crashes, then this string will be magically included in the automatically-generated crash log
const char *__crashreporter_info__ = VersionString_SCCS + 5;
asm(".desc ___crashreporter_info__, 0x10");
#endif
