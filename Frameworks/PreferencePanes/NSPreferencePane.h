/*
 * ravynOS Preference Panes
 *
 * Copyright (C) 2024 Zoe Knox <zoe@pixin.net>
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

#import <Foundation/NSObject.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSMutableDictionary.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSView.h>

enum _NSPreferencePaneUnselectReply : NSUInteger {
    NSUnselectCancel = 0,
    NSUnselectNow,
    NSUnselectLater,
};
typedef enum _NSPreferencePaneUnselectReply NSPreferencePaneUnselectReply;

extern NSString * const NSPreferencePrefPaneIsAvailableNotification;
extern NSString * const NSPreferencePaneDoUnselectNotification;
extern NSString * const NSPreferencePaneCancelUnselectNotification;
extern NSString * const NSPreferencePaneSwitchToPaneNotification;
extern NSString * const NSPreferencePaneUpdateHelpMenuNotification;

extern NSString * const NSPrefPaneHelpMenuInfoPListKey;
extern NSString * const NSPrefPaneHelpMenuTitleKey;
extern NSString * const NSPrefPaneHelpMenuAnchorKey;

#define kNSPrefPaneHelpMenuInfoPListKey NSPrefPaneHelpMenuInfoPListKey
#define kNSPrefPaneHelpMenuTitleKey NSPrefPaneHelpMenuTitleKey
#define kNSPrefPaneHelpMenuAnchorKey NSPrefPaneHelpMenuAnchorKey

@interface NSPreferencePane : NSObject {
    NSMutableDictionary *_helpItemsDict;
    NSMenu *_help;
}

@property(strong, readonly) NSBundle *bundle;
@property(strong, readonly) NSString *mainNibName;
@property(strong) NSView *mainView;
@property(readonly, getter=isSelected) BOOL selected;
@property(readonly) NSPreferencePaneUnselectReply shouldUnselect;
@property(strong) NSView *firstKeyView;
@property(strong) NSView *initialKeyView;
@property(strong) NSView *lastKeyView;
@property(readonly) BOOL autoSaveTextFields;

@property(weak) IBOutlet NSWindow *window;

-(id)initWithBundle:(NSBundle *)bundle;
-(NSView *)loadMainView;
-(void)assignMainView;
-(void)mainViewDidLoad;

-(void)willSelect;
-(void)didSelect;
-(void)willUnselect;
-(void)didUnselect;
-(void)replyToShouldUnselect:(BOOL)shouldUnselect;

-(void)updateHelpMenuWithArray:(NSArray *)inArrayOfMenuItems;

@end

