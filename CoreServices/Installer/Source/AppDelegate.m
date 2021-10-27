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

@interface AppDelegate ()
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    myBundle = [NSBundle mainBundle];
    
    NSString *verLabel = [[_versionLabel stringValue] stringByReplacingOccurrencesOfString:@"X.X.X" withString:[NSString stringWithUTF8String:AIRYX_VERSION]];
    [_versionLabel setStringValue:verLabel];

    [_scrollView setAutoresizesSubviews:YES];
    NSData *rtf = [NSData dataWithContentsOfFile:[myBundle pathForResource:@"terms" ofType:@"rtf"]];
    NSAttributedString *text = [[NSAttributedString alloc] initWithRTF:rtf documentAttributes:nil];

    NSTextView *content = [[_scrollView contentView] documentView];
    [[content textStorage] setAttributedString:text];

    rtf = [NSData dataWithContentsOfFile:[myBundle pathForResource:@"header" ofType:@"rtf"]];
    text = [[NSAttributedString alloc] initWithRTF:rtf documentAttributes:nil];
    content = [[_instructionsView contentView] documentView];
    [[content textStorage] setAttributedString:text];
}

- (IBAction)proceedToDiskList:(id)sender {
    NSFont *font = [NSFont systemFontOfSize:12.0];
    font = [[NSFontManager sharedFontManager] convertFont:font
        toNotHaveTrait:NSItalicFontMask];
    NSMutableDictionary *attr = [NSMutableDictionary
        dictionaryWithObjects:@[font, [NSColor blackColor]]
        forKeys:@[NSFontAttributeName, NSForegroundColorAttributeName]];

    NSTextStorage *textStorage = [[[_instructionsView contentView]
        documentView] textStorage];

    [textStorage setAttributedString:[[NSAttributedString alloc]
        initWithString:@"Select the disk where airyxOS should be installed.\n\n" // FIXME: localize
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
            initWithString:@"No suitable disk was found on which to install airyxOS.\n\n"
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

#ifdef __AIRYX__
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
    font = [[NSFontManager sharedFontManager] convertFont:font
        toNotHaveTrait:NSItalicFontMask];
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
        //NSArray *zones = @[@"America", @"Asia", @"Atlantic", @"Australia", @"Europe",
        //    @"Indian", @"Pacific", @"UTC", @"Zulu"];
        NSArray *zones = @[@"Atlantic",@"Zulu"];
        NSFileManager *fm = [NSFileManager defaultManager];
        NSEnumerator *dirs = [zones objectEnumerator];
        NSString *dir;
        BOOL isDir = NO;
        while(dir = [dirs nextObject]) {
            NSString *zonepath = [NSString stringWithFormat:@"/usr/share/zoneinfo/%@", dir];
            if([fm fileExistsAtPath:zonepath isDirectory:&isDir]) {
                if(isDir) {
                    NSArray *entries = [fm contentsOfDirectoryAtPath:zonepath error:NULL];
                    NSEnumerator *entryiter = [entries objectEnumerator];
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
    
    if(selectedTimeZone) {
        NSLog(@"selected time zone is %@",selectedTimeZone);
        [_timeZones selectItemWithTitle:selectedTimeZone];
        [_timeZones synchronizeTitleAndSelectedItem];
    }

    [_scrollView setAutohidesScrollers:YES];
    [[_scrollView contentView] setDocumentView:_userInfoView];
    [_userInfoView becomeKey];
    [_BackButton setAction:@selector(proceedToDiskList:)];
    [_BackButton setEnabled:YES];
    [_NextButton setAction:@selector(validateUserInfo:)];
}

- (IBAction)validateUserInfo:(id)sender {
    NSFont *font = [NSFont systemFontOfSize:12.0];
    font = [[NSFontManager sharedFontManager] convertFont:font
        toNotHaveTrait:NSItalicFontMask];
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
        initWithString:@"All contents of the selected disk will be ERASED if you proceed!" // FIXME: localize
        attributes:attr]];

    [self fullNameEditingDidFinish:_fullName];
    [self userNameEditingDidFinish:_userName];
    [self passwordEditingDidFinish:_password];
    [self hostNameEditingDidFinish:_hostName];

    NSLog(@"validate: selected time zone is %@",selectedTimeZone);
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
    [[_scrollView contentView] setDocumentView:_infoConfirmationView];
    [_BackButton setAction:@selector(proceedToUserInfo:)];
//     [_NextButton setAction:@selector(proceedToInstall:)];
     [_NextButton setAction:@selector(proceedToFinalize:)];
}

- (IBAction)proceedToFinalize:(id)sender {
    NSLog(@"finalizing");
    [_NextButton setAction:@selector(terminate:)];
    [_NextButton setTarget:NSApp];
    [_NextButton setTitle:@"Quit"];
    // FIXME: display "all done" screen
}

- (void)deviceSelected:(NSNotification *)aNotification {
    [_NextButton setEnabled:YES];
    selectedDisk = [[[aNotification object] selectedRowIndexes] firstIndex];
}

- (IBAction)timeZoneSelected:(id)sender {
    selectedTimeZone = [sender titleOfSelectedItem];
    NSLog(@"selection complete - time zone is %@",selectedTimeZone);
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
    NSArray *name = [[sender stringValue] componentsSeparatedByString:@" "];
    
    NSString *user;
    NSString *host;

    if([name count] > 1) {
        user = [NSString stringWithFormat:@"%@%@",
            [[name firstObject] substringToIndex:1],
            [name lastObject]];
        host = [[name componentsJoinedByString:@"-"]
                stringByAppendingString:@"-Airyx"];
    } else {
        user = [name firstObject];
        host = @"Airyx";
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

    NSTextView *v = [[NSTextView alloc] initWithFrame:[[_scrollView contentView] frame]];
#ifdef __AIRYX__
    [[[_scrollView contentView] documentView] release];
    [[_scrollView contentView] release];
#endif

    NSClipView *clip = [NSClipView new];
    [clip setDocumentView:v];

    [_scrollView setContentView:clip];
    [_scrollView setAutohidesScrollers:YES];

    GSGeomDisk *disk = [disks objectAtIndex:selectedDisk];
    [disk setDelegate:self];

    [self appendInstallLog:[NSString
        stringWithFormat:@"Clearing disk %@\n", [disk name]]];
    [disk createGPT];
    [self appendInstallLog:@"Creating partitions\n"];
    [disk createPartitions];
    [self appendInstallLog:@"Creating volumes\n"];
    [disk createPools];
    [self appendInstallLog:@"Installing EFI loader\n"];
    [disk initializeEFI];
    [self appendInstallLog:@"Installing files\n"];
    [disk copyFilesystem];

    [_NextButton setAction:@selector(proceedToFinalize:)];
    [_NextButton setEnabled:YES];
    [_NextButton performClick:self];
}

- (void)appendInstallLog:(NSString *)text {
    NSFont *font = [NSFont systemFontOfSize:12.0];
    font = [[NSFontManager sharedFontManager] convertFont:font
        toNotHaveTrait:NSItalicFontMask];
    NSDictionary *attr = [NSDictionary
        dictionaryWithObjects:@[font, [NSColor blackColor]]
        forKeys:@[NSFontAttributeName, NSForegroundColorAttributeName]];
    NSAttributedString *string = [[NSAttributedString alloc] initWithString:text
        attributes:attr];
    NSTextStorage *ts = [[[_scrollView contentView] documentView] textStorage];
    if([ts length] == 0)
        [ts setAttributedString:string];
    else
        [ts appendAttributedString:string];
}

- (void)fileHandleReadDidComplete:(NSNotification *)aNotification {
    NSData *data = [[aNotification userInfo] objectForKey:NSFileHandleNotificationDataItem];
    if([data length] == 0)
        return;
    NSString *text = [[NSString alloc] initWithData:data
        encoding:NSUTF8StringEncoding];
    [self appendInstallLog:text];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
    NSLog(@"will terminate");
}


- (BOOL)applicationSupportsSecureRestorableState:(NSApplication *)app {
    return YES;
}


@end
