/* Copyright (c) 2007 Johannes Fortmann

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSObject.h>
#import <AppKit/AppKitExport.h>

@class NSString, NSMutableDictionary, NSValueTransformer, NSArray;

@interface _NSBinder : NSObject {
    id _source;
    id _destination;
    NSString *_keyPath;
    NSString *_bindingPath;
    NSString *_binding;
    NSMutableDictionary *_options;
}

// override this if you need to provide different defaults.
// the default implementation gets its options from source.
- (id)defaultBindingOptionsForBinding:(id)binding;

- (id)options;
- (void)setOptions:(id)value;

- (id)source;
- (void)setSource:(id)value;

- (id)destination;
- (void)setDestination:(id)value;

- (NSString *)keyPath;
- (void)setKeyPath:(NSString *)value;

- (NSString *)binding;
- (void)setBinding:(NSString *)value;

- (void)bind;
- (void)unbind;

- (void)setBindingPath:(id)value;
- (id)bindingPath;

- (NSArray *)peerBinders;

- (void)startObservingChanges;
- (void)stopObservingChanges;

@end

@interface _NSBinder (BindingOptions)
- (BOOL)conditionallySetsEditable;
- (BOOL)conditionallySetsEnabled;
- (BOOL)allowsEditingMultipleValues;
- (BOOL)createsSortDescriptor;
- (BOOL)raisesForNotApplicableKeys;
- (BOOL)continuouslyUpdatesValue;
- (id)multipleValuesPlaceholder;
- (id)noSelectionPlaceholder;
- (id)nullPlaceholder;
- (id)transformedObject:(id)object;
- (id)reverseTransformedObject:(id)object;
@end
