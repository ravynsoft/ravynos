//
//  NSPrintProgressPanelController.m
//  AppKit
//
//  Created by Robert Grant on 7/25/13.
//
//

#import "NSPrintProgressPanelController.h"

@interface NSPrintProgressPanelController ()

@end

@implementation NSPrintProgressPanelController

+ (NSPrintProgressPanelController *)printProgressPanelController
{
    return [[[NSPrintProgressPanelController alloc] initWithWindowNibName: @"NSPrintProgressPanel"] autorelease];
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    
    // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
}

- (void)setTitle:(NSString *)title
{
    [[self window] setTitle: title];
}

- (void)setMaxPages:(NSInteger)maxPages
{
    [progressIndicator setDoubleValue: 0];
    [progressIndicator setMaxValue: maxPages];
}

- (void)setCurrentPage:(NSInteger)currentPage
{
    [progressIndicator setDoubleValue: currentPage];
    [progressIndicator displayIfNeeded];
}

- (void)showPanel
{
    [[self window] center];
    [self showWindow: nil];
}


- (void)hidePanel
{
    [self close];
}

@end
