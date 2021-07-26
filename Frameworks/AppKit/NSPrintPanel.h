#import <Foundation/NSObject.h>
#import <AppKit/AppKitExport.h>

@class NSMutableDictionary;

enum {

    NSPrintPanelShowsCopies = 1 << 0,
    NSPrintPanelShowsPageRange = 1 << 1,
    NSPrintPanelShowsPaperSize = 1 << 2,
    NSPrintPanelShowsOrientation = 1 << 3,
    NSPrintPanelShowsScaling = 1 << 4,
    NSPrintPanelShowsPrintSelection = 1 << 5,
    NSPrintPanelShowsPageSetupAccessory = 1 << 8,
    NSPrintPanelShowsPreview = 1 << 17
};

typedef NSInteger NSPrintPanelOptions;

@interface NSPrintPanel : NSObject {
    NSMutableDictionary *_attributes;
    NSInteger _options;
}

+ (NSPrintPanel *)printPanel;

- (void)setOptions:(NSPrintPanelOptions)options;
- (NSPrintPanelOptions)options;

- (int)runModal;

- (void)updateFromPrintInfo;
- (void)finalWritePrintInfo;

@end

APPKIT_EXPORT NSString *const NSPrintPanelAccessorySummaryItemNameKey;
APPKIT_EXPORT NSString *const NSPrintPanelAccessorySummaryItemDescriptionKey;
