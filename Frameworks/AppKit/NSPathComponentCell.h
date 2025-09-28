#import <AppKit/NSTextFieldCell.h>

@interface NSPathComponentCell : NSTextFieldCell {
    NSURL *_URL;
}

- (NSURL *)URL;
- (void)setURL:(NSURL *)value;

@end
