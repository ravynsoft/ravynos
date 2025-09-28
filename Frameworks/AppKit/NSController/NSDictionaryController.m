/* Copyright (c) 2008 Johannes Fortmann
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSDictionaryController.h>
#import <Foundation/NSString.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSException.h>
#import <Foundation/NSKeyedArchiver.h>
#import <Foundation/NSSortDescriptor.h>

@interface NSDictionaryControllerProxy : NSObject
{
   id _key;
   id _dictionary;
   id _controller;
}
@property (copy) id key;
@property (retain) id value;
@property (retain) id dictionary;
@property (assign) id controller;

@end

@implementation NSDictionaryControllerProxy
@synthesize key=_key;

@synthesize dictionary=_dictionary;
@synthesize controller=_controller;

-(id)value {
   return [_dictionary objectForKey:_key];
}

-(void)setValue:(id)newVal {
   [_dictionary setObject:newVal forKey:_key];
}

-(id)description {
   return [NSString stringWithFormat:@"%@ (%@ %p)", [super description], [self key], [self value]];
}

-(void)dealloc {
   [_key release];
   [_dictionary release];
   [super dealloc];
}
@end

@implementation NSDictionaryController

-init {
   [super init];
   // NSDictionaryController has a default sort descriptor, NSArrayController does not
   [self setSortDescriptors:[NSArray arrayWithObject:[NSSortDescriptor sortDescriptorWithKey:@"key" ascending:YES]]];
   _initialKey=@"key";
   _initialValue=@"value";
   return self;
}

-initWithCoder:(NSCoder*)coder {
   if([super initWithCoder:coder]==nil)
    return nil;
    
      _includedKeys=[[coder decodeObjectForKey:@"NSIncludedKeys"] retain];
      _excludedKeys=[[coder decodeObjectForKey:@"NSExcludedKeys"] retain];
   [self setSortDescriptors:[NSArray arrayWithObject:[NSSortDescriptor sortDescriptorWithKey:@"key" ascending:YES]]];
   _initialKey=@"key";
   _initialValue=@"value";
      
	return self;
}

-(void)dealloc {
   [_contentDictionary release];
   [_includedKeys release];
   [_excludedKeys release];
   [_initialKey release];
   [_initialValue release];
   [super dealloc];
}

-(void)prepareContent {
// do nothing ?
}


-(id)contentDictionary {
   return _contentDictionary;
}

-(void)setContent:(id)value {
   if(value==_contentDictionary)
    return;
    
      [_contentDictionary release];
   _contentDictionary=nil;
      
   if(NSIsControllerMarker(value)) {
    [super setContent:[NSMutableArray array]];
    return;
      }
      
   if([value isKindOfClass:[NSDictionary class]]){
      _contentDictionary=[value retain];

      id contentArray=[NSMutableArray array];

      for(id key in [_contentDictionary allKeys]) {
         if(![_excludedKeys containsObject:key]) {
            NSDictionaryControllerProxy* proxy=[NSDictionaryControllerProxy new];
            proxy.key=key;
            proxy.dictionary=_contentDictionary;
            [contentArray addObject:proxy];
            [proxy release];
         }
      }
      
      [super setContent:contentArray];
   }
}

-(void)_setContentDictionary:value {
   [self setContent:value];
}

-(id)newObject {
   NSDictionaryControllerProxy *result=[NSDictionaryControllerProxy new];
   
   result.key=_initialKey;
   result.dictionary=_contentDictionary;
   
   return result;
}

-(NSString *)initialKey {
   return _initialKey;
}

-initialValue {
   return _initialValue;
}

-(void)setInitialKey:(NSString *)key {
   key=[key copy];
   [_initialKey release];
   _initialKey=key;
}

-(void)setInitialValue:value {
   value=[value retain];
   [_initialValue release];
   _initialValue=value;
}

@end
