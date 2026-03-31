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

#import "CNBrowseDomainsController.h"
#import "CNDomainBrowserViewController.h"
#import "CNDomainBrowserPathUtils.h"
#import "BonjourSCStore.h"
#import <AssertMacros.h>

#define kTag_AddBrowse                      100
#define kTag_AddManual                      101

const NSString *    _CNBrowseDomainKey_fullname     = (NSString *)SC_DYNDNS_DOMAIN_KEY;
const NSString *    _CNBrowseDomainKey_enabled      = (NSString *)SC_DYNDNS_ENABLED_KEY;

#define LocalizedStringFromMyBundle(key, comment )     \
        NSLocalizedStringFromTableInBundle(key, @"Localizable", [NSBundle bundleForClass: [self class]], comment )

@interface CNPathPopoverViewController : UIViewController

@property (strong) UILabel *    label;
@property (copy)   NSArray *    pathArray;
@property (assign) CGPoint      offset;

@end

@implementation CNPathPopoverViewController

- (instancetype)initWithPathArray:(NSArray *)pathArray
{
    if (self = [self initWithNibName: nil bundle: nil] )
    {
        _pathArray = pathArray;
        NSMutableParagraphStyle * paragraphStyle = [[NSMutableParagraphStyle alloc] init];
        [paragraphStyle setLineSpacing: [UIFont labelFontSize] * 0.3];
        
        NSMutableAttributedString * itemStr = [[NSMutableAttributedString alloc] init];
        NSUInteger  count = 0;
        for (NSString * next in _pathArray )
        {
            NSString * nextLine = [NSString stringWithFormat: @"%@%@", itemStr.length ? @"\n" : @"", next];
            UIColor *  nextColor = ((count++ == 0) || (count == _pathArray.count)) ? [UIColor grayColor] : [UIColor blackColor];
            [itemStr appendAttributedString:
             [[NSMutableAttributedString alloc] initWithString: nextLine
                                                    attributes: @{ NSForegroundColorAttributeName: nextColor,
                                                                   NSParagraphStyleAttributeName: paragraphStyle
                                                                }]];
        }

        _label = [[UILabel alloc] initWithFrame: CGRectZero];
        _label.numberOfLines = 0;
        _label.attributedText = itemStr;
        [_label sizeToFit];
        
        _offset = CGPointMake(25, 15 );
        CGRect rect = _label.frame;
        rect.origin.x += _offset.x;
        rect.origin.y += _offset.y;
        [_label setFrame: rect];
    }
    return(self );
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    [self.view addSubview: _label];
}

- (void)willTransitionToTraitCollection:(UITraitCollection *)newCollection withTransitionCoordinator:(id <UIViewControllerTransitionCoordinator>)coordinator
{
    [super willTransitionToTraitCollection: newCollection withTransitionCoordinator: coordinator];
    [self.presentingViewController dismissViewControllerAnimated: NO completion: nil];
}

@end

@interface CNPathAccessoryView : UIView

@property (strong, readonly) UIButton *         elipsisButton;
@property (strong, readonly) UIButton *         pathLabel;
@property (weak)             UITableView *      tableView;
@property (weak)             UITableViewCell *  cell;
@property (strong, readonly) NSArray *          pathArray;

@end

@implementation CNPathAccessoryView

- (instancetype) initWithFrame:(CGRect)frame pathArray:(NSArray *)pathArray
{
    self = [super initWithFrame:frame];
    if (self) {
        _pathArray = pathArray;
        if (_pathArray )
        {
            _pathLabel = [UIButton buttonWithType: UIButtonTypeCustom];
            [_pathLabel setTitle: _pathArray[_pathArray.count-1] forState: UIControlStateNormal];
            [_pathLabel setTitleColor: [UIColor blackColor] forState: UIControlStateNormal];
            _pathLabel.titleLabel.font = [UIFont systemFontOfSize: [UIFont labelFontSize]];
            [self addSubview: _pathLabel];
            
            _pathLabel.translatesAutoresizingMaskIntoConstraints = NO;
            [_pathLabel sizeToFit];
            [_pathLabel.widthAnchor      constraintEqualToConstant: _pathLabel.frame.size.width].active = YES;
            [_pathLabel.trailingAnchor   constraintEqualToAnchor: self.trailingAnchor].active = YES;
            [_pathLabel.topAnchor        constraintEqualToAnchor: self.topAnchor].active = YES;
            [_pathLabel.bottomAnchor     constraintEqualToAnchor: self.bottomAnchor].active = YES;
        }
        if (_pathArray.count > 2 )
        {
            _elipsisButton = [UIButton buttonWithType: UIButtonTypeCustom];
            [_elipsisButton setTitle: @"…" forState: UIControlStateNormal];
            [_elipsisButton setTitleColor: _elipsisButton.tintColor forState: UIControlStateNormal];
            [_elipsisButton setTitleColor: [UIColor grayColor] forState: UIControlStateHighlighted];
            _elipsisButton.titleLabel.font = [UIFont boldSystemFontOfSize: [UIFont labelFontSize]];
            [_elipsisButton addTarget: self action: @selector(pathButtonPressed:withEvent:) forControlEvents: UIControlEventTouchUpInside];
            _elipsisButton.userInteractionEnabled = YES;
            [self addSubview: _elipsisButton];

            _elipsisButton.translatesAutoresizingMaskIntoConstraints = NO;
            [_elipsisButton sizeToFit];
            [_elipsisButton.widthAnchor      constraintEqualToConstant: _elipsisButton.frame.size.width].active = YES;
            [_elipsisButton.trailingAnchor   constraintEqualToAnchor: _pathLabel.leadingAnchor].active = YES;
            [_elipsisButton.topAnchor        constraintGreaterThanOrEqualToAnchor: self.topAnchor].active = YES;
            [_elipsisButton.bottomAnchor     constraintLessThanOrEqualToAnchor: self.bottomAnchor].active = YES;
        }
     }
    return self;
}

- (BOOL) canBecomeFirstResponder
{
    return YES;
}

- (UIModalPresentationStyle)adaptivePresentationStyleForPresentationController:(UIPresentationController *)controller traitCollection:(UITraitCollection *)traitCollection
{
    (void)controller;       // Unused
    (void)traitCollection;  // Unused
    return UIModalPresentationNone;
}

- (void) pathButtonPressed:(UIControl *)button withEvent:(UIEvent *)event
{
    (void)button;           // Unused
    
    if (!self.cell.showingDeleteConfirmation )
    {
        [self becomeFirstResponder];
        
        UIView *buttonView = [[event.allTouches anyObject] view];
        CGRect buttonFrame = [buttonView convertRect: buttonView.frame toView: self];
        
        CNPathPopoverViewController * controller = [[CNPathPopoverViewController alloc] initWithPathArray: self.pathArray];
        controller.modalPresentationStyle = UIModalPresentationPopover;
        controller.preferredContentSize = CGSizeMake(controller.label.frame.size.width + controller.offset.x * 2, controller.label.frame.size.height + controller.offset.y * 2 );
        
        UIPopoverPresentationController *popover =  controller.popoverPresentationController;
        popover.delegate = (id<UIPopoverPresentationControllerDelegate>)self;
        popover.sourceView = buttonView;
        popover.sourceRect = buttonFrame;
        popover.permittedArrowDirections = UIPopoverArrowDirectionLeft | UIPopoverArrowDirectionRight;
        
        [(UIViewController *)self.tableView.delegate presentViewController: controller
                                                                  animated: YES
                                                                completion: nil];
    }
}

- (CGSize) sizeThatFits:(CGSize)size
{
    (void)size;         // Unused
    CGSize ret = CGSizeZero;
    CGFloat maxX = 0.0;
    CGFloat maxHeight = 0.0;
    for (UIView *view in [self subviews] )
    {
        CGRect frame = view.frame;
        CGFloat frameMaxX = CGRectGetMaxX(frame);
        CGFloat frameHeight = CGRectGetHeight(frame);
        maxX += frameMaxX;
        if (frameHeight > maxHeight )
        {
            maxHeight = frameHeight;
        }
    }
    ret.width = maxX;
    ret.height = maxHeight;
    return ret;
}

@end

@interface CNManualDomainViewController : UITableViewController

@property (weak)        CNBrowseDomainsController * delegate;
@property (weak)        UITextField *               textField;

@end

#define kTag_EditTextField          103

@implementation CNManualDomainViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.tableView.dataSource = (id<UITableViewDataSource>)self;
    self.tableView.delegate = (id<UITableViewDelegate>)self;
    
    self.title = LocalizedStringFromMyBundle(@"_dnsBrowser.manualdomain.title", nil );
    self.navigationItem.leftBarButtonItem = [[UIBarButtonItem alloc] initWithTitle: LocalizedStringFromMyBundle(@"_dnsBrowser.manualdomain.cancel", nil )
                                                                             style: UIBarButtonItemStylePlain
                                                                            target: self
                                                                            action: @selector(cancelAction:)];
    
    self.navigationItem.rightBarButtonItem = [[UIBarButtonItem alloc] initWithTitle: LocalizedStringFromMyBundle(@"_dnsBrowser.manualdomain.add", nil )
                                                                             style: UIBarButtonItemStylePlain
                                                                            target: self
                                                                            action: @selector(addAction:)];
    self.navigationItem.rightBarButtonItem.enabled = NO;
}

- (void)viewWillDisappear:(BOOL)animated
{
    [super viewWillDisappear: animated];
    if (self.isMovingFromParentViewController )
    {
        [[NSNotificationCenter defaultCenter] removeObserver: self];
    }
}

- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear: animated];
    if (self.isMovingToParentViewController )
    {
        [self.textField becomeFirstResponder];
    }
}

- (IBAction)addAction:(id)sender
{
    (void)sender;   // Unused
    [self.navigationController dismissViewControllerAnimated: YES completion: ^{
        [self.delegate addBrowseDomain: self.textField.text];
    }];
}

- (IBAction)cancelAction:(id)sender
{
    (void)sender;   // Unused
    [self.navigationController dismissViewControllerAnimated: YES completion: nil];
}

#pragma mark - Notifications

- (void)textFieldDidChange:(NSNotification *)notification
{
    UITextField * textField = (UITextField *)notification.object;
    self.navigationItem.rightBarButtonItem.enabled = (textField.text.length > 0);
}

#pragma mark - TableView Delegates

- (CGFloat)tableView:(UITableView *)tableView estimatedHeightForRowAtIndexPath:(NSIndexPath *)indexPath
{
    (void)tableView;        // Unused
    (void)indexPath;        // Unused
    return UITableViewAutomaticDimension;
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    (void)tableView;        // Unused
    NSInteger result = 0;
    
    if (tableView == self.tableView )   result = 1;
    
    return(result );
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    (void)section;        // Unused
    NSInteger result = 0;
    
    if (tableView == self.tableView && section == 0 ) result = 1;
    
    return(result );
}

- (nullable NSString *)tableView:(UITableView *)tableView titleForFooterInSection:(NSInteger)section
{
    NSString * result = nil;
    
    if (tableView == self.tableView && section == 0 )
    {
        result = LocalizedStringFromMyBundle(@"_dnsBrowser.manualdomain.footer", nil );
    }
    
    return(result );
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    UITableViewCell *cell = nil;
    
    if (tableView == self.tableView )
    {
        if (indexPath.section == 0 && indexPath.row == 0 )
        {
            static NSString *MyIdentifier = @"manual_domain_cell_id";
            cell = [tableView dequeueReusableCellWithIdentifier: MyIdentifier];
            if (!cell )
            {
                cell = [[UITableViewCell alloc] initWithStyle: UITableViewCellStyleValue1  reuseIdentifier: MyIdentifier];
            }
            
            cell.textLabel.hidden = YES;
            cell.detailTextLabel.hidden = YES;
            
            [[cell viewWithTag: kTag_EditTextField] removeFromSuperview];
            UITextField * textField = [[UITextField alloc] initWithFrame:CGRectZero];
            textField.tag = kTag_EditTextField;
            textField.adjustsFontSizeToFitWidth = YES;
            textField.placeholder = LocalizedStringFromMyBundle(@"_dnsBrowser.manualdomain.defaultValue", nil );
            textField.keyboardType = UIKeyboardTypeURL;
            textField.autocapitalizationType = UITextAutocapitalizationTypeNone;
            textField.autocorrectionType = UITextAutocorrectionTypeNo;
            textField.enablesReturnKeyAutomatically = YES;
            textField.clearButtonMode = UITextFieldViewModeAlways;
            [cell.contentView addSubview: textField];
            
            textField.translatesAutoresizingMaskIntoConstraints = NO;
            [textField.leadingAnchor    constraintEqualToAnchor: cell.layoutMarginsGuide.leadingAnchor].active = YES;
            [textField.trailingAnchor   constraintEqualToAnchor: cell.layoutMarginsGuide.trailingAnchor].active = YES;
            [textField.topAnchor        constraintEqualToAnchor: cell.layoutMarginsGuide.topAnchor].active = YES;
            [textField.bottomAnchor     constraintEqualToAnchor: cell.layoutMarginsGuide.bottomAnchor].active = YES;

            [[NSNotificationCenter defaultCenter] addObserver: self
                                                     selector: @selector(textFieldDidChange:)
                                                         name: UITextFieldTextDidChangeNotification
                                                       object: textField];
            self.textField = textField;
        }
    }
    
    return(cell );
}

- (nullable NSIndexPath *)tableView:(UITableView *)tableView willSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    (void)tableView;    // Unused
    (void)indexPath;    // Unused
    return(nil );
}

@end

@interface CNBrowseDomainsController ()

@property (strong) NSDictionary *                   selectedInstance;
@property (strong) NSMutableDictionary *            instanceInfoStrings;
@property (strong) NSMutableDictionary *            instanceStatusViews;

@property (strong) CNDomainBrowserViewController *  browseController;

@end

@implementation CNBrowseDomainsController

- (instancetype)initWithStyle:(UITableViewStyle)style
{
    if (self = [super initWithStyle: style] )
    {
        [self commonInit];
    }
    return(self );
}

- (void)contentViewsInit
{
    self.tableView.allowsMultipleSelectionDuringEditing = NO;
}

- (void)commonInit
{
    self.instanceInfoStrings = [NSMutableDictionary dictionary];
    self.instanceStatusViews = [NSMutableDictionary dictionary];
    self.browseController = [[CNDomainBrowserViewController alloc] initWithStyle: UITableViewStylePlain];
    _browseController.delegate = (id<CNDomainBrowserViewControllerDelegate>)self;
    _browseController.ignoreLocal = YES;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    [self contentViewsInit];
    
    self.tableView.dataSource = (id<UITableViewDataSource>)self;
    self.tableView.delegate = (id<UITableViewDelegate>)self;
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear: animated];
    if (self.isMovingToParentViewController)
    {
        [_browseController startBrowse];
    }
}

- (void)viewWillDisappear:(BOOL)animated
{
    [super viewWillDisappear: animated];
    if (self.isMovingFromParentViewController )
    {
        [self savePreferences];
        [_browseController stopBrowse];
    }
}

-(void)savePreferences
{
    [BonjourSCStore setObject: _browseDomainsA.count ? _browseDomainsA : nil forKey: (NSString *)SC_DYNDNS_BROWSEDOMAINS_KEY];
}

- (NSDictionary *)browseDomainForRow:(NSInteger)row
{
    NSDictionary *      result = nil;
    NSInteger           curRow = 0;
    
    for (NSDictionary * nextDomain in self.browseDomainsA )
    {
        if (curRow == row )
        {
            result = nextDomain;
            break;
        }
        curRow++;
    }
    
    return(result );
}

- (NSInteger)indexOfDomainInList:(NSString *)domainString
{
    NSInteger  index = 0;
    if (_browseDomainsA ) {
        NSDictionary *domainDict;
        NSString     *domainName;
        NSEnumerator *arrayEnumerator = [_browseDomainsA objectEnumerator];
        while ((domainDict = [arrayEnumerator nextObject]) != NULL)
        {
            domainName = [domainDict objectForKey:_CNBrowseDomainKey_fullname];
            if ([domainString caseInsensitiveCompare:domainName] == NSOrderedSame) return index;
            index++;
        }
    }
    return NSNotFound;
}

- (void)addBrowseDomain:(NSString *)fullPath
{
    NSString * trimmedPath = TrimCharactersFromDNSDomain(fullPath );
    NSInteger index = [self indexOfDomainInList: trimmedPath];
    NSMutableArray * domains = [NSMutableArray arrayWithArray: self.browseDomainsA];

    if (index == NSNotFound )
    {
        [domains addObject: @{
                              _CNBrowseDomainKey_fullname: trimmedPath,
                              _CNBrowseDomainKey_enabled: @YES
                              }];
        
        self.browseDomainsA = [NSArray arrayWithArray: domains];
        [self.tableView insertRowsAtIndexPaths: @[[NSIndexPath indexPathForRow: domains.count-1 inSection: 0]] withRowAnimation: UITableViewRowAnimationAutomatic];
    }
    else
    {
        NSDictionary * domain = domains[index];
        domains[index] = @{
                           _CNBrowseDomainKey_fullname: domain[_CNBrowseDomainKey_fullname],
                           _CNBrowseDomainKey_enabled: @YES
                           };
        self.browseDomainsA = [NSArray arrayWithArray: domains];
        NSIndexPath * indexPath = [NSIndexPath indexPathForRow: index inSection: 0];
        [CATransaction begin];
        [CATransaction setCompletionBlock:^{
            [self.tableView deselectRowAtIndexPath: indexPath animated: YES];
        }];
        [self.tableView reloadRowsAtIndexPaths: @[indexPath] withRowAnimation: UITableViewRowAnimationAutomatic];
        [self.tableView selectRowAtIndexPath: indexPath animated: YES scrollPosition: UITableViewScrollPositionMiddle];
        [CATransaction commit];

    }
}

#pragma mark - CNDomainBrowserViewControllerDelegate

- (void)domainBrowserDomainSelected:(NSString *)domain
{
    [self newPathSelected: domain];
}

- (void)bonjourBrowserDomainUpdate:(NSString *)defaultDomain
{
    (void)defaultDomain;    // Unused
    [self.tableView reloadData];
}

#pragma mark - TableView Delegates

- (CGFloat)tableView:(UITableView *)tableView estimatedHeightForRowAtIndexPath:(NSIndexPath *)indexPath
{
    (void)tableView;        // Unused
    (void)indexPath;        // Unused
    return UITableViewAutomaticDimension;
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    (void)tableView;        // Unused
    NSInteger result = 0;
    
    if (tableView == self.tableView )   result = (self.browseController.flattenedDNSDomains.count > 0) ? 3 : 2;
    else                                result = 1;
    
    return(result );
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    (void)section;        // Unused
    NSInteger result = 0;

    if (tableView == self.tableView )
    {
        if (section == 0 )
        {
            result = self.browseDomainsA.count;
        }
        else
        {
            result = 1;
        }
    }
    
    return(result );
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    UITableViewCell *cell = nil;
    
    if (tableView == self.tableView )
    {
        if (indexPath.section == 0 )
        {
            NSDictionary * record = [self browseDomainForRow: indexPath.row];
            if (record )
            {
                static NSString *MyIdentifier = @"browse_cell_id";
                cell = [tableView dequeueReusableCellWithIdentifier: MyIdentifier];
                if (!cell )
                {
                    cell = [[UITableViewCell alloc] initWithStyle: UITableViewCellStyleSubtitle  reuseIdentifier: MyIdentifier];
                }
                NSArray *  pathArray = DNSDomainToDomainPath(record[_CNBrowseDomainKey_fullname] );
                cell.textLabel.text = pathArray[pathArray.count-1];
                cell.textLabel.textColor = nil;
                cell.textLabel.numberOfLines = 0;
                cell.accessoryType = UITableViewCellAccessoryNone;
                cell.accessoryView = nil;
                cell.imageView.image = [[UIImage imageNamed: @"UIPreferencesBlueCheck.png"
                                                   inBundle: [NSBundle bundleForClass: [self class]]
                              compatibleWithTraitCollection: nil] imageWithRenderingMode: UIImageRenderingModeAlwaysTemplate];
                cell.imageView.hidden = ![record[_CNBrowseDomainKey_enabled] boolValue];
                
                NSArray *   accPathArray = (pathArray.count > 2) ?
                                            [[pathArray reverseObjectEnumerator] allObjects] :
                                            ((pathArray.count > 1) ? @[pathArray[0]] : nil);
                if (accPathArray )
                {
                    CNPathAccessoryView * accView = [[CNPathAccessoryView alloc] initWithFrame: CGRectZero
                                                                                     pathArray: accPathArray];
                    accView.tableView = tableView;
                    accView.cell = cell;
                    [accView sizeToFit];
                    cell.accessoryView = accView;
                }
            }
        }
        else
        {
            static NSString *MyIdentifier = @"button_cell_id";
            cell = [tableView dequeueReusableCellWithIdentifier: MyIdentifier];
            if (!cell )
            {
                cell = [[UITableViewCell alloc] initWithStyle: UITableViewCellStyleDefault  reuseIdentifier: MyIdentifier];
            }
            
           if (self.browseController.flattenedDNSDomains.count > 0 && indexPath.section == 1 )
           {
                cell.textLabel.textColor = self.view.tintColor;
                cell.textLabel.text = LocalizedStringFromMyBundle(@"_dnsBrowser.domains.selectdomain", nil );
                cell.accessoryType = UITableViewCellAccessoryNone;
                cell.tag = kTag_AddBrowse;
           }
           else
           {
               cell.textLabel.textColor = self.view.tintColor;
               cell.textLabel.text = LocalizedStringFromMyBundle(@"_dnsBrowser.domains.selectmanual", nil );
               cell.accessoryType = UITableViewCellAccessoryNone;
               cell.tag = kTag_AddManual;
           }
        }
    }
    
    return(cell );
}

- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath
{
    BOOL result = NO;
    
    if (tableView == self.tableView && indexPath.section == 0 ) result = YES;
    
    return(result );
}

- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath
{
    if (editingStyle == UITableViewCellEditingStyleDelete )
    {
        NSMutableArray * domains = [NSMutableArray arrayWithArray: self.browseDomainsA];
        [domains removeObjectAtIndex: indexPath.row];
        self.browseDomainsA = [NSArray arrayWithArray: domains];
        [tableView deleteRowsAtIndexPaths: @[indexPath] withRowAnimation: UITableViewRowAnimationAutomatic];
    }
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    UITableViewCell * cell = [tableView cellForRowAtIndexPath: indexPath];
    if (tableView == self.tableView )
    {
        if (indexPath.section == 0 )
        {   //  Toggle check and enable bit
            NSMutableArray * domains = [NSMutableArray arrayWithArray: self.browseDomainsA];
            NSMutableDictionary *  selection = [NSMutableDictionary dictionaryWithDictionary: domains[indexPath.row]];
            selection[_CNBrowseDomainKey_enabled] = [NSNumber numberWithBool: ![selection[_CNBrowseDomainKey_enabled] boolValue]];
            domains[indexPath.row] = selection;
            self.browseDomainsA = [NSArray arrayWithArray: domains];
            [tableView reloadRowsAtIndexPaths: @[indexPath] withRowAnimation: UITableViewRowAnimationAutomatic];
        }
        else
        {
            if (cell.tag == kTag_AddBrowse )
            {
                CNDomainBrowserViewController *c = _browseController;
                c.title = LocalizedStringFromMyBundle(@"_bonjour.browse.name", nil);
                c.clearsSelectionOnViewWillAppear = NO;
                c.modalPresentationStyle = UIModalPresentationFormSheet;
                UINavigationController * nv = [[UINavigationController alloc] initWithRootViewController: c];
                [self presentViewController: nv animated: YES completion: nil];
            }
            else if (cell.tag == kTag_AddManual )
            {
                CNManualDomainViewController * c = [[CNManualDomainViewController alloc] initWithStyle: UITableViewStyleGrouped];
                c.delegate = self;
                c.modalPresentationStyle = UIModalPresentationFormSheet;
                UINavigationController * nv = [[UINavigationController alloc] initWithRootViewController: c];
                [self presentViewController: nv animated: YES completion: nil];
            }
        }
    }
    [tableView deselectRowAtIndexPath: indexPath animated: YES];
}

#pragma mark - Commands

- (void)newPathSelected:(NSString *)fullPath
{
    [CATransaction begin];
    [CATransaction setCompletionBlock:^{
        [self addBrowseDomain: fullPath];
    }];
    [self.navigationController popToViewController: self animated: YES];
    [CATransaction commit];
 }

@end
