/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/Foundation.h>
#import <AppKit/NSEvent.h>

@class NSMenu, NSUndoManager;

@interface NSResponder : NSObject <NSCoding> {
    id _nextResponder;
}

- (NSResponder *)nextResponder;

- (NSMenu *)menu;
- (NSUndoManager *)undoManager;

- (void)setNextResponder:(NSResponder *)responder;
- (void)setMenu:(NSMenu *)menu;

- validRequestorForSendType:(NSString *)sendType returnType:(NSString *)returnType;

- (void)interpretKeyEvents:(NSArray *)events;
- (BOOL)performKeyEquivalent:(NSEvent *)event;
- (BOOL)tryToPerform:(SEL)action with:object;
- (void)noResponderFor:(SEL)action;

- (BOOL)acceptsFirstResponder;
- (BOOL)becomeFirstResponder;
- (BOOL)resignFirstResponder;

- (void)flagsChanged:(NSEvent *)event;

- (NSError *)willPresentError:(NSError *)error;
- (BOOL)presentError:(NSError *)error;
- (void)presentError:(NSError *)error modalForWindow:(NSWindow *)window delegate:delegate didPresentSelector:(SEL)selector contextInfo:(void *)info;

- (void)keyUp:(NSEvent *)event;
- (void)keyDown:(NSEvent *)event;

- (void)cursorUpdate:(NSEvent *)event;
- (void)scrollWheel:(NSEvent *)event;

- (void)mouseUp:(NSEvent *)event;
- (void)mouseDown:(NSEvent *)event;
- (void)mouseMoved:(NSEvent *)event;
- (void)mouseEntered:(NSEvent *)event;
- (void)mouseExited:(NSEvent *)event;
- (void)mouseDragged:(NSEvent *)event;

- (void)rightMouseUp:(NSEvent *)event;
- (void)rightMouseDown:(NSEvent *)event;
- (void)rightMouseDragged:(NSEvent *)event;

@end

@interface NSResponder (NSResponder_keyBindings)

- (void)doCommandBySelector:(SEL)selector;

- (void)insertText:(NSString *)text;
- (void)insertNewline:sender;
- (void)insertNewlineIgnoringFieldEditor:sender;
- (void)insertTab:sender;
- (void)insertTabIgnoringFieldEditor:sender;
- (void)insertBacktab:sender;
- (void)insertParagraphSeparator:sender;

- (void)deleteForward:sender;
- (void)deleteBackward:sender;
- (void)deleteWordForward:sender;
- (void)deleteWordBackward:sender;
- (void)deleteToBeginningOfLine:sender;
- (void)deleteToEndOfLine:sender;
- (void)deleteToBeginningOfParagraph:sender;
- (void)deleteToEndOfParagraph:sender;

- (void)transpose:sender;
- (void)transposeWords:sender;

- (void)indent:sender;
- (void)complete:sender;
- (void)yank:sender;

- (void)cancelOperation:(id)sender;
- (void)uppercaseWord:sender;
- (void)lowercaseWord:sender;
- (void)capitalizeWord:sender;
- (void)changeCaseOfLetter:sender;

- (void)setMark:sender;
- (void)deleteToMark:sender;
- (void)selectToMark:sender;
- (void)swapWithMark:sender;

- (void)selectWord:sender;
- (void)selectLine:sender;
- (void)selectParagraph:sender;
- (void)selectAll:sender;
- (void)centerSelectionInVisibleArea:sender;

- (void)moveUpAndModifySelection:sender;
- (void)moveDownAndModifySelection:sender;

- (void)moveForwardAndModifySelection:sender;
- (void)moveBackwardAndModifySelection:sender;
- (void)moveRightAndModifySelection:sender;
- (void)moveLeftAndModifySelection:sender;

- (void)moveWordForwardAndModifySelection:sender;
- (void)moveWordBackwardAndModifySelection:sender;
- (void)moveWordRightAndModifySelection:sender;
- (void)moveWordLeftAndModifySelection:sender;

- (void)moveUp:sender;
- (void)moveDown:sender;
- (void)moveLeft:sender;
- (void)moveRight:sender;
- (void)moveForward:sender;
- (void)moveBackward:sender;

- (void)moveWordForward:sender;
- (void)moveWordBackward:sender;
- (void)moveWordRight:sender;
- (void)moveWordLeft:sender;

- (void)moveToBeginningOfLine:sender;
- (void)moveToEndOfLine:sender;

- (void)moveToBeginningOfParagraph:sender;
- (void)moveToEndOfParagraph:sender;

- (void)moveToBeginningOfDocument:sender;
- (void)moveToEndOfDocument:sender;

- (void)pageUp:sender;
- (void)pageDown:sender;

- (void)scrollPageUp:sender;
- (void)scrollPageDown:sender;
- (void)scrollLineUp:sender;
- (void)scrollLineDown:sender;

- (void)scrollToBeginningOfDocument:(id)sender;
- (void)scrollToEndOfDocument:(id)sender;

- (void)noop:sender;

- (void)moveToEndOfDocumentAndModifySelection:sender; // n.b.:
- (void)moveToBeginningOfDocumentAndModifySelection:sender; // these aren't in OSX (can't hurt)

@end
