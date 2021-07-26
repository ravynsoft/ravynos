//
//  NSUserDefaultsControllerProxy.m
//  AppKit
//
//  Created by Johannes Fortmann on 26.09.08.
//  Copyright 2008 -. All rights reserved.
//

#import <AppKit/NSUserDefaultsControllerProxy.h>
#import <AppKit/NSUserDefaultsController.h>
#import <Foundation/NSString.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSUserDefaults.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSKeyValueObserving.h>

@implementation NSUserDefaultsControllerProxy

-(id)initWithController:(NSUserDefaultsController*)controller {
   if((self=[super init])!=nil){
      _controller = controller;
      _cachedValues = [NSMutableDictionary new];
      [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(userDefaultsDidChange:) name:NSUserDefaultsDidChangeNotification object:[_controller defaults]];
   }
   return self;
}

-(void)dealloc
{
   [_cachedValues release];
   [[NSNotificationCenter defaultCenter] removeObserver:self];
   [super dealloc];
}

-(id)valueForKey:(NSString*)key
{
   id value=[_cachedValues objectForKey:key];
   if(!value)
   {
      value=[[_controller defaults] objectForKey:key];
      if(!value)
         value=[[_controller initialValues] objectForKey:key];

       if(value)
         [_cachedValues setObject:value forKey:key];
   }
   return value;
}


-(void)setValue:(id)value forKey:(NSString*)key
{
    [self willChangeValueForKey:key];
   [_cachedValues setObject:value forKey:key];
   if([_controller appliesImmediately])
      [[_controller defaults] setObject:value forKey:key];
    [self didChangeValueForKey:key];
}

-(void)revert
{
   for(NSString *key in [_cachedValues allKeys])
   {
      [self willChangeValueForKey:key];
      [_cachedValues removeObjectForKey:key];
      [self didChangeValueForKey:key];
   }
}

-(void)save
{
   for(NSString *key in [_cachedValues allKeys])
   {
      [[_controller defaults] setObject:[_cachedValues objectForKey:key] forKey:key];
   }
}

-(void)revertToInitialValues
{
   id initial=[_controller initialValues];
   for(NSString *key in [_cachedValues allKeys])
   {
      [self willChangeValueForKey:key];
      
      id val=[initial objectForKey:key];
      if(val)
         [_cachedValues setObject:val forKey:key];
      else
         [_cachedValues removeObjectForKey:key];
      
      [self didChangeValueForKey:key];
   }
}

-(void)userDefaultsDidChange:(id)notification
{
    // It would be much easier if we have the key in the notification...
   id defaults=[_controller defaults];
    
    NSArray *allKeys = [[defaults dictionaryRepresentation] allKeys];
    for(NSString *key in allKeys) {
      id val=[_cachedValues objectForKey:key];
      id newVal=[defaults objectForKey:key];
      if(![val isEqual:newVal])
      {
         [self willChangeValueForKey:key];

          if (newVal) {
              [_cachedValues setObject:newVal forKey:key];
          } else {
              [_cachedValues removeObjectForKey:key];
          }
         [self didChangeValueForKey:key];
      }      
   }
}

-(BOOL)hasUnappliedChanges
{
   id defaults=[_controller defaults];
   for(NSString *key in [_cachedValues allKeys])
   {
      id val=[_cachedValues objectForKey:key];
      id newVal=[defaults objectForKey:key];
      if(![val isEqual:newVal])
      {
         return YES;
      }      
   }  
   return NO;
}

@end
