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

#import "BonjourSettingsController.h"
#import "HostnameController.h"
#import "BonjourSCStore.h"
#import "CNBrowseDomainsController.h"
#import <AssertMacros.h>

#define LocalizedStringFromMyBundle(key, comment)     \
    NSLocalizedStringFromTableInBundle(key, @"Localizable", [NSBundle bundleForClass: [self class]], comment)

@interface BonjourSettingsController ()

@property (strong) NSString *               bonjourHostname;
@property (strong) NSArray *                browseDomainsA;

@end

@implementation BonjourSettingsController

- (instancetype)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    if (self = [super initWithNibName: nibNameOrNil bundle: nibBundleOrNil])
    {
        [self commonInit];
    }
    return(self);
}

- (void)commonInit
{
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear: animated];
    [self readPreferences];
    [self reloadSpecifiers];
}

- (id) getDomainCount:(PSSpecifier *)specifier
{
    (void)specifier;    // Unused
    return [NSNumber numberWithInteger: [_browseDomainsA filteredArrayUsingPredicate: [NSPredicate predicateWithFormat: @"(%K == %@)", (NSString *)SC_DYNDNS_ENABLED_KEY, @YES]].count].stringValue;
}

- (id) getHostname:(PSSpecifier *)specifier
{
    (void)specifier;    // Unused
    return _bonjourHostname.length ? _bonjourHostname : LocalizedStringFromMyBundle(@"_bonjour.hostname.unset", nil);
}

- (NSArray *)specifiers
{
    if (!_specifiers) {
        PSSpecifier * specifier;
        NSMutableArray * specifiers = [NSMutableArray array];
        
        specifier = [PSSpecifier groupSpecifierWithName: LocalizedStringFromMyBundle(@"_bonjour.hostname.groupname", nil)];
        [specifiers addObject: specifier];
        specifier = [PSSpecifier preferenceSpecifierNamed: LocalizedStringFromMyBundle(@"_bonjour.hostname.name", nil)
                                                   target: self
                                                      set: nil
                                                      get: @selector(getHostname:)
                                                   detail: [HostnameController class]
                                                     cell: PSLinkListCell
                                                     edit: nil];
        [specifier setProperty: @"hostnameID"
                        forKey: PSIDKey];
        [specifiers addObject: specifier];
        
        specifier = [PSSpecifier groupSpecifierWithName: LocalizedStringFromMyBundle(@"_bonjour.browse.groupname", nil)];
        [specifiers addObject: specifier];
        specifier = [PSSpecifier preferenceSpecifierNamed: LocalizedStringFromMyBundle(@"_bonjour.browse.name", nil)
                                                   target: self
                                                      set: nil
                                                      get: @selector(getDomainCount:)
                                                   detail: [CNBrowseDomainsController class]
                                                     cell: PSLinkListCell
                                                     edit: nil];
        [specifier setProperty: @"browseID"
                        forKey: PSIDKey];
        [specifiers addObject: specifier];
       
        _specifiers = specifiers;
    }
    return _specifiers;
}

-(void)readPreferences
{
    self.browseDomainsA = [BonjourSCStore objectForKey: (NSString *)SC_DYNDNS_BROWSEDOMAINS_KEY];
    
    NSArray * hostArray;
    hostArray = [BonjourSCStore objectForKey: (NSString *)SC_DYNDNS_HOSTNAMES_KEY];
    if (hostArray && [hostArray count] > 0)
    {
        self.bonjourHostname = hostArray[0][(NSString *)SC_DYNDNS_DOMAIN_KEY];
    }
    else self.bonjourHostname = nil;
    
    if (!_browseDomainsA) self.browseDomainsA = [NSMutableArray array];
    if (!_bonjourHostname) self.bonjourHostname = [NSString string];
}

#pragma mark - TableView Delegates

- (void)listItemSelected:(NSIndexPath *)indexPath //sender is NSIndexPath of selection
{
    if (indexPath.section == 0)
    {
        HostnameController * c = [[HostnameController alloc] initWithStyle: UITableViewStyleGrouped];
        c.bonjourHostname = _bonjourHostname;
        c.title = LocalizedStringFromMyBundle(@"_bonjour.hostname.name", nil);
        [self.navigationController pushViewController: c animated: YES];
    }
    else if (indexPath.section == 1)
    {
        CNBrowseDomainsController * c = [[CNBrowseDomainsController alloc] initWithStyle: UITableViewStyleGrouped];
        c.browseDomainsA = _browseDomainsA;
        c.title = LocalizedStringFromMyBundle(@"_bonjour.browse.name", nil);
        [self.navigationController pushViewController: c animated: YES];
    }
    
    [_table deselectRowAtIndexPath: indexPath animated: YES];
}

@end
