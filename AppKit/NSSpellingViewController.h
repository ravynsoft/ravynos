/* Copyright (c) 2011 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSViewController.h>
#import <AppKit/NSNibLoading.h>

@class NSTextField, NSButton, NSPopUpButton, NSTableView;

@interface NSSpellingViewController : NSViewController {
    IBOutlet NSTextField *_currentWord;
    IBOutlet NSTextField *_infoLabel;
    IBOutlet NSButton *_changeButton;
    IBOutlet NSButton *_findNextButton;

    IBOutlet NSTableView *_suggestionTable;
    IBOutlet NSButton *_ignoreButton;
    IBOutlet NSButton *_learnButton;
    IBOutlet NSButton *_defineButton;
    IBOutlet NSButton *_guessButton;
    IBOutlet NSPopUpButton *_languagesPopUp;
    NSString *_misspelledWord;
    IBOutlet NSTextField *_spellingHint;
}

- (void)updateSpellingPanelWithMisspelledWord:(NSString *)word;

- (void)change:sender;
- (void)findNext:sender;
- (void)ignore:sender;
- (void)learn:sender;
- (void)guess:sender;

@end
