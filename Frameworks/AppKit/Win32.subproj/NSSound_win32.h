#import <AppKit/NSSound.h>

@interface NSSound_win32 : NSSound {
    NSString *_soundFilePath;
    BOOL _paused;
    unsigned int _handle;
}

@end
