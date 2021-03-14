/* Copyright (c) 2009 Andy Balholm
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// A subclass of _NSBinder that uses KVO to observe the destination but not the source.
// It caches values so that changes are propagated only if the value has actually changed.

#import "NSBinder.h"

@interface _NSCachingBinder : _NSBinder {
    id _cachedValue;
    BOOL _bound;
    BOOL _currentlyTransferring;
    BOOL _isObserving;
}

- (id)cachedValue;
- (void)setCachedValue:(id)value;

// Set the source (view) object's value to newValue
- (void)showValue:(id)newValue;

// Set the destination (model) object's value to newValue
- (void)applyValue:(id)newValue;
- (void)applyDisplayedValue;

- (id)destinationValue;

- (void)syncUp;
- (void)bind;
- (void)unbind;

@end
