/*
 * Copyright (C) 2021 Zoe Knox <zoe@pixin.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#import <Cocoa/Cocoa.h>

@interface AppDelegate : NSObject <NSApplicationDelegate> {
    int selectedDisk; // table row = disks array index
}

@property (strong) IBOutlet NSWindow *mainWindow;
@property (strong) IBOutlet NSScrollView *scrollView;
@property (strong) IBOutlet NSScrollView *instructionsView;
@property (strong) IBOutlet NSButton *ProceedButton;
@property (strong) IBOutlet NSButton *CancelButton;
@property (strong) IBOutlet NSTextField *versionLabel;
@property (strong) IBOutlet NSClipView *userInfoView;
@property (strong) IBOutlet NSTextField *fullName;
@property (strong) IBOutlet NSTextField *userName;
@property (strong) IBOutlet NSSecureTextField *password;
@property (strong) IBOutlet NSTextField *hostName;
@property (strong) IBOutlet NSPopUpButton *timeZones;

- (IBAction)proceedToDiskList:(id)sender;
- (IBAction)proceedToPartition:(id)sender;

- (void)deviceSelected:(NSNotification *)aNotification;

@end

