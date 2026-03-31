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

#import "HostnameController.h"
#import "BonjourSCStore.h"
#import <AssertMacros.h>
#import <Preferences/Preferences.h>

#define LocalizedStringFromMyBundle(key, comment)     \
    NSLocalizedStringFromTableInBundle(key, @"Localizable", [NSBundle bundleForClass: [self class]], comment)

@interface HostnameController ()

@property (strong) UITextField *            textField;

@end

@implementation HostnameController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.tableView.dataSource = (id<UITableViewDataSource>)self;
    self.tableView.delegate = (id<UITableViewDelegate>)self;
}

- (void)viewWillDisappear:(BOOL)animated
{
    [super viewWillDisappear: animated];
    if (self.isMovingFromParentViewController)
    {
        [[self firstResponder] resignFirstResponder];       //  Ends any outstanding edits
        self.bonjourHostname = _textField.attributedText.string;
        [self savePreferences];    }
}

-(void)savePreferences
{
    [BonjourSCStore setObject: _bonjourHostname.length ? @[@{
                                                               (NSString *)SC_DYNDNS_DOMAIN_KEY  : _bonjourHostname,
                                                               (NSString *)SC_DYNDNS_ENABLED_KEY : @YES
                                                            }] : nil
                       forKey: (NSString *)SC_DYNDNS_HOSTNAMES_KEY];
}

- (void)_setHostname:(NSString *)value
{
    self.bonjourHostname = value;
}

#pragma mark - TableView Delegates

- (CGFloat)tableView:(UITableView *)tableView estimatedHeightForRowAtIndexPath:(NSIndexPath *)indexPath
{
    (void)tableView;    // Unused
    (void)indexPath;    // Unused
    return UITableViewAutomaticDimension;
}

- (nullable NSString *)tableView:(UITableView *)tableView titleForFooterInSection:(NSInteger)section
{
    (void)tableView;    // Unused
    (void)section;      // Unused
    return(LocalizedStringFromMyBundle(@"_bonjour.hostname.desc", nil));
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    (void)tableView;    // Unused
    (void)section;      // Unused
    return(1);
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    PSEditableTableCell *cell = nil;
    
    if (tableView == self.tableView && indexPath.section == 0)
    {
        static NSString *MyIdentifier = @"hostname_cell_id";
        cell = (PSEditableTableCell *)[tableView dequeueReusableCellWithIdentifier: MyIdentifier];
        if (!cell)
        {
            cell = [[PSEditableTableCell alloc] initWithStyle: [PSEditableTableCell cellStyle]  reuseIdentifier: MyIdentifier];
        }
        cell.placeholderText = LocalizedStringFromMyBundle(@"_bonjour.hostname.placeholder", nil);
        cell.textField.autocapitalizationType = UITextAutocapitalizationTypeNone;
        cell.textField.clearButtonMode = UITextFieldViewModeAlways;
        cell.textField.autocorrectionType = UITextAutocorrectionTypeNo;
        cell.textField.keyboardType = UIKeyboardTypeURL;
        cell.title = @"";
        cell.value = _bonjourHostname;
        self.textField = cell.textField;
    }
    
    return(cell);
}

- (BOOL)tableView:(UITableView *)tableView shouldHighlightRowAtIndexPath:(NSIndexPath *)indexPath
{
    (void)tableView;    // Unused
    BOOL    result = YES;
    
    if (indexPath.section == 0 && indexPath.row == 0) result = NO;
    
    return(result);
}

@end

