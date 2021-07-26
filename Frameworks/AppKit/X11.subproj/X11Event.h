#import <AppKit/CGEvent.h>
#import <X11/Xlib.h>

@interface X11Event : CGEvent {
    XEvent _event;
}

- initWithXEvent:(XEvent)event;

@end
