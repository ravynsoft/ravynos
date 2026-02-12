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

#import "CNDomainBrowserViewController.h"
#import "_CNDomainBrowser.h"
#import "CNDomainBrowserPathUtils.h"
#import <AssertMacros.h>

#define LocalizedStringFromTableInMyBundle(key, table, comment)     \
        NSLocalizedStringFromTableInBundle(key, table, [NSBundle bundleForClass: [self class]], comment)


@interface CNTableViewController()
@property (copy)  NSArray *   pathArray;
@property (assign)  NSInteger   selectionIndex;
@end

@implementation CNTableViewController

- (instancetype)initWithStyle:(UITableViewStyle)style
{
   if (self = [super initWithStyle: style])
   {
       _selectionIndex = NSNotFound;
       _pathArray = [NSArray array];
   }
    return self;
}

@end

@interface CNDomainBrowserViewController ()

@property _CNDomainBrowser *               bonjour;

@property (strong) NSMutableDictionary *    instanceInfoStrings;
@property (strong) NSMutableDictionary *    instanceStatusViews;

@end

@implementation CNDomainBrowserViewController

- (instancetype)initWithStyle:(UITableViewStyle)style
{
    if (self = [super initWithStyle: style])
    {
        [self commonInit];
    }
    return(self);
}

- (instancetype)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    if (self = [super initWithNibName: nibNameOrNil bundle: nibBundleOrNil])
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

- (void)commonInit
{
    self.bonjour = [[_CNDomainBrowser alloc] initWithDelegate: (id<_CNDomainBrowserDelegate>)self];
    self.bonjour.browseRegistration = _browseRegistration;
    self.bonjour.ignoreLocal = _ignoreLocal;
    self.instanceInfoStrings = [NSMutableDictionary dictionary];
    self.instanceStatusViews = [NSMutableDictionary dictionary];
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear: animated];
    NSArray * pathArray = DNSDomainToDomainPath(_selectedDNSDomain);
    if ((self.isMovingToParentViewController || self.isBeingPresented) && (pathArray.count > 1))
    {
        [self updateUIToDomainPathArray: pathArray];
    }
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.tableView.allowsMultipleSelectionDuringEditing = NO;
    self.tableView.dataSource = (id<UITableViewDataSource>)self;
    self.tableView.delegate = (id<UITableViewDelegate>)self;
    
    self.navigationItem.leftBarButtonItem = [[UIBarButtonItem alloc] initWithTitle: LocalizedStringFromTableInMyBundle(@"_dnsBrowser.browsedomains.cancel", @"DomainBrowser", nil)
                                                                             style: UIBarButtonItemStylePlain
                                                                            target: self
                                                                            action: @selector(cancelAction:)];

}

- (void)updateUIToDomainPathArray:(NSArray *)newPathArray
{
    if (newPathArray.count > 1)
    {
        CNTableViewController * controller = nil;
        NSMutableArray * newSubPathArray = [NSMutableArray array];
        for (NSString * nextPathComponent in newPathArray)
        {
            BOOL animate = NO;//(newPathArray.count == ++count);
            controller = [self pushNewBrowseController: newSubPathArray.lastObject animated: animate];
            controller.pathArray = newSubPathArray;
            [newSubPathArray addObject: nextPathComponent];
        }
    }
}

- (IBAction)cancelAction:(id)sender
{
    (void)sender;   // Unused
    [self.navigationController dismissViewControllerAnimated: YES completion: nil];
}

- (CNTableViewController *)pushNewBrowseController:(NSString *)title animated:(BOOL)animated
{
    CNTableViewController *tvc = [[CNTableViewController alloc] initWithStyle: self.tableView.style];
    tvc.title = title;
    tvc.clearsSelectionOnViewWillAppear = NO;
    tvc.tableView.dataSource = (id<UITableViewDataSource>)self;
    tvc.tableView.delegate = (id<UITableViewDelegate>)self;
    [self.navigationController pushViewController: tvc animated: animated];
    return(tvc);
}

- (CNTableViewController *)controllerForTableView:(UITableView *)tableView
{
    CNTableViewController * result = nil;
    
    for (CNTableViewController * next in self.navigationController.viewControllers)
    {
        if ([next isKindOfClass: [CNTableViewController class]] && next.tableView == tableView)
        {
            result = next;
            break;
        }
    }
    
    return(result);
}

- (NSArray *)selectedPathArrayForTableView:(UITableView *)tableView includeSelectedRow:(BOOL)includeSelection
{
    NSMutableArray * pathArray = [NSMutableArray array];
    CNTableViewController * controller = [self controllerForTableView: tableView];
    
    [pathArray addObjectsFromArray: controller.pathArray];
    
    if (includeSelection && controller.selectionIndex != NSNotFound)
    {
        NSArray * rowArray = [[self.bonjour subDomainsAtDomainPath: pathArray] sortedArrayUsingComparator: ^(id obj1, id obj2) {
            return (NSComparisonResult)[ obj1[_CNSubDomainKey_subPath] compare: obj2[_CNSubDomainKey_subPath]];
        }];
        if (controller.selectionIndex < (NSInteger)rowArray.count)
        {
            [pathArray addObject: rowArray[controller.selectionIndex][_CNSubDomainKey_subPath]];
        }
    }
    
    return(pathArray);
}

- (void)plusButtonPressed:(UIControl *)button withEvent:(UIEvent *)event
{
    UITableView * tableView = ((UITableViewController *)self.navigationController.topViewController).tableView;
    NSIndexPath * indexPath = [tableView indexPathForRowAtPoint: [[[event touchesForView: button] anyObject] locationInView: tableView]];
    if (indexPath != nil && self.delegate)
    {
        [self controllerForTableView: tableView].selectionIndex = indexPath.row;
        NSArray * pathArray = [self selectedPathArrayForTableView: ((CNTableViewController *)self.navigationController.topViewController).tableView includeSelectedRow: YES];
        _selectedDNSDomain = DomainPathToDNSDomain(pathArray);
        [self.navigationController dismissViewControllerAnimated: YES completion: ^{
            if ([self.delegate respondsToSelector: @selector(domainBrowserDomainSelected:)])
            {
                [self.delegate domainBrowserDomainSelected: _selectedDNSDomain];
            }
        }];
    }
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
}

- (BOOL)isBrowsing
{
    return(self.bonjour.isBrowsing);
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
    return(1);
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    (void)section;        // Unused
    return([self.bonjour subDomainsAtDomainPath: [self selectedPathArrayForTableView: tableView includeSelectedRow: NO]].count);
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    UITableViewCell *cell = nil;
    
    static NSString *MyIdentifier = @"browse_cell_id";
    cell = [tableView dequeueReusableCellWithIdentifier: MyIdentifier];
    if (!cell)
    {
        cell = [[UITableViewCell alloc] initWithStyle: UITableViewCellStyleDefault  reuseIdentifier: MyIdentifier];
    }
    
    //	Get the name
    NSMutableArray * pathArray = [NSMutableArray arrayWithArray: [self selectedPathArrayForTableView: tableView includeSelectedRow: NO]];
    NSArray * rowArray = [[self.bonjour subDomainsAtDomainPath: pathArray] sortedArrayUsingComparator: ^(id obj1, id obj2) {
        return (NSComparisonResult)[ obj1[_CNSubDomainKey_subPath] compare: obj2[_CNSubDomainKey_subPath]];
    }];
    if (indexPath.row < (NSInteger)rowArray.count)
    {
        NSDictionary *	item = [rowArray objectAtIndex: indexPath.row];
        NSString *val = item[_CNSubDomainKey_subPath];
        cell.textLabel.text = val;
        
        //  Set selection
        BOOL selected = ([self controllerForTableView: tableView].selectionIndex == indexPath.row);
        if (selected) [tableView selectRowAtIndexPath: indexPath animated: NO scrollPosition: UITableViewScrollPositionNone];
        
        //	Make Default domain bold
        if ([item[_CNSubDomainKey_defaultFlag] boolValue])  cell.textLabel.font = [UIFont boldSystemFontOfSize: [UIFont labelFontSize]];
        else                                                cell.textLabel.font = nil;
        
        //	See if it's a leaf
        [pathArray addObject: val];
        cell.accessoryType = [self.bonjour subDomainsAtDomainPath: pathArray].count ? UITableViewCellAccessoryDisclosureIndicator : UITableViewCellAccessoryNone;
        
        //  Add the "+" button
        UIButton * plus = [UIButton buttonWithType: UIButtonTypeContactAdd];
        [plus addTarget: self action: @selector(plusButtonPressed:withEvent:) forControlEvents: UIControlEventTouchUpInside];
        plus.userInteractionEnabled = YES;
        [cell.contentView addSubview: plus];
        
        plus.translatesAutoresizingMaskIntoConstraints = NO;
        [plus.widthAnchor       constraintEqualToConstant: plus.frame.size.width].active = YES;
        [plus.heightAnchor      constraintEqualToConstant: plus.frame.size.height].active = YES;
        [plus.centerYAnchor     constraintEqualToAnchor: cell.layoutMarginsGuide.centerYAnchor].active = YES;
        [plus.trailingAnchor    constraintEqualToAnchor: cell.layoutMarginsGuide.trailingAnchor constant: -20].active = YES;
    }
    
    return(cell);
}

- (nullable NSIndexPath *)tableView:(UITableView *)tableView willSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    UITableViewCell * cell = [tableView cellForRowAtIndexPath: indexPath];
    return((cell.accessoryType == UITableViewCellAccessoryNone) ? nil : indexPath);
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    UITableViewCell * cell = [tableView cellForRowAtIndexPath: indexPath];
    if (cell.accessoryType != UITableViewCellAccessoryNone)
    {
        //  Push next
        CNTableViewController * controller = [self controllerForTableView: tableView];
        NSArray * lastpathArray = controller.pathArray;
        controller.selectionIndex = indexPath.row;
        NSString * title = cell.textLabel.text;
        controller = [self pushNewBrowseController: title animated: YES];
        controller.pathArray = [lastpathArray arrayByAddingObject: title];
    }
    [tableView deselectRowAtIndexPath: indexPath animated: YES];
}

#pragma mark - _CNDomainBrowser Delegates

- (void)bonjourBrowserDomainUpdate:(NSArray *)defaultDomainPath
{
    _selectedDNSDomain = DomainPathToDNSDomain(defaultDomainPath);
    [((UITableViewController *)self.navigationController.topViewController).tableView reloadData];
    if ([self.delegate respondsToSelector: @selector(bonjourBrowserDomainUpdate:)])
    {
        [self.delegate bonjourBrowserDomainUpdate: _selectedDNSDomain];
    }
}

@end
