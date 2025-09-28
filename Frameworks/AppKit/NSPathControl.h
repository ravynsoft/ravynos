#import <AppKit/NSControl.h>
#import <AppKit/NSPathCell.h>
#import <AppKit/NSDragging.h>

@class NSPathComponentCell;
@class NSPathControl;
@class NSOpenPanel;

@protocol NSPathControlDelegate

//@optional

- (BOOL)pathControl:(NSPathControl *)pathControl acceptDrop:(id<NSDraggingInfo>)info;
- (BOOL)pathControl:(NSPathControl *)pathControl shouldDragPathComponentCell:(NSPathComponentCell *)pathComponentCell withPasteboard:(NSPasteboard *)pasteboard;
- (NSDragOperation)pathControl:(NSPathControl *)pathControl validateDrop:(id<NSDraggingInfo>)info;

- (void)pathControl:(NSPathControl *)pathControl willDisplayOpenPanel:(NSOpenPanel *)openPanel;
- (void)pathControl:(NSPathControl *)pathControl willPopUpMenu:(NSMenu *)menu;

@end

@interface NSPathControl : NSControl {
}

- (NSPathStyle)pathStyle;
- (void)setPathStyle:(NSPathStyle)style;

- (NSColor *)backgroundColor;
- (void)setBackgroundColor:(NSColor *)color;

- (NSPathComponentCell *)clickedPathComponentCell;
- (NSArray *)pathComponentCells;
- (void)setPathComponentCells:(NSArray *)cells;

- (SEL)doubleAction;
- (void)setDoubleAction:(SEL)action;

- (NSURL *)URL;
- (void)setURL:(NSURL *)url;

- (id<NSPathControlDelegate>)delegate;
- (void)setDelegate:(id<NSPathControlDelegate>)delegate;

- (void)setDraggingSourceOperationMask:(NSDragOperation)mask forLocal:(BOOL)isLocal;

- (NSMenu *)menu;
- (void)setMenu:(NSMenu *)menu;

@end