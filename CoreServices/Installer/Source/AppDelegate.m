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

#import "AppDelegate.h"
#import "GSGeomDisk.h"

#include <fcntl.h>
#include <unistd.h>

int logfd = -1;

void appendLog(NSData *data) {
    if(logfd < 0)
        logfd = open("/tmp/Installer_Log.txt", O_CREAT|O_RDWR, 0644);
    write(logfd, [data bytes], [data length]);
}

void closeLog() {
    if(logfd >= 0)
        close(logfd);
}

@interface AppDelegate ()
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    myBundle = [NSBundle mainBundle];
    _updateTimer = nil;
    
    NSString *verLabel = [[_versionLabel stringValue] stringByReplacingOccurrencesOfString:@"X.X.X" withString:[NSString stringWithUTF8String:RAVYNOS_VERSION]];
    [_versionLabel setStringValue:verLabel];

    [_scrollView setAutoresizesSubviews:YES];
    NSDictionary *attr = [NSDictionary new];
    NSData *rtf = [NSData dataWithContentsOfFile:[myBundle pathForResource:@"terms" ofType:@"rtf"]];
    NSAttributedString *text = [[NSAttributedString alloc] initWithRTF:rtf documentAttributes:&attr];

    NSTextView *content = [[_scrollView contentView] documentView];
    [[content textStorage] setAttributedString:text];

    rtf = [NSData dataWithContentsOfFile:[myBundle pathForResource:@"header" ofType:@"rtf"]];
    text = [[NSAttributedString alloc] initWithRTF:rtf documentAttributes:&attr];
    content = [[_instructionsView contentView] documentView];
    [[content textStorage] setAttributedString:text];
}

- (IBAction)proceedToDiskList:(id)sender {
    NSFont *font = [NSFont systemFontOfSize:12.0];
    NSMutableDictionary *attr = [NSMutableDictionary
        dictionaryWithObjects:@[font, [NSColor blackColor]]
        forKeys:@[NSFontAttributeName, NSForegroundColorAttributeName]];

    NSTextStorage *textStorage = [[[_instructionsView contentView]
        documentView] textStorage];

    [textStorage setAttributedString:[[NSAttributedString alloc]
        initWithString:@"Select the disk where ravynOS should be installed.\n\n" // FIXME: localize
        attributes:attr]];

    [attr setObject:[NSColor redColor] forKey:NSForegroundColorAttributeName];
    font = [[NSFontManager sharedFontManager] convertFont:font
        toHaveTrait:NSBoldFontMask];
    [attr setObject:font forKey:NSFontAttributeName];

    [textStorage appendAttributedString:[[NSAttributedString alloc]
        initWithString:@"WARNING! Everything on the selected disk will be erased." // FIXME: localize
        attributes:attr]];

    if(discoverGEOMs(YES) == NO || [disks count] < 1) {
        NSLog(@"error discovering devices!");
        [textStorage setAttributedString:[[NSAttributedString alloc]
            initWithString:@"No suitable disk was found on which to install ravynOS.\n\n"
            "Ensure you have a compatible device of at least 10 GB installed and try again." // FIXME: localize
            attributes:attr]];

        [_BackButton setEnabled:NO];
        [_NextButton setEnabled:NO];
        [[_scrollView contentView] setDocumentView:nil];
        return;
    }

    NSTableView *table = [[NSTableView alloc]
        initWithFrame:[[_scrollView contentView] frame]];
    [table setAllowsColumnReordering:NO];

    NSArray *columnIDs = @[@"device",@"size",@"descr"];
    NSArray *headerLabels = @[@"Device",@"Size",@"Description"]; // FIXME: localize
    for(int x = 0; x < 3; ++x) {
        NSTableColumn *col = [NSTableColumn new];
        [col setIdentifier:[columnIDs objectAtIndex:x]];
        [col setMinWidth:10.0];
        [col setMaxWidth:MAXFLOAT];
        [col setWidth:100.0];
        [col setHeaderCell:[[NSTableHeaderCell alloc]
            initTextCell:[headerLabels objectAtIndex:x]]];
        [col setDataCell:[[NSTextFieldCell alloc] initTextCell:@""]];
        [table addTableColumn:col];
    }
    [table setDataSource:[GSGeomDisk new]];

    NSClipView *clip = [NSClipView new];
    [clip setDocumentView:table];
    [table sizeLastColumnToFit];

    [_scrollView setContentView:clip];
    [_scrollView setAutohidesScrollers:YES];

#ifdef __RAVYNOS__
    [_NextButton setEnabled:NO];
#endif
    
    [[NSNotificationCenter defaultCenter] addObserver:self
        selector:@selector(deviceSelected:)
        name:NSTableViewSelectionDidChangeNotification
        object:table];
    [_BackButton setEnabled:NO];
    [_NextButton setAction:@selector(proceedToUserInfo:)];
}

- (IBAction)proceedToUserInfo:(id)sender {
    NSFont *font = [NSFont systemFontOfSize:12.0];
    NSMutableDictionary *attr = [NSMutableDictionary
        dictionaryWithObjects:@[font, [NSColor blackColor]]
        forKeys:@[NSFontAttributeName, NSForegroundColorAttributeName]];

    NSTextStorage *textStorage = [[[_instructionsView contentView]
        documentView] textStorage];
    [textStorage setAttributedString:[[NSAttributedString alloc]
        initWithString:@"\nEnter the information below to finish setting "
        "up your new system.\nAll fields are required.\n\n" // FIXME: localize
        attributes:attr]];

    if([[[_timeZones menu] itemArray] count] < 2) {
        NSArray *zones = @[@"Pacific", @"America", @"Atlantic", @"UTC", @"Europe",
            @"Indian", @"Asia", @"Australia"];
        NSFileManager *fm = [NSFileManager defaultManager];
        NSEnumerator *dirs = [zones objectEnumerator];
        NSString *dir;
        BOOL isDir = NO;
        while(dir = [dirs nextObject]) {
            NSString *zonepath = [NSString stringWithFormat:@"/usr/share/zoneinfo/%@", dir];
            if([fm fileExistsAtPath:zonepath isDirectory:&isDir]) {
                if(isDir) {
                    NSArray *entries = [fm contentsOfDirectoryAtPath:zonepath error:NULL];
                    NSEnumerator *entryiter = [[entries
                        sortedArrayUsingSelector:@selector(compare:)] objectEnumerator];
                    NSString *zone;
                    while(zone = [entryiter nextObject]) {
                        NSString *title = [NSString stringWithFormat:@"%@/%@",dir,zone];
                        [_timeZones addItemWithTitle:title];
                    }
                } else {
                    [_timeZones addItemWithTitle:dir];
                }
            }
        }
    }
    
    if(selectedTimeZone)
        [_timeZones selectItemWithTitle:selectedTimeZone];
    else
        [_timeZones selectItemWithTitle:@"UTC"];
    [_timeZones synchronizeTitleAndSelectedItem];

#ifdef __RAVYNOS__
    [[_scrollView contentView] release];
#endif

    NSClipView *clip = [NSClipView new];
    [clip setDocumentView:_userInfoView];
    [_scrollView setContentView:clip];
    [_scrollView setAutohidesScrollers:YES];
    [[_scrollView documentView] scrollPoint:
        NSMakePoint(0,[_userInfoView bounds].size.height)];

    [_BackButton setAction:@selector(proceedToDiskList:)];
    [_BackButton setEnabled:YES];
    [_NextButton setAction:@selector(validateUserInfo:)];

    [_fullName becomeFirstResponder];
}

- (IBAction)validateUserInfo:(id)sender {
    NSFont *font = [NSFont systemFontOfSize:12.0];
    NSMutableDictionary *attr = [NSMutableDictionary
        dictionaryWithObjects:@[font, [NSColor blackColor]]
        forKeys:@[NSFontAttributeName, NSForegroundColorAttributeName]];

    NSTextStorage *textStorage = [[[_instructionsView contentView]
        documentView] textStorage];
    [textStorage setAttributedString:[[NSAttributedString alloc]
        initWithString:@"Verify that the information below is correct. "
        "When you are finished, click the Next button to start the install process.\n\n" // FIXME: localize
        attributes:attr]];
    font = [[NSFontManager sharedFontManager] convertFont:font
        toHaveTrait:NSBoldFontMask];
    [attr setObject:font forKey:NSFontAttributeName];
    [textStorage appendAttributedString:[[NSAttributedString alloc]
        initWithString:@"WARNING: All contents of the selected disk will be ERASED if you proceed!" // FIXME: localize
        attributes:attr]];

    [self fullNameEditingDidFinish:_fullName];
    [self userNameEditingDidFinish:_userName];
    [self passwordEditingDidFinish:_password];
    [self hostNameEditingDidFinish:_hostName];

    if(selectedTimeZone == nil)
        selectedTimeZone = @"UTC";

    [_confirmFullName setStringValue:UserInfoFullName];
    [_confirmUserName setStringValue:UserInfoUserName];
    [_confirmHostName setStringValue:UserInfoHostName];
    [_confirmPassword setStringValue:UserInfoPassword];
    [_confirmTimeZone setStringValue:selectedTimeZone];
    GSGeomDisk *disk = [disks objectAtIndex:selectedDisk];
    [_confirmDisk setStringValue:[NSString stringWithFormat:@"%@ (%@, %@)", [disk name],
        formatMediaSize([disk mediaSize]), [disk mediaDescription]]];

#ifdef __RAVYNOS__
    [[_scrollView contentView] release];
#endif
    [_scrollView setAutohidesScrollers:YES];
    NSClipView *clip = [NSClipView new];
    [clip setDocumentView:_infoConfirmationView];
    [_scrollView setContentView:clip];
    [[_scrollView documentView] scrollPoint:
        NSMakePoint(0,[_userInfoView bounds].size.height)];

    [_BackButton setAction:@selector(proceedToUserInfo:)];
    [_NextButton setAction:@selector(proceedToInstall:)];
}

- (IBAction)proceedToFinalize:(id)sender {
    [self appendStatus:@"Configuring installed system\n" bold:YES];
    [[disks objectAtIndex:selectedDisk] finalizeInstallation];

    [spinner stopAnimation:nil];
    closeLog();
    if(_updateTimer != nil) {
        [_updateTimer invalidate];
        [_updateTimer release];
        _updateTimer=nil;
    }

    [_NextButton setAction:@selector(terminate:)];
    [_NextButton setTarget:NSApp];
    [_NextButton setTitle:@"Quit"];
    [_BackButton setHidden:YES];
    [_CancelButton setHidden:YES];

    NSFont *font = [NSFont systemFontOfSize:12.0];
    NSMutableDictionary *attr = [NSMutableDictionary
        dictionaryWithObjects:@[font, [NSColor blackColor]]
        forKeys:@[NSFontAttributeName, NSForegroundColorAttributeName]];

    NSAttributedString *rest = [[NSAttributedString alloc]
        initWithString:@" Review the log below for any errors."
        "You can Quit this application when you are finished.\n\nRemove the installation"
        " media and restart to try your new system." // FIXME: localize
        attributes:attr];

    NSTextStorage *textStorage = [[_scrollView documentView] textStorage];
    NSString *s = [[[NSString alloc] initWithContentsOfFile:@"/tmp/Installer_Log.txt"] retain];

    [textStorage beginEditing];
    [textStorage setAttributedString:
        [[[NSAttributedString alloc] initWithString:s attributes:attr] retain]];
    [textStorage endEditing];
    [[_scrollView documentView] scrollToBeginningOfDocument:nil];

    font = [[NSFontManager sharedFontManager] convertFont:font
        toHaveTrait:NSBoldFontMask];
    [attr setObject:font forKey:NSFontAttributeName];

    textStorage = [[[_instructionsView contentView]
        documentView] textStorage];
    [textStorage beginEditing];
    [textStorage setAttributedString:[[NSAttributedString alloc]
        initWithString:@"Installation has finished!" // FIXME: localize
        attributes:attr]];
    [textStorage appendAttributedString:rest];
    [textStorage endEditing];

    [_scrollView setNeedsDisplay:YES];
    [[_scrollView contentView] setNeedsDisplay:YES];
    [[_scrollView documentView] setNeedsDisplay:YES];
}

- (void)deviceSelected:(NSNotification *)aNotification {
    [_NextButton setEnabled:YES];
    selectedDisk = [[[aNotification object] selectedRowIndexes] firstIndex];
}

- (IBAction)timeZoneSelected:(id)sender {
    selectedTimeZone = [sender titleOfSelectedItem];
}

- (IBAction)hostNameEditingDidFinish:(id)sender {
    userDidEditHostName = YES;
    UserInfoHostName = [sender stringValue];
}

- (IBAction)passwordEditingDidFinish:(id)sender {
    UserInfoPassword = [sender stringValue];
}

- (IBAction)userNameEditingDidFinish:(id)sender {
    userDidEditUserName = YES;
    UserInfoUserName = [sender stringValue];
}

- (IBAction)fullNameEditingDidFinish:(id)sender {
    NSMutableArray *name = [NSMutableArray arrayWithCapacity:5];
    NSArray *input = [[sender stringValue] componentsSeparatedByString:@" "];

    for(int x = 0; x < [input count]; ++x)
        if(! [[input objectAtIndex:x] isEqualToString:@""])
            [name addObject:[input objectAtIndex:x]];

    NSString *user;
    NSString *host;

    switch([name count]) {
    case 0:
        user = @"user";
        host = @"ravynOS";
        break;
    case 1:
        user = [name firstObject];
        host = [user stringByAppendingString:@"-ravynOS"];
        break;
    default:
        user = [NSString stringWithFormat:@"%@%@",
            [[name firstObject] substringToIndex:1],
            [name lastObject]];
        host = [[name componentsJoinedByString:@"-"]
                stringByAppendingString:@"-ravynOS"];
    }

    if(!userDidEditUserName) {
        [_userName setStringValue:[[user lowercaseString]
            substringWithRange:NSMakeRange(0,MIN(8, [user length]))]];
        UserInfoUserName = [_userName stringValue];
    }

    if(!userDidEditHostName) {
        [_hostName setStringValue:host];
        UserInfoHostName = host;
    }

    UserInfoFullName = [sender stringValue];
}

- (IBAction)proceedToInstall:(id)sender {
    [_NextButton setEnabled:NO];
    [_BackButton setEnabled:NO];
    [_CancelButton setEnabled:NO];

#ifdef __RAVYNOS__
    [[[_scrollView contentView] documentView] release];
#endif

    NSSize contentSize = [_scrollView contentSize];
    NSTextView *v = [[NSTextView alloc] initWithFrame:
        NSMakeRect(0, 0, contentSize.width, contentSize.height)];
    [v setMinSize:NSMakeSize(0.0, contentSize.height)];
    [v setMaxSize:NSMakeSize(FLT_MAX, FLT_MAX)];
    [v setVerticallyResizable:YES];
    [v setHorizontallyResizable:NO];
    [v setEditable:NO];
    [v setAutoresizingMask:NSViewWidthSizable];

    [[v textContainer] setContainerSize:
        NSMakeSize(contentSize.width, FLT_MAX)];
    [[v textContainer] setWidthTracksTextView:YES];

    [_scrollView setDocumentView:v];
    [_scrollView setAutohidesScrollers:NO];
    [_NextButton setAction:@selector(proceedToFinalize:)];

    [[_scrollView window] makeFirstResponder:v];

    GSGeomDisk *disk = [disks objectAtIndex:selectedDisk];
    [disk setDelegate:self];

    _updateTimer = [[NSTimer timerWithTimeInterval:0.05
        target:self selector:@selector(redisplay:) userInfo:nil
        repeats:YES] retain];
    [[NSRunLoop currentRunLoop] addTimer:_updateTimer forMode:NSDefaultRunLoopMode];

    spinner = [[NSProgressIndicator alloc] initWithFrame:
    NSMakeRect(contentSize.width / 2 - 12, contentSize.height / 2 - 12, 24, 24)];
    [spinner setControlSize:NSRegularControlSize];
    [spinner setIndeterminate:YES];
    [spinner setAnimationDelay:0.025];
    [spinner setUsesThreadedAnimation:NO];
    [spinner setStyle:NSProgressIndicatorSpinningStyle];
    [v addSubview:spinner];
    [spinner startAnimation:nil];

    [self appendStatus:[NSString
        stringWithFormat:@"Clearing disk %@\n", [disk name]] bold:YES];
    [disk createGPT];
    [self appendStatus:@"Creating partitions\n" bold:YES];
    [disk createPartitions];
    [self appendStatus:@"Creating volumes\n" bold:YES];
    [disk createPools];
    [self appendStatus:@"Installing EFI loader\n" bold:YES];
    [disk initializeEFI];
    [self appendStatus:@"Installing files\n" bold:YES];

    [disk performSelectorInBackground:@selector(copyFilesystem) withObject:nil];
}

- (void)redisplay:(id)sender {
    NSTextView *tv = [_scrollView documentView];
    [tv setNeedsDisplay:YES];
}

- (void)proceed {
    [_NextButton setEnabled:YES];
    [self performSelectorOnMainThread:@selector(proceedToFinalize:) withObject:nil waitUntilDone:NO];
}

- (void)appendStatus:(NSString *)text bold:(BOOL)bold {
    appendLog([text dataUsingEncoding:NSUTF8StringEncoding]);

    NSFont *font = [NSFont systemFontOfSize:12.0];
    if(bold)
        font = [[NSFontManager sharedFontManager] convertFont:font toHaveTrait:NSBoldFontMask];
    NSDictionary *attr = [NSDictionary
        dictionaryWithObjects:@[font, [NSColor blackColor]]
        forKeys:@[NSFontAttributeName, NSForegroundColorAttributeName]];
    NSAttributedString *string = [[NSAttributedString alloc] initWithString:text
        attributes:attr];

    NSTextStorage *ts = [[_scrollView documentView] textStorage];
    [ts beginEditing];
    [ts appendAttributedString:string];
    [ts endEditing];
    [[_scrollView documentView] scrollToEndOfDocument:nil];
}

- (void)appendStatus:(NSString *)text {
    [self appendStatus:text bold:NO];
}

- (NSString *)userInfoHostName {
    return UserInfoHostName;
}

- (NSString *)userInfoUserName {
    return UserInfoUserName;
}

- (NSString *)userInfoFullName {
    return UserInfoFullName;
}

- (NSString *)userInfoPassword {
    return UserInfoPassword;
}

- (NSString *)timeZone {
    return selectedTimeZone;
}

- (void)fileHandleReadDidComplete:(NSNotification *)aNotification {
    NSData *data = [[aNotification userInfo] objectForKey:NSFileHandleNotificationDataItem];
    if([data length] == 0)
        return;
    appendLog(data);
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
    //NSLog(@"will terminate");
}


- (BOOL)applicationSupportsSecureRestorableState:(NSApplication *)app {
    return YES;
}


@end
