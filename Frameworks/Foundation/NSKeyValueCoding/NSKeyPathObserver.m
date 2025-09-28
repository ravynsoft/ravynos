#import "NSKeyPathObserver.h"
#import <Foundation/NSDictionary.h>
#import <Foundation/NSString.h>

@implementation NSKeyPathObserver

-initWithObject:object observer:observer keyPath:(NSString *)keyPath options:(NSKeyValueObservingOptions)options context:(void *)context {
   _object=object;
   _observer=observer;
   _keyPath=[keyPath copy];
   _options=options;
   _context=context;
   return self;
}

-(void)dealloc {
   [_keyPath release];
   [_changeDictionary release];
   [super dealloc];
}

-object {
   return _object;
}

-observer {
   return _observer;
}

-(NSString *)keyPath {
   return _keyPath;
}

-(NSKeyValueObservingOptions)options {
   return _options;
}

-(void *)context {
   return _context;
}

-(BOOL)willChangeAlreadyChanging {
   _willChangeCount++;
   
   return (_willChangeCount>1)?YES:NO;
}

-(BOOL)didChangeAlreadyChanging {
   _willChangeCount--;
   
   return (_willChangeCount>0)?YES:NO;
}

-(NSMutableDictionary *)changeDictionaryWithInfo:(NSDictionary *)info {
   if(_changeDictionary==nil)
    _changeDictionary=[[NSMutableDictionary alloc] init];
   else
    [_changeDictionary removeAllObjects];
   
   [_changeDictionary addEntriesFromDictionary:info];
   
   return _changeDictionary;
}

-(NSMutableDictionary *)changeDictionary {
   return _changeDictionary;
}

-(void)clearChangeDictionary {
   [_changeDictionary release];
   _changeDictionary=nil;
}

-(NSString *)description {
	return [NSString stringWithFormat:@"<%@ %x _object: %@ _keypath: %@>",isa, self,_object,_keyPath];
}

@end

