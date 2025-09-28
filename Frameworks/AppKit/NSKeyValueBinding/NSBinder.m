/* Copyright (c) 2007 Johannes Fortmann

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import "NSBinder.h"
#import "NSObject+BindingSupport.h"
#import <Foundation/NSString.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSValue.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSKeyValueCoding.h>
#import <Foundation/NSKeyValueObserving.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSValueTransformer.h>
#import <AppKit/NSController.h>

static void* NSBinderChangeContext;

@implementation _NSBinder (BindingOptions)
-(BOOL)conditionallySetsEditable
{
	return [[_options objectForKey:NSConditionallySetsEditableBindingOption] boolValue] && 
   [_source respondsToSelector:@selector(setEditable:)];
}

-(BOOL)conditionallySetsEnabled
{
	return [[_options objectForKey:NSConditionallySetsEnabledBindingOption] boolValue] && 
    [_source respondsToSelector:@selector(setEnabled:)];
}

-(BOOL)allowsEditingMultipleValues
{
	return [[_options objectForKey:NSAllowsEditingMultipleValuesSelectionBindingOption] boolValue];
}

-(BOOL)createsSortDescriptor
{
	return [[_options objectForKey:NSCreatesSortDescriptorBindingOption] boolValue];
}


-(BOOL)raisesForNotApplicableKeys
{
	return [[_options objectForKey:NSRaisesForNotApplicableKeysBindingOption] boolValue];
}

-(BOOL)continuouslyUpdatesValue
{
  return [[_options objectForKey:NSContinuouslyUpdatesValueBindingOption] boolValue];
}

-(id)multipleValuesPlaceholder
{
	id ret=[_options objectForKey:NSMultipleValuesPlaceholderBindingOption];

	// nil or whatever the was configured is fine

	return ret;
}

-(id)noSelectionPlaceholder
{
	id ret=[_options objectForKey:NSNoSelectionPlaceholderBindingOption];

	// nil or whatever the was configured is fine

	return ret;
}

-(id)nullPlaceholder {
	id ret = [_options objectForKey:NSNullPlaceholderBindingOption];
	
	// nil or whatever the was configured is fine

	return ret;
}

-valueTransformer {
   id result=[_options objectForKey:NSValueTransformerBindingOption];
   
   if(result==nil){
    NSString *name=[_options objectForKey:NSValueTransformerNameBindingOption];
    
    if(name==nil)
     return nil;
     
    result=[NSValueTransformer valueTransformerForName:name];
    
	   if(result==nil) {
		   NSBindingDebugLog(kNSBindingDebugLogLevel1, @"[NSValueTransformer valueTransformerForName:%@] failed in NSBinder.m",name);
	   }
	   
    if(result!=nil)
     [_options setObject:result forKey:NSValueTransformerBindingOption];
   }
   
   return result;	
}

-(id)transformedObject:(id)object
{
	id transformer=[self valueTransformer];
	if(!transformer)
		return object;
        
	return [transformer transformedValue:object];
}

-(id)reverseTransformedObject:(id)object
{
	id transformer=[self valueTransformer];
	if(!transformer)
		return object;
	return [transformer reverseTransformedValue:object];
}

-(BOOL)allowsReverseTransformation {
   NSValueTransformer *transformer=[self valueTransformer];
   
   if(transformer==nil)
    return YES;
   
   return [[transformer class] allowsReverseTransformation];
}

@end

@implementation _NSBinder

- (id)source {
    return _source;
}

- (void)setSource:(id)value {
    if (_source != value) 
	{
        _source = value;
		[self setBindingPath:[_source _replacementKeyPathForBinding:_binding]];
    }
}

- (id)destination 
{
    return _destination;
}

- (void)setDestination:(id)value  {
   value=[value retain];
   [_destination release];
   _destination=value;
}

- (NSString*)keyPath {
    return _keyPath;
}

- (void)setKeyPath:(NSString*)value {
    if (_keyPath != value) {
        [_keyPath release];
        _keyPath = [value copy];
    }
}

- (NSString*)binding {
    return _binding;
}

- (void)setBinding:(NSString*)value {
    if (_binding != value) {
        [_binding release];
        _binding = [value copy];
		[self setBindingPath:[_source _replacementKeyPathForBinding:_binding]];
    }
}

-(id)defaultBindingOptionsForBinding:(id)thisBinding {
	return [_source _defaultBindingOptionsForBinding:thisBinding];
}

-options {
    return _options;
}

- (void)setOptions:(id)value {
	[_options release];
// We only use the defaults if no options are specified
// If the Cocoa behavior is to merge the defaults, the defaults we have are wrong
	if(value)
     _options=[value mutableCopy];
    else
 	 _options=[[self defaultBindingOptionsForBinding:_binding] mutableCopy];
}

- (id)bindingPath {
    return _bindingPath;
}

- (void)setBindingPath:(id)value {
    if (_bindingPath != value) {
        [_bindingPath release];
        _bindingPath = [value copy];
    }
}

-(void)startObservingChanges {
   if([self allowsReverseTransformation])
    [_source addObserver:self forKeyPath:_bindingPath  options:0 context:&NSBinderChangeContext];
}


-(void)stopObservingChanges {
   if([self allowsReverseTransformation])
    [_source removeObserver:self forKeyPath:_bindingPath];
}

- (void)observeValueForKeyPath:(NSString *)kp ofObject:(id)object change:(NSDictionary *)change context:(void *)context {

	NSBindingDebugLog(kNSBindingDebugLogLevel1, @"keyPath: %@\n   object: %@\n   change: %@\n    context: %p", kp, object, change, context);
   if([self allowsReverseTransformation]){

    if(context==&NSBinderChangeContext) {
      [self stopObservingChanges];

      NSBindingDebugLog(kNSBindingDebugLogLevel2, @"bind event from %@.%@ alias %@ to %@.%@ (%@)", [_source className], _binding, _bindingPath, [_destination className], _keyPath, self);
      NSBindingDebugLog(kNSBindingDebugLogLevel2, @"new value %@", [_source valueForKeyPath:_bindingPath]);
		NSBindingDebugLog(kNSBindingDebugLogLevel2, @"DST setting %@, for %@",[_source valueForKeyPath:_bindingPath],_keyPath);
      
       id value=[_source valueForKeyPath:_bindingPath];
       
       value=[self reverseTransformedObject:value];
       
       [_destination setValue:value forKeyPath:_keyPath];
      
       [self startObservingChanges];
    }
   }
}

-(void)dealloc {
	[self stopObservingChanges];
	[_keyPath release];
	[_binding release];
	[_options release];
	[_bindingPath release];
    [_destination release];
	[super dealloc];
}

-(void)bind {
}

-(void)unbind {
}

-(NSComparisonResult)compare:(id)other {
	// FIXME: needs to be a compare understanding that 11<20
	return [_binding compare:[other binding]];
}


// Returns all of the source binders sharing the same binding (or fake binding - like enabled, enabled2...)
-(NSArray *)peerBinders {
    NSArray *allUsedBinders=[_source _allUsedBinders];
    
    // there is only one binder and it's self
    if([allUsedBinders count]==1)
     return allUsedBinders;
    
	// FIXME: total hack. assume only one digit at end, 1-9

    NSString *baseName = _binding;
    NSRange numberAtEnd=NSMakeRange([_binding length]-1, 1);
	if([[_binding substringWithRange:numberAtEnd] intValue]!=0) {
        // Check if the path is a valid path or some "fake" property like "enabled2", which then should be part of the "enabled"
        // peers
        @try {
            [_source valueForKeyPath: _bindingPath];
            // No exception - it is a valid path - that will be our base name
        }
        @catch (id e) {
            // "XXXX[digit] is not a real property - could be something like "enable2" - it will be part of the peers for XXXX
            baseName = [_binding substringToIndex:numberAtEnd.location];
        }
    }

    // Find all of the binders having the base name as their binding or something like <base name>[1..9]
	NSMutableArray *result=[NSMutableArray arrayWithObject:self];
    for(_NSBinder *check in allUsedBinders){
        if (check != self) {
            NSString *checkBinding = [check binding];
            if([checkBinding hasPrefix:baseName] && [checkBinding length] <= [baseName length] + 1) {
                BOOL peer = YES;
                if ([checkBinding length] == [baseName length] + 1) {
                    // Only keep the binder if it has a 1..9 at the end
                    NSRange numberAtEnd=NSMakeRange([checkBinding length]-1, 1);
                    if([[checkBinding substringWithRange:numberAtEnd] intValue]==0) {
                        peer = NO;
                    }
                }
                if (peer) {
                    [result addObject:check];
                }
            }
        }
    }

    return result;
}

-(NSString *)description {
   return [NSString stringWithFormat:@"%@:%p %@, %@ -> %@", [self class], self, _binding, _source, _destination];
}

@end


