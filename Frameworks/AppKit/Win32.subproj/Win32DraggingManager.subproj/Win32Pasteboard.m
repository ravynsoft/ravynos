/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - Christopher Lloyd <cjwl@objc.net>
#import <AppKit/Win32Pasteboard.h>
#import <AppKit/Win32GeneralPasteboard.h>
#import <AppKit/Win32TypesAndOwner.h>

@implementation Win32Pasteboard

+(int)nextChangeCount {
   static int nextChangeCount=1;

   return nextChangeCount++;
}

-init {
   _changeCount=[[self class] nextChangeCount];
   _typesAndOwners=[NSMutableArray new];
   _typeToData=[NSMutableDictionary new];
   return self;
}

-(void)dealloc {
   [_typesAndOwners release];
   [_typeToData release];
   [super dealloc];
}

-(int)changeCount {
   return _changeCount;
}

-(void)incrementChangeCount {
   _changeCount=[[self class] nextChangeCount];
}

-(void)incrementServerCount {
   _serverCount++;
}

-(void)decrementServerCount {
   _serverCount--;

   if(_serverCount==0)
    [self incrementChangeCount];
}

-(BOOL)isClient {
   return (_serverCount<=0)?YES:NO;
}

-(int)declareTypes:(NSArray *)types owner:(id)owner {
    Win32TypesAndOwner *typesAndOwner=[Win32TypesAndOwner typesAndOwnerWithTypes:types owner:owner];
    
    [self incrementChangeCount];
    [_typesAndOwners removeAllObjects];
    [_typesAndOwners addObject:typesAndOwner];
    [_typeToData removeAllObjects];
    
    return 0;
}

-(int)addTypes:(NSArray *)types owner:(id)owner {
    Win32TypesAndOwner *typesAndOwner=[Win32TypesAndOwner typesAndOwnerWithTypes:types owner:owner];
    
    [self incrementChangeCount];
    // Add the new types to the pasteboard and remove existing data for it
    [_typesAndOwners addObject:typesAndOwner];
    [_typeToData removeObjectsForKeys:types];
    
    return 0;
}

-(NSArray *)types {
   NSMutableArray *result=[NSMutableArray array];
   int             i,count=[_typesAndOwners count];

   for(i=0;i<count;i++)
    [result addObjectsFromArray:[[_typesAndOwners objectAtIndex:i] types]];

   return result;
}

-(BOOL)setData:(NSData *)data forType:(NSString *)type {
   [_typeToData setObject:[[data copy] autorelease] forKey:type];
   return YES;
}

-(NSData *)dataForType:(NSString *)type {
   NSData *result=[_typeToData objectForKey:type];

   if(result==nil){
    int i,count=[_typesAndOwners count];
 
    for(i=0;i<count;i++){
     Win32TypesAndOwner *check=[_typesAndOwners objectAtIndex:i];

     if([[check types] containsObject:type]){
      [[check owner] pasteboard:self provideDataForType:type];
      break;
     } 
    }
   }

   return [_typeToData objectForKey:type];
}

@end
