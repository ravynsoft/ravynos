/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>
#import <Foundation/NSMapTable.h>
#import <AppKit/AppKitExport.h>

@class NSString, NSAttributedString;

APPKIT_EXPORT NSString *const NSContextHelpModeDidActivateNotification;
APPKIT_EXPORT NSString *const NSContextHelpModeDidDeactivateNotification;

@interface NSHelpManager : NSObject {
    NSMapTable *_objectToText;
}

+ (NSHelpManager *)sharedHelpManager;

+ (BOOL)isContextHelpModeActive;
+ (void)setContextHelpModeActive:(BOOL)flag;

- (NSAttributedString *)contextHelpForObject:object;
- (void)setContextHelp:(NSAttributedString *)text forObject:object;
- (void)removeContextHelpForObject:object;

- (void)showContextHelpForObject:object locationHint:(NSPoint)hintPoint;

- (void)findString:(NSString *)string inBook:(NSString *)bookName;
- (void)openHelpAnchor:(NSString *)anchor inBook:(NSString *)bookName;

@end
