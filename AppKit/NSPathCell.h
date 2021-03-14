#import <AppKit/NSActionCell.h>
#import <AppKit/NSSavePanel.h>

@class NSPathComponentCell, NSURL, NSColor;

enum {
    NSPathStyleStandard,
    NSPathStyleNavigationBar,
    NSPathStylePopUp,
};
typedef NSInteger NSPathStyle;

@interface NSPathCell : NSActionCell /* < NSOpenSavePanelDelegate > */ {
    NSURL *_URL;
    NSPathStyle _pathStyle;
    SEL _doubleAction;
    NSArray *_pathComponentCells;
    NSPathComponentCell *_clickPathComponentCell;
    id _delegate;
    NSColor *_backgroundColor;
    NSArray *_allowedTypes;
    NSAttributedString *_placeholder;
}

- (void)mouseEntered:(NSEvent *)event withFrame:(NSRect)frame inView:(NSView *)view;
- (void)mouseExited:(NSEvent *)event withFrame:(NSRect)frame inView:(NSView *)view;

- (NSArray *)allowedTypes;
- (void)setAllowedTypes:(NSArray *)allowedTypes;

- (NSPathStyle)pathStyle;
- (void)setPathStyle:(NSPathStyle)style;

- (id)objectValue;
- (void)setObjectValue:(id<NSCopying>)obj;

- (NSAttributedString *)placeholderAttributedString;
- (void)setPlaceholderAttributedString:(NSAttributedString *)string;
- (NSString *)placeholderString;
- (void)setPlaceholderString:(NSString *)string;
- (NSColor *)backgroundColor;
- (void)setBackgroundColor:(NSColor *)color;

+ (Class)pathComponentCellClass;
- (NSRect)rectOfPathComponentCell:(NSPathComponentCell *)cell withFrame:(NSRect)frame inView:(NSView *)view;
- (NSPathComponentCell *)pathComponentCellAtPoint:(NSPoint)point withFrame:(NSRect)frame inView:(NSView *)view;
- (NSPathComponentCell *)clickedPathComponentCell;
- (NSArray *)pathComponentCells;
- (void)setPathComponentCells:(NSArray *)cells;

- (SEL)doubleAction;
- (void)setDoubleAction:(SEL)action;

- (NSURL *)URL;
- (void)setURL:(NSURL *)url;

- (id)delegate;
- (void)setDelegate:(id)delegate;

@end
