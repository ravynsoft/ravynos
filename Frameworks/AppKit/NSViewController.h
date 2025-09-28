#import <AppKit/NSResponder.h>

@class NSView;

@interface NSViewController : NSResponder {
    NSString *_nibName;
    NSBundle *_nibBundle;
    id _representedObject;
    NSString *_title;
    NSView *_view;
}

- initWithNibName:(NSString *)name bundle:(NSBundle *)bundle;

- (NSString *)nibName;
- (NSBundle *)nibBundle;

- (NSView *)view;
- (NSString *)title;
- representedObject;

- (void)setRepresentedObject:object;

- (void)setTitle:(NSString *)value;

- (void)setView:(NSView *)value;

- (void)loadView;

- (void)discardEditing;

- (BOOL)commitEditing;
- (void)commitEditingWithDelegate:delegate didCommitSelector:(SEL)didCommitSelector contextInfo:(void *)contextInfo;

@end
