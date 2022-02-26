//
//  X11Display.h
//  AppKit
//
//  Created by Johannes Fortmann on 13.10.08.
//  Copyright 2008 -. All rights reserved.
//  Copyright (C) 2022 Zoe Knox. All rights reserved.
//

#import <AppKit/NSDisplay.h>
#import <wayland-client.h>

@interface WLDisplay : NSDisplay {
    struct wl_display *_display;
    int _fileDescriptor;
    NSSelectInputSource *_inputSource;
    NSMutableDictionary *_windowsByID;

    id lastFocusedWindow;
    NSTimeInterval lastClickTimeStamp;
    int clickCount;
}

- (struct wl_display *)display;

- (void)setWindow:(id)window forID:(unsigned long)i;

- (float)doubleClickInterval;
- (int)handleError:(void *)errorEvent;
@end
