/* Copyright (c) 2007 Johannes Fortmann

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import "NSKVOBinder.h"
#import <Foundation/NSKeyValueObserving.h>
#import <Foundation/NSKeyValueCoding.h>
#import <Foundation/NSException.h>
#import <Foundation/NSString.h>
#import <Foundation/NSDictionary.h>
#import <AppKit/NSController.h>
#import <AppKit/NSControl.h>
#import "NSObject+BindingSupport.h"

static void *NSKVOBinderChangeContext;

@implementation _NSKVOBinder
-(void)startObservingChanges
{ 
  if(!_isObserving){
   @try {
	   NSBindingDebugLog(kNSBindingDebugLogLevel2, @"start observing binding between %@.%@ alias %@ and %@.%@ (%@)", [_source className], _binding, _bindingPath, [_destination className], _keyPath, self);
      [super startObservingChanges];
      [_destination addObserver:self  forKeyPath:_keyPath  options:0 context:&NSKVOBinderChangeContext];
      _isObserving=YES;
   } @catch(id ex) {
      NSLog(@"startObservingChanges %@", ex);
   }
  }
}

-(void)stopObservingChanges {
   @try {
      if(_isObserving) {
		  NSBindingDebugLog(kNSBindingDebugLogLevel2, @"stop observing binding between %@.%@ alias %@ and %@.%@ (%@)", [_source className], _binding, _bindingPath, [_destination className], _keyPath, self);
         [super stopObservingChanges];
         [_destination removeObserver:self forKeyPath:_keyPath];
       _isObserving=NO;
      }
   } @catch(id ex) {
      NSLog(@"stopObservingChanges %@", ex);
   }
}

NSString *NSFormatDisplayPattern(NSString *pattern,id *values,NSUInteger valueCount){
   NSInteger i,length=[pattern length];
   NSUInteger valueIndex=0;
   unichar   buffer[length];
   NSInteger resultCount=0,resultCapacity=256;
   unichar  *result=NSZoneMalloc(NULL,sizeof(unichar)*resultCapacity);
   enum {
    STATE_NONE,
    STATE_PERCENT,
    STATE_OPEN_BRACE,
    STATE_CLOSE_BRACE,
   } state=STATE_NONE;
   
   [pattern getCharacters:buffer];
   for(i=0;i<length;i++){
    unichar code=buffer[i];
    BOOL    append=NO;
    
    switch(state){
    
     case STATE_NONE:
      if(code=='%'){
       valueIndex=0;
       state=STATE_PERCENT;
      }
      else
       append=YES;
      break;
      
     case STATE_PERCENT:
      if(code=='{')
       state=STATE_OPEN_BRACE;
      else {
       append=YES;
       state=STATE_NONE;
      }
      break;
     
     case STATE_OPEN_BRACE:
      if(code=='}')
       state=STATE_CLOSE_BRACE;
       
      if(code>='0' && code<='9'){
       valueIndex*=10;
       valueIndex+=(code-'0');
      }
      break;
     
     case STATE_CLOSE_BRACE:
      valueIndex--;
      if(code=='@'){
       NSString *string=(valueIndex<valueCount)?[values[valueIndex] description]:@"";
       NSInteger s,stringLength=[string length];
       
       while(resultCount+stringLength>=resultCapacity){
        resultCapacity*=2;
        result=NSZoneRealloc(NULL,result,sizeof(unichar)*resultCapacity);
       }
       [string getCharacters:result+resultCount];
       resultCount+=stringLength;
      }
      state=STATE_NONE;
      break;
    }
    
    if(append){
     if(resultCount+1>=resultCapacity){
      resultCapacity*=2;
      result=NSZoneRealloc(NULL,result,sizeof(unichar)*resultCapacity);
     }
     result[resultCount++]=code;
    }
    
   }
   NSString *display=[NSString stringWithCharacters:result length:resultCount];
   NSZoneFree(NULL,result);
   
   return display;
}

-(void)writeDestinationToSource {
   NSArray  *peersIncludingSelf=[self peerBinders];
   NSInteger i,count=(peersIncludingSelf==nil)?1:[peersIncludingSelf count];
   id        allBinders[count];
   id        allValues[count];
   BOOL      isEditable=YES;
   BOOL      containsPlaceholder=NO;
      
   if(count>1)
    peersIncludingSelf=[peersIncludingSelf sortedArrayUsingSelector:@selector(compare:)];

   if(peersIncludingSelf==nil)
    allBinders[0]=self;
   else
    [peersIncludingSelf getObjects:allBinders];

   for(i=0;i<count;i++){
    _NSBinder *binder=allBinders[i];
    id         dstValue=[[binder destination] valueForKeyPath:[binder keyPath]];
    BOOL       isPlaceholder=NO;
          
    if(dstValue==NSMultipleValuesMarker){
     dstValue=[binder multipleValuesPlaceholder];
     
     if(![binder allowsEditingMultipleValues])
      isEditable=NO;
     containsPlaceholder=isPlaceholder=YES;
    }
    else if(dstValue==NSNoSelectionMarker){
     dstValue=[binder noSelectionPlaceholder];
     isEditable=NO;
     containsPlaceholder=isPlaceholder=YES;
    }
    else if(!dstValue || dstValue==[NSNull null]){
      if((dstValue=[binder nullPlaceholder])!=nil)
       containsPlaceholder=isPlaceholder=YES;
    }

    if(!isPlaceholder)
     dstValue=[binder transformedObject:dstValue];
    
   allValues[i]=dstValue;
   }
   
   NSString *pattern=[[allBinders[0] options] objectForKey:NSDisplayPatternBindingOption];
   id        value;
   
	if(pattern!=nil) {
		value=NSFormatDisplayPattern(pattern,allValues,count);
	} else if(count==1) {
		value=allValues[0];
	} else {
	   if([allValues[count -1] isKindOfClass:[NSNumber class]])
	   {
		   BOOL ret;
           // "hidden" property value is a OR of all binders values
           // Other properties values are a AND of all binders values
		   if([_binding isEqual:@"hidden"])
		   {
			   ret=NO;
			   for(i=0; i<count; i++)
			   {
				   id value=allValues[i];
				   if([value respondsToSelector:@selector(boolValue)])
					   ret|=[value boolValue];
				   else
					   ret=YES;
			   }
		   }
		   else
		   {
			   ret=YES;
			   for(i=0; i<count; i++)
			   {
				   id value=allValues[i];
				   if([value respondsToSelector:@selector(boolValue)])
					   ret&=[value boolValue];
				   else
					   ret=NO;
			   }				
		   }
		   value = [NSNumber numberWithBool:ret];
	   } else {
		   value=allValues[0];
	   }
   }
	
   // Somewhere in the binding logic it generates a proper instance for the formatter if there isn't one
   // More binding logic needs to be moved into the view per the KVB doc.s
   if([_source isKindOfClass:[NSControl class]]){
    NSFormatter *formatter=[(NSControl *)_source formatter];
    
    if([formatter isKindOfClass:[NSDateFormatter class]]){
     if(value!=nil && ![value isKindOfClass:[NSDate class]])
      value=[NSDate dateWithTimeIntervalSinceReferenceDate:0];
    }
    
   }
   
	
	BOOL isValidKeyPath = YES;
	id currentValue = nil;
	id bindingPath = [allBinders[0] bindingPath]; // We want the real one - not "xxxx2" fake path from non-main peers
	@try {
		currentValue = [_source valueForKeyPath: bindingPath];
	}
	@catch (id ex) {
		// This might be a "set-only" binding, like valuePath for image views - in these cases, there is nothing to 
		// compare to
		isValidKeyPath = NO;
	}

	if (isValidKeyPath == NO || (currentValue != value && [currentValue isEqual: value] == NO)) {
		// Only update the source if the value is actually different
		NSBindingDebugLog(kNSBindingDebugLogLevel2, @"setting value: %@ on _source: %@ forKeyPath: %@", value, _source, bindingPath);
		for (int i = 0; i < count; ++i) {
			[allBinders[i] stopObservingChanges];
		}
		@try {
			// Not sure it's the right place to do that  - it's probably not - but on Cocoa, BOOL values bound to a nil value are set to NO,
			// even if setting the same property by code to nil using setValue:forKey: is throwing an exception
			// That's certainly the case for properties like "enabled"
			if (value == nil) {
				@try {
					[_source setValue:nil forKeyPath:bindingPath];
				}
				@catch(id ex) {
                    if (isValidKeyPath == NO || [currentValue isEqualTo:[NSNumber numberWithBool:NO]] == NO) {
                        [_source setValue:[NSNumber numberWithBool:NO] forKeyPath:bindingPath];
                    }
				}
			} else {
				[_source setValue:value forKeyPath:bindingPath];
			}
		}
		@catch(id ex) {
			if([self raisesForNotApplicableKeys]){
				for (int i = 0; i < count; ++i) {
					[allBinders[i] startObservingChanges];
				}
				[ex raise];
			}
		}
		for (int i = 0; i < count; ++i) {
			[allBinders[i] startObservingChanges];
		}
		
	} else {
		NSBindingDebugLog(kNSBindingDebugLogLevel2, @"skipping setting value on _source: %@ forKeyPath: %@", _source, bindingPath);
	}

   if([self conditionallySetsEditable])
      [_source setEditable:isEditable];
   if([self conditionallySetsEnabled])
      [_source setEnabled:isEditable];

   if(containsPlaceholder && [_source respondsToSelector:@selector(_setCurrentValueIsPlaceholder:)])
      [_source _setCurrentValueIsPlaceholder:YES];  
}

-(void)syncUp
{
	[self writeDestinationToSource];
}

- (void)observeValueForKeyPath:(NSString *)kp ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	NSBindingDebugLog(kNSBindingDebugLogLevel1, @"keyPath: %@\n   object: %@\n   change: %@\n    context: %p", kp, object, change, context);
	
// We want changes to propogate one way so we stop observing for both our handling and super's,
// otherwise our change will generate a change for the super's observation and it will write a value
// back the value we just got here which causes a problem for read only values
// If this isn't how it's supposed to be, there must be some other logic which prevents writing back values
// which are read only
   
   if(context==&NSKVOBinderChangeContext) {
		[self writeDestinationToSource];
	} else {
		NSBindingDebugLog(kNSBindingDebugLogLevel3, @"punting to super");
		[super observeValueForKeyPath:kp ofObject:object change:change context:context];
    }
}

-(void)bind
{
    NSBindingDebugLog(kNSBindingDebugLogLevel1, @"bind event from %@.%@ to %@.%@ alias %@ (%@)", [_destination className], _keyPath, [_source className], _binding, _bindingPath, self);
	[self syncUp];
	[self startObservingChanges];
}

-(void)unbind
{
    NSBindingDebugLog(kNSBindingDebugLogLevel1, @"alias: %@ (%@)", _bindingPath, self);
	[self stopObservingChanges];
}

@end

