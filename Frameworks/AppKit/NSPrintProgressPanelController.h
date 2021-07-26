//
//  NSPrintProgressPanelController.h
//  AppKit
//
//  Created by Robert Grant on 7/25/13.
//
//

#import <AppKit/AppKit.h>

@interface NSPrintProgressPanelController : NSWindowController {
    IBOutlet NSProgressIndicator *progressIndicator;
}

+ (NSPrintProgressPanelController *)printProgressPanelController;

- (void)setTitle:(NSString *)title;

- (void)setMaxPages:(NSInteger)maxPages;

- (void)setCurrentPage:(NSInteger)currentPage;

- (void)showPanel;
- (void)hidePanel;

@end
