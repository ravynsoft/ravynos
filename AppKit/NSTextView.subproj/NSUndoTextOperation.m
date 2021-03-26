//
//  NSUndoTextOperation.m
//  AppKit
//
//  Created by Airy ANDRE on 23/11/12.
//
//

#import "NSUndoTextOperation.h"

#import <AppKit/NSRaise.h>

@implementation _NSAttributes
- (id)initWithAttributes:(NSDictionary *)attributes range:(NSRange)range
{
    if ((self = [super init])) {
        _range = range;
        _attributes = [attributes copy];
    }
    return self;
}

- (void)dealloc
{
    [_attributes release];
    
    [super dealloc];
}

- (void)setAttributesInTextStorage:(NSTextStorage *)textStorage
{
    [textStorage setAttributes:_attributes range:_range];
}

@end


@implementation _NSAttributeRun
- (id)initWithTextStorage:(NSTextStorage *)textStorage range:(NSRange)range
{
    if ((self = [super init])) {
        _range = range;
        _textStorage = [_textStorage retain];

        _attributesArray = [[NSMutableArray alloc] init];
        int location = range.location;
        do {
            NSRange effectiveRange;
            NSDictionary *attrs = [textStorage attributesAtIndex:location effectiveRange:&effectiveRange];
            NSRange attrRange = NSIntersectionRange(range, effectiveRange);
            _NSAttributes *attributes = [[[_NSAttributes alloc] initWithAttributes:attrs range:attrRange] autorelease];
            [_attributesArray addObject:attributes];
            
            location = NSMaxRange(effectiveRange);
        } while (location < NSMaxRange(range));
    }
    return self;
}

- (void)dealloc
{
    [_textStorage release];
    [_attributesArray release];
    
    [super dealloc];
}

- (void)restoreAttributesOfTextStorage:(NSTextStorage *)textStorage
{
    [textStorage beginEditing];
    for (_NSAttributes *attr in _attributesArray) {
        [attr setAttributesInTextStorage:textStorage];
    }
    [textStorage endEditing];
}

- (id)copyWithZone:(NSZone *)zone
{
    _NSAttributeRun *run = [[[self class] allocWithZone:zone] init];
    run->_range = _range;
    run->_attributesArray = [_attributesArray copyWithZone:zone];
    run->_textStorage = [_textStorage retain];
    return run;
}

- (NSRange)range
{
    return _range;
}
@end

@implementation NSUndoTextOperation
- (id)initWithAffectedRange:(NSRange)affectedRange layoutManager:(NSLayoutManager *)layoutManager undoManager:(NSUndoManager *)undoManager
{
    if ((self = [super init])) {
        _affectedRange = affectedRange;
        _layoutManager = [layoutManager retain];
        _undoManager = [undoManager retain];
    }
    return self;
}

- (void)dealloc
{
    [_undoManager release];
    [_layoutManager release];
    
    [super dealloc];
}

- (void)undoRedo:(NSTextStorage *)textStorage
{
    NSInvalidAbstractInvocation();
}

- (NSTextView *)firstTextViewForTextStorage:(NSTextStorage *)textStorage
{
    return [[textStorage.layoutManagers objectAtIndex:0] firstTextView];;
}

- (BOOL)isSupportingCoalescing
{
    return NO;
}

- (NSRange)affectedRange
{
    return _affectedRange;
}

- (void)setAffectedRange:(NSRange)range
{
    _affectedRange = range;
}

- (NSUndoManager *)undoManager
{
    return _undoManager;
}

- (void)setUndoManager:(NSUndoManager *)undoManager
{
    undoManager = [undoManager retain];
    [_undoManager release];
    _undoManager = undoManager;
}

@end

@implementation NSUndoTyping
- (id)initWithAffectedRange:(NSRange)affectedRange layoutManager:(NSLayoutManager *)layoutManager undoManager:(NSUndoManager *)undoManager replacementRange:(NSRange)replacementRange
{
    if ((self = [super initWithAffectedRange:affectedRange layoutManager:layoutManager undoManager:undoManager])) {
        _replacementRange = replacementRange;
        _attributedString = [[layoutManager.textStorage attributedSubstringFromRange:affectedRange] mutableCopy];
    }
    return self;
}

- (void)dealloc
{
    [_attributedString release];
    [super dealloc];
}

- (BOOL)coalesceAffectedRange:(NSRange)affectedRange replacementRange:(NSRange)replacementRange selectedRange:(NSRange)selectedRange text:(NSAttributedString *)string
{
    // - selection is at the end of the known replaced range and zero length and the affected & replacement range location matches
    if ((selectedRange.length == 0) && (selectedRange.location == NSMaxRange(_replacementRange)) && (affectedRange.location == replacementRange.location)) {
        if (replacementRange.length > 0) {
            // Extending the replacement range - new text is just added
            _replacementRange.length += replacementRange.length;
            return YES;
        } else {
            // First try to shorten the replacement range (we're deleting some chars previously inserted)
            if (_replacementRange.length > affectedRange.length) {
                _replacementRange.length -= affectedRange.length;
            } else {
                // Then, when empty, extend the affected range (we're attacking the original text - we must save it from being forgotten)
                int delta = affectedRange.length - _replacementRange.length;
                _replacementRange.length = 0;
                _replacementRange.location -= delta;
                
                _affectedRange.length += delta;
                _affectedRange.location -= delta;
                NSAttributedString *killedString = [string attributedSubstringFromRange:NSMakeRange(_affectedRange.location, delta)];
                [_attributedString insertAttributedString:killedString atIndex:0];
            }
            return YES;
       }
    }
    // We can't coalesce the new change
    return NO;
}

- (void)undoRedo:(NSTextStorage *)textStorage
{
    [[self firstTextViewForTextStorage:textStorage] breakUndoCoalescing];

    // Register the undo/redo operation
    NSUndoTyping *undoOperation = [[[[self class] alloc] initWithAffectedRange:_replacementRange layoutManager:_layoutManager undoManager:_undoManager replacementRange:_affectedRange] autorelease];
    [[_undoManager prepareWithInvocationTarget: undoOperation] undoRedo: textStorage];
    
    // Undo the change
    // Reset the old value and select it
    [textStorage replaceCharactersInRange:_replacementRange withAttributedString:_attributedString];
    [[self firstTextViewForTextStorage:textStorage] setSelectedRange:_affectedRange];

    [[self firstTextViewForTextStorage:textStorage] didChangeText];
}

- (BOOL)isSupportingCoalescing
{
    return YES;
}

@end

@implementation NSUndoSetAttributes
- (id)initWithAffectedRange:(NSRange)affectedRange layoutManager:(NSLayoutManager *)layoutManager undoManager:(NSUndoManager *)undoManager
{
    if ((self = [super initWithAffectedRange:affectedRange layoutManager:layoutManager undoManager:undoManager])) {
        _attributes = [[_NSAttributeRun alloc] initWithTextStorage:layoutManager.textStorage range:affectedRange];
    }
    return self;
}

- (void)dealloc
{
    [_attributes release];
    
    [super dealloc];
}

- (void)undoRedo:(NSTextStorage *)textStorage
{
    [[self firstTextViewForTextStorage:textStorage] breakUndoCoalescing];

    // Register the undo/redo operation
    NSUndoSetAttributes *undoOperation = [[[[self class] alloc] initWithAffectedRange:_affectedRange layoutManager:_layoutManager undoManager:_undoManager] autorelease];
    [[_undoManager prepareWithInvocationTarget: undoOperation] undoRedo: textStorage];
    
    // Undo the change
    [_attributes restoreAttributesOfTextStorage:textStorage];

    [[self firstTextViewForTextStorage:textStorage] didChangeText];
}
@end

@implementation NSUndoReplaceCharacters
- (id)initWithAffectedRange:(NSRange)affectedRange layoutManager:(NSLayoutManager *)layoutManager undoManager:(NSUndoManager *)undoManager replacementRange:(NSRange)replacementRange
{
    if ((self = [super initWithAffectedRange:affectedRange layoutManager:layoutManager undoManager:undoManager])) {
        _replacementRange = replacementRange;
        _attributedString = [[layoutManager.textStorage attributedSubstringFromRange:affectedRange] retain];
    }
    return self;
}

- (void)dealloc
{
    [_attributedString release];
    
    [super dealloc];
}

- (void)undoRedo:(NSTextStorage *)textStorage
{
    [[self firstTextViewForTextStorage:textStorage] breakUndoCoalescing];
    
    // Register the undo/redo operation
    NSUndoReplaceCharacters *undoOperation = [[[[self class] alloc] initWithAffectedRange:_replacementRange layoutManager:_layoutManager undoManager:_undoManager replacementRange:_affectedRange] autorelease];
    [[_undoManager prepareWithInvocationTarget: undoOperation] undoRedo: textStorage];
 
    // Undo the change
    // Reset the old value and select it
    [textStorage replaceCharactersInRange:_replacementRange withAttributedString:_attributedString];
    [[self firstTextViewForTextStorage:textStorage] setSelectedRange:_affectedRange];
    
    [[self firstTextViewForTextStorage:textStorage] didChangeText];
}
@end
